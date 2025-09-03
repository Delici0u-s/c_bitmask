/*
 * de_bitmask_bench.h
 *
 * Header-only benchmark for de_msk bitmask API.
 *
 * Usage:
 *   - Include this header somewhere in your project.
 *   - Compile with DE_CONTAINER_BITMASK_BENCH_IMPLEMENTATION defined to build
 * an executable.
 *
 * Example compile commands:
 * Strict build (lots of checks):
 *   gcc -std=gnu11 -DE_CONTAINER_BITMASK_BENCH_IMPLEMENTATION -Wall -Wextra
 * -Werror \ -pedantic -g -fsanitize=address,undefined -O0 de_bitmask_bench.h -o
 * bench_strict
 *
 * Release build (high optimization):
 *   gcc -std=gnu11 -DE_CONTAINER_BITMASK_BENCH_IMPLEMENTATION -O3 -march=native
 * -flto \ -DNDEBUG de_bitmask_bench.h -o bench_release
 *
 * (Replace gcc with clang as you prefer.)
 *
 * The bench prints a readable table with times and Mops/sec.
 *
 * NOTE: The bench uses clock_gettime(CLOCK_MONOTONIC). On Windows, adapt to
 * QueryPerformanceCounter if you need Win32 compatibility.
 */

#ifndef DE_BITMASK_BENCH_H
#define DE_BITMASK_BENCH_H

#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Bring in the user's common types and the bitmask API */
#include "common.h"
#include "de_bitmask.h"

/* ------------------------------------------------------------------
 * Configuration
 * ------------------------------------------------------------------ */
#ifndef DE_BENCH_WARMUP_SEC
#define DE_BENCH_WARMUP_SEC 0.1 /* warmup before measurement (seconds) */
#endif

#ifndef DE_BENCH_MIN_SECONDS
#define DE_BENCH_MIN_SECONDS 0.25 /* minimum measurement time per test */
#endif

#ifndef DE_BENCH_TARGET_OPS
#define DE_BENCH_TARGET_OPS                                                    \
  (10000000ULL) /* target ops to try to accumulate per test (for stable        \
                   numbers) */
#endif

/* ------------------------------------------------------------------
 * Portable high-resolution timing (POSIX)
 * ------------------------------------------------------------------ */
static inline double de_time_now_sec(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

/* ------------------------------------------------------------------
 * Readable number formatting helpers
 * ------------------------------------------------------------------ */
static inline void de_format_u64_commas(uint64_t v, char *buf, size_t bufsz) {
  /* Format integer with commas, e.g. 1,234,567 */
  char tmp[64];
  if (bufsz == 0)
    return;
  int pos = 0;
  if (v == 0) {
    snprintf(buf, bufsz, "0");
    return;
  }
  while (v > 0 && pos < (int)sizeof(tmp) - 1) {
    tmp[pos++] = '0' + (v % 10);
    v /= 10;
  }
  int outpos = 0;
  for (int i = 0; i < pos; ++i) {
    if (i > 0 && (i % 3) == 0) {
      if ((size_t)outpos + 1 < bufsz)
        buf[outpos++] = ',';
    }
    if ((size_t)outpos + 1 < bufsz)
      buf[outpos++] = tmp[pos - 1 - i];
  }
  buf[outpos < (int)bufsz ? outpos : (int)bufsz - 1] = '\0';
}

static inline void de_format_double_commas(double v, char *buf, size_t bufsz,
                                           int frac_digits) {
  /* Format double with comma-separated integer part and fixed fractional digits
   */
  if (!isfinite(v)) {
    snprintf(buf, bufsz, "%g", v);
    return;
  }
  if (v < 0.0) {
    char tmp[128];
    de_format_double_commas(-v, tmp, sizeof(tmp), frac_digits);
    snprintf(buf, bufsz, "-%s", tmp);
    return;
  }
  uint64_t intpart = (uint64_t)floor(v);
  double frac = v - (double)intpart;
  char intbuf[64];
  de_format_u64_commas(intpart, intbuf, sizeof(intbuf));
  /* build fractional part */
  char fracbuf[32] = {0};
  if (frac_digits > 0) {
    double scaled = round(frac * pow(10.0, frac_digits));
    /* handle rounding that propagates to integer */
    if (scaled >= pow(10.0, frac_digits)) {
      intpart += 1;
      de_format_u64_commas(intpart, intbuf, sizeof(intbuf));
      scaled = 0;
    }
    snprintf(fracbuf, sizeof(fracbuf), ".%0*llu", frac_digits,
             (unsigned long long)(uint64_t)scaled);
  }
  snprintf(buf, bufsz, "%s%s", intbuf, fracbuf);
}

/* ------------------------------------------------------------------
 * Benchmark utilities
 * ------------------------------------------------------------------ */
typedef void (*de_bench_fn)(void *ctx);

struct de_bench_case {
  const char *name;
  de_bench_fn fn;
  void *ctx;
  uint64_t ops_estimate; /* ops per single invocation of fn (used to compute
                            Mops/sec) */
};

static inline void de_run_bench_case(const struct de_bench_case *c,
                                     double warmup_sec, double min_sec,
                                     uint64_t target_ops, double *out_secs,
                                     uint64_t *out_ops_done) {
  /* Warmup: call the test function repeatedly for warmup_sec */
  double t0 = de_time_now_sec();
  while (de_time_now_sec() - t0 < warmup_sec) {
    c->fn(c->ctx);
  }

  /* Now measure until we've done at least target_ops or min_sec */
  uint64_t ops_done = 0;
  double total_time = 0.0;
  const uint64_t batch =
      1; /* number of fn invocations per inner loop iteration */
  while (total_time < min_sec || ops_done < target_ops) {
    double s = de_time_now_sec();
    c->fn(c->ctx); /* do one batch */
    double e = de_time_now_sec();
    double elapsed = e - s;
    total_time += elapsed;
    ops_done += (c->ops_estimate * batch);
    /* safety: break if time exploded */
    if (total_time > 30.0 && ops_done == 0)
      break;
  }
  if (out_secs)
    *out_secs = total_time;
  if (out_ops_done)
    *out_ops_done = ops_done;
}

/* ------------------------------------------------------------------
 * Bench test implementations (contexts + test functions)
 * Each test function performs a fixed amount of work and returns.
 * The runner calls it repeatedly and we count how many ops per invocation.
 * ------------------------------------------------------------------ */

/* Context: bits and a freshly created mask */
struct de_ctx_create_destroy {
  usize bits;
};
static inline void de_test_create_destroy(void *vctx) {
  struct de_ctx_create_destroy *c = (struct de_ctx_create_destroy *)vctx;
  de_msk m = de_msk_create(c->bits);
  de_msk_delete(&m);
}

/* Context: allocate one mask, set all bits sequentially, then clear it for next
 * invocation */
struct de_ctx_set_all {
  usize bits;
  de_msk m;
};
static inline void de_test_set_all(void *vctx) {
  struct de_ctx_set_all *c = (struct de_ctx_set_all *)vctx;
  /* set every bit */
  for (usize i = 0; i < c->bits; ++i) {
    de_msk_set(&c->m, i, true);
  }
  /* clear for next run */
  de_msk_clear(&c->m);
}

/* Context: allocate one mask, get all bits sequentially */
struct de_ctx_get_all {
  usize bits;
  de_msk m;
};
static inline void de_test_get_all(void *vctx) {
  struct de_ctx_get_all *c = (struct de_ctx_get_all *)vctx;
  volatile bool acc = false;
  for (usize i = 0; i < c->bits; ++i) {
    acc ^= de_msk_get(&c->m, i);
  }
  (void)acc;
}

/* Context: flip all bits sequentially */
struct de_ctx_flip_all {
  usize bits;
  de_msk m;
};
static inline void de_test_flip_all(void *vctx) {
  struct de_ctx_flip_all *c = (struct de_ctx_flip_all *)vctx;
  for (usize i = 0; i < c->bits; ++i) {
    de_msk_flip(&c->m, i);
  }
}

/* Context: count bits repeatedly */
struct de_ctx_count {
  usize bits;
  de_msk m;
};
static inline void de_test_count(void *vctx) {
  struct de_ctx_count *c = (struct de_ctx_count *)vctx;
  volatile usize cnt = de_msk_count(&c->m);
  (void)cnt;
}

/* Context: fill & clear */
struct de_ctx_fill_clear {
  usize bits;
  de_msk m;
};
static inline void de_test_fill_clear(void *vctx) {
  struct de_ctx_fill_clear *c = (struct de_ctx_fill_clear *)vctx;
  de_msk_fill(&c->m);
  de_msk_clear(&c->m);
}

/* Context: bulk logical ops against a second mask */
struct de_ctx_bulk_logic {
  usize bits;
  de_msk a;
  de_msk b;
};
static inline void de_test_and_or_xor_not(void *vctx) {
  struct de_ctx_bulk_logic *c = (struct de_ctx_bulk_logic *)vctx;
  /* do and, or, xor, not in sequence */
  de_msk_and_msk(&c->a, &c->b);
  de_msk_or_msk(&c->a, &c->b);
  de_msk_xor_msk(&c->a, &c->b);
  de_msk_not(&c->a);
}

/* ------------------------------------------------------------------
 * Top-level runner: builds cases for a given bit size and runs them.
 * ------------------------------------------------------------------ */
static inline void de_bench_run_for_bits(usize bits, double warmup_sec,
                                         double min_sec, uint64_t target_ops) {
  /* Prepare readable bit count */
  char bitsbuf[64];
  de_format_u64_commas((uint64_t)bits, bitsbuf, sizeof(bitsbuf));

  /* Prepare contexts */
  struct de_ctx_create_destroy ctx_cd = {.bits = bits};

  struct de_ctx_set_all ctx_set;
  ctx_set.bits = bits;
  ctx_set.m = de_msk_create(bits);

  struct de_ctx_get_all ctx_get;
  ctx_get.bits = bits;
  ctx_get.m = de_msk_create(bits);

  struct de_ctx_flip_all ctx_flip;
  ctx_flip.bits = bits;
  ctx_flip.m = de_msk_create(bits);

  struct de_ctx_count ctx_cnt;
  ctx_cnt.bits = bits;
  ctx_cnt.m = de_msk_create(bits);

  struct de_ctx_fill_clear ctx_fc;
  ctx_fc.bits = bits;
  ctx_fc.m = de_msk_create(bits);

  struct de_ctx_bulk_logic ctx_bl;
  ctx_bl.bits = bits;
  ctx_bl.a = de_msk_create(bits);
  ctx_bl.b = de_msk_create(bits);
  de_msk_fill(&ctx_bl.b); /* fill b so logic ops do work */

  /* Create test table */
  struct de_bench_case cases[] = {
      {"create/destroy", de_test_create_destroy, &ctx_cd,
       1 /* 1 op = create+delete pair */},
      {"set all bits", de_test_set_all, &ctx_set, bits},
      {"get all bits", de_test_get_all, &ctx_get, bits},
      {"flip all bits", de_test_flip_all, &ctx_flip, bits},
      {"count bits", de_test_count, &ctx_cnt, 1 /* one count call */},
      {"fill+clear", de_test_fill_clear, &ctx_fc,
       bits + bits /* fill + clear approximated as 2*bits ops */},
      {"and/or/xor/not", de_test_and_or_xor_not, &ctx_bl,
       (bits * 3) + bits /* and + or + xor + not => approx 4*bits */}};
  size_t ncases = sizeof(cases) / sizeof(cases[0]);

  /* Print header for this bit size */
  printf("---------------------------------------------------------------------"
         "-----\n");
  printf("Bitmask benchmark - bits: %s\n", bitsbuf);
  printf("---------------------------------------------------------------------"
         "-----\n");
  printf("%-20s | %-12s | %-12s | %-12s | %-12s\n", "Test", "Ops", "Time(s)",
         "Mops/sec", "Readable Mops/sec");
  printf("---------------------------------------------------------------------"
         "-----\n");

  for (size_t i = 0; i < ncases; ++i) {
    double secs = 0.0;
    uint64_t ops_done = 0;
    de_run_bench_case(&cases[i], warmup_sec, min_sec, target_ops, &secs,
                      &ops_done);
    double mops = 0.0;
    if (secs > 0.0) {
      /* ops_done is in "operations" as estimated by case->ops_estimate per
       * invocation. At present de_run_bench_case increments ops_done by
       * (ops_estimate * batch) per invocation, so ops_done already counts
       * operations (approx). Compute Mops/sec accordingly.
       */
      mops = (double)ops_done / 1e6 / secs;
    }
    char opsbuf[64];
    de_format_u64_commas(ops_done, opsbuf, sizeof(opsbuf));
    char mopsbuf[64];
    de_format_double_commas(mops, mopsbuf, sizeof(mopsbuf), 2);
    char mopsread[64];
    /* Human-friendly — append "Mops/s" */
    snprintf(mopsread, sizeof(mopsread), "%s Mops/s", mopsbuf);

    printf("%-20s | %-12s | %-12.6f | %-12.2f | %-12s\n", cases[i].name, opsbuf,
           secs, mops, mopsread);
  }

  printf("\n");

  /* Clean up */
  de_msk_delete(&ctx_set.m);
  de_msk_delete(&ctx_get.m);
  de_msk_delete(&ctx_flip.m);
  de_msk_delete(&ctx_cnt.m);
  de_msk_delete(&ctx_fc.m);
  de_msk_delete(&ctx_bl.a);
  de_msk_delete(&ctx_bl.b);
}

/* ------------------------------------------------------------------
 * Expose a friendly run function
 * ------------------------------------------------------------------ */
static inline void de_bench_run_all(void) {
  double warmup = DE_BENCH_WARMUP_SEC;
  double minsec = DE_BENCH_MIN_SECONDS;
  uint64_t target = DE_BENCH_TARGET_OPS;
#ifdef NDEBUG
#define deb_str "yes"
#else
#define deb_str "no"
#endif

  printf("de_bitmask_bench: warmup=%.3fs min_measure=%.3fs target_ops=%" PRIu64
         "\n",
         warmup, minsec, target);
  printf("Build info: sizeof(void*)=%zu, sizeof(size_t)=%zu, NDEBUG=%s\n",
         sizeof(void *), sizeof(size_t), deb_str);
  printf("\n");

  /* A set of bit sizes to exercise small to large masks:
   * 64 bits (small inline), 512, 4096, 65536, 1048576 (1M)
   * You can extend this list if desired.
   */
  const usize sizes[] = {64, 512, 4096, 65536, 1048576};
  size_t nsizes = sizeof(sizes) / sizeof(sizes[0]);
  for (size_t i = 0; i < nsizes; ++i) {
    de_bench_run_for_bits(sizes[i], warmup, minsec, target);
  }
}

/* ------------------------------------------------------------------
 * Main entry if user defined DE_CONTAINER_BITMASK_BENCH_IMPLEMENTATION
 * ------------------------------------------------------------------ */
#ifdef DE_CONTAINER_BITMASK_BENCH_IMPLEMENTATION
int main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  printf("de_bitmask_bench - header-only benchmark\n");
  printf(
      "This binary will run multiple tests and print Mops/sec in a table.\n\n");
  de_bench_run_all();
  return 0;
}
#endif /* DE_CONTAINER_BITMASK_BENCH_IMPLEMENTATION */

#endif /* DE_BITMASK_BENCH_H */
