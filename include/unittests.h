/*
  de_bitmask_unittest.h — header‑only, strict & verbose unit tests for de_msk

  Usage (single TU):
    #define DE_CONTAINER_BITMASK_IMPLEMENTATION
    #define DE_MSK_TEST_MAIN
    #include "common.h"
    #include "de_bitmask.h"
    #include "de_bitmask_unittest.h"

  Build examples:
    cc -O2 -std=c11 -Wall -Wextra -Wpedantic test.c -o test
    cc -DNDEBUG -O3 -std=c11 -Wall -Wextra -Wpedantic test.c -o test_release

  Notes:
    • Tests ONLY use the public API (no internal data access).
    • Designed to run in both debug and release. In debug, any internal
      assertions inside the implementation may also fire; this test itself
      never relies on them.
    • Deterministic pseudo‑random tests (seed can be overridden).

  Optional defines you can pass before including this header:
    DE_MSK_TEST_SEED    — u64 seed for reproducible randomized checks (default
  0xC0FFEEULL) DE_MSK_TEST_MAXBITS — upper bound for some stress tests (default
  1024) DE_MSK_TEST_VERBOSE — if defined, prints per‑case details (default on
  when DE_MSK_TEST_MAIN)
*/
#ifndef DE_BITMASK_UNITTEST_HEADER
#define DE_BITMASK_UNITTEST_HEADER

#include <common.h>
#include <de_bitmask.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef DE_MSK_TEST_SEED
#define DE_MSK_TEST_SEED 0x0000000000C0FFEEull
#endif
#ifndef DE_MSK_TEST_MAXBITS
#define DE_MSK_TEST_MAXBITS 1024u
#endif

#ifndef DE_MSK_TEST_VERBOSE
#ifdef DE_MSK_TEST_MAIN
#define DE_MSK_TEST_VERBOSE 1
#else
#define DE_MSK_TEST_VERBOSE 0
#endif
#endif

/* ---------- tiny test harness ---------- */

#ifndef DE_MSK_TEST_ABORT_ON_FAIL
#define DE_MSK_TEST_ABORT_ON_FAIL 1
#endif

typedef struct {
  usize total;
  usize passed;
  usize failed;
} de_msk_test_stats;

static de_msk_test_stats de__msk_stats = {0, 0, 0};

#define DE__LIKELY(x) (x)
#define DE__UNLIKELY(x) (x)

#define DE__PRINT(...)                                                         \
  do {                                                                         \
    if (DE_MSK_TEST_VERBOSE) {                                                 \
      printf(__VA_ARGS__);                                                     \
    }                                                                          \
  } while (0)

#define DE__FAIL(msg, ...)                                                     \
  do {                                                                         \
    de__msk_stats.total++;                                                     \
    de__msk_stats.failed++;                                                    \
    fprintf(stderr, "[FAIL] %s:%d: " msg "\n", __FILE__, __LINE__,             \
            ##__VA_ARGS__);                                                    \
    if (DE_MSK_TEST_ABORT_ON_FAIL)                                             \
      abort();                                                                 \
  } while (0)

#define DE__PASS()                                                             \
  do {                                                                         \
    de__msk_stats.total++;                                                     \
    de__msk_stats.passed++;                                                    \
  } while (0)

#define DE_EXPECT_TRUE(expr)                                                   \
  do {                                                                         \
    if (DE__LIKELY((expr))) {                                                  \
      DE__PASS();                                                              \
    } else {                                                                   \
      DE__FAIL("EXPECT_TRUE(" #expr ")");                                      \
    }                                                                          \
  } while (0)

#define DE_EXPECT_FALSE(expr) DE_EXPECT_TRUE(!(expr))

#define DE_EXPECT_EQ_U64(a, b)                                                 \
  do {                                                                         \
    u64 _va = (u64)(a);                                                        \
    u64 _vb = (u64)(b);                                                        \
    if (_va == _vb) {                                                          \
      DE__PASS();                                                              \
    } else {                                                                   \
      DE__FAIL("EXPECT_EQ (u64) %llu != %llu", (unsigned long long)_va,        \
               (unsigned long long)_vb);                                       \
    }                                                                          \
  } while (0)

#define DE_EXPECT_EQ_USIZE(a, b)                                               \
  do {                                                                         \
    usize _va = (usize)(a);                                                    \
    usize _vb = (usize)(b);                                                    \
    if (_va == _vb) {                                                          \
      DE__PASS();                                                              \
    } else {                                                                   \
      DE__FAIL("EXPECT_EQ (usize) %zu != %zu", _va, _vb);                      \
    }                                                                          \
  } while (0)

#define DE_EXPECT_BIT_EQ(actual, expected)                                     \
  do {                                                                         \
    bool _a = (actual);                                                        \
    bool _e = (expected);                                                      \
    if (_a == _e) {                                                            \
      DE__PASS();                                                              \
    } else {                                                                   \
      DE__FAIL("EXPECT_BIT %d != %d", (int)_a, (int)_e);                       \
    }                                                                          \
  } while (0)

/* ---------- helpers that ONLY use public API ---------- */

static inline u64 de__xorshift64star(u64 *s) {
  u64 x = *s;
  x ^= x >> 12;
  x ^= x << 25;
  x ^= x >> 27;
  *s = x;
  return x * 2685821657736338717ull;
}

static inline void de__msk_fill_pattern_incremental(de_msk *m) {
  /* set bits at positions following a simple pattern: i%3==0 or i%5==0 */
  const usize n = de_msk_info_size(m);
  for (usize i = 0; i < n; ++i) {
    bool v = ((i % 3u) == 0u) || ((i % 5u) == 0u);
    de_msk_set(m, i, v);
  }
}

static inline void de__msk_fill_random(de_msk *m, u64 *seed) {
  const usize n = de_msk_info_size(m);
  for (usize i = 0; i < n; ++i) {
    u64 r = de__xorshift64star(seed);
    de_msk_set(m, i, (r >> 63) & 1u);
  }
}

static inline usize de__msk_count_by_get(const de_msk *m) {
  const usize n = de_msk_info_size(m);
  usize c = 0u;
  for (usize i = 0; i < n; ++i) {
    if (de_msk_get(m, i))
      c++;
  }
  return c;
}

static inline void de__msk_expect_equal_bits(const de_msk *a, const de_msk *b) {
  const usize na = de_msk_info_size(a);
  const usize nb = de_msk_info_size(b);
  DE_EXPECT_EQ_USIZE(na, nb);
  const usize n = (na < nb) ? na : nb;
  for (usize i = 0; i < n; ++i) {
    DE_EXPECT_BIT_EQ(de_msk_get(a, i), de_msk_get(b, i));
  }
}

/* builds expected bit value from two inputs under op */
static inline bool de__bit_and(bool x, bool y) { return x && y; }
static inline bool de__bit_or(bool x, bool y) { return x || y; }
static inline bool de__bit_xor(bool x, bool y) { return (!!x) ^ (!!y); }
static inline bool de__bit_not(bool x) { return !x; }

/* ---------- test cases ---------- */

static void de__test_lifecycle_basic(void) {
  DE__PRINT("\n[case] lifecycle_basic\n");
  {
    de_msk m = de_msk_create(0);
    DE_EXPECT_TRUE(de_msk_info_valid(&m));
    DE_EXPECT_EQ_USIZE(de_msk_info_size(&m), 0);
    DE_EXPECT_TRUE(de_msk_none(&m));
    DE_EXPECT_FALSE(de_msk_any(&m));
    DE_EXPECT_TRUE(!de_msk_all(&m)); /* all() over 0 bits should not be true */
    de_msk_delete(&m);
  }
  {
    const usize sizes[] = {1, 2, 63, 64, 65, 127, 128, 129, 255, 256, 511, 512};
    for (usize k = 0; k < sizeof(sizes) / sizeof(sizes[0]); ++k) {
      usize n = sizes[k];
      de_msk m = de_msk_create(n);
      DE_EXPECT_TRUE(de_msk_info_valid(&m));
      DE_EXPECT_EQ_USIZE(de_msk_info_size(&m), n);
      DE_EXPECT_TRUE(de_msk_none(&m));
      DE_EXPECT_FALSE(de_msk_any(&m));
      DE_EXPECT_FALSE(de_msk_all(&m));
      de_msk_delete(&m);
    }
  }
}

static void de__test_single_bit_ops(void) {
  DE__PRINT("\n[case] single_bit_ops\n");
  de_msk m = de_msk_create(130);
  DE_EXPECT_TRUE(de_msk_info_valid(&m));
  DE_EXPECT_EQ_USIZE(de_msk_info_size(&m), 130);
  DE_EXPECT_TRUE(de_msk_none(&m));

  /* set + get boundaries */
  de_msk_set(&m, 0, true);
  DE_EXPECT_TRUE(de_msk_get(&m, 0));
  de_msk_set(&m, 129, true);
  DE_EXPECT_TRUE(de_msk_get(&m, 129));

  /* crossing 64‑bit boundaries */
  de_msk_set(&m, 63, true);
  de_msk_set(&m, 64, true);
  de_msk_set(&m, 65, true);
  DE_EXPECT_TRUE(de_msk_get(&m, 63));
  DE_EXPECT_TRUE(de_msk_get(&m, 64));
  DE_EXPECT_TRUE(de_msk_get(&m, 65));

  /* flip */
  de_msk_flip(&m, 64);
  DE_EXPECT_FALSE(de_msk_get(&m, 64));
  de_msk_flip(&m, 64);
  DE_EXPECT_TRUE(de_msk_get(&m, 64));

  /* clear should zero everything */
  de_msk_clear(&m);
  DE_EXPECT_TRUE(de_msk_none(&m));
  DE_EXPECT_EQ_USIZE(de_msk_count(&m), 0);

  /* fill should set everything */
  de_msk_fill(&m);
  DE_EXPECT_TRUE(de_msk_all(&m));
  DE_EXPECT_FALSE(de_msk_none(&m));
  DE_EXPECT_EQ_USIZE(de_msk_count(&m), de_msk_info_size(&m));

  de_msk_delete(&m);
}

static void de__test_copy_and_move(void) {
  DE__PRINT("\n[case] copy_and_move\n");
  de_msk a = de_msk_create(137);
  de__msk_fill_pattern_incremental(&a);

  /* copy into b */
  de_msk b = de_msk_create(137);
  de_msk_copy(&b, &a);
  de__msk_expect_equal_bits(&a, &b);
  DE_EXPECT_EQ_USIZE(de_msk_count(&a), de_msk_count(&b));

  /* mutate b and ensure a unaffected */
  de_msk_flip(&b, 0);
  DE_EXPECT_FALSE(de_msk_get(&a, 0) == de_msk_get(&b, 0));

  /* move into c; after move we only assert c has a's content. We do not
     rely on any specific state of the moved‑from object. */
  de_msk c = de_msk_create(0);
  de_msk_copy(&c, &a); /* ensure c sized correctly beforehand */
  de_msk_move(&c, &a);
  /* Verify c now equals previous b? No—b was mutated. Compare c with (b flipped
   * back) expectation. */
  de_msk_flip(&b, 0); /* restore b to match original a */
  de__msk_expect_equal_bits(&c, &b);

  de_msk_delete(&a);
  de_msk_delete(&b);
  de_msk_delete(&c);
}

static void de__test_logic_ops_truth_tables(void) {
  DE__PRINT("\n[case] logic_ops_truth_tables\n");
  const usize n = 257; /* spans 4x 64‑bit blocks + remainder */
  de_msk x = de_msk_create(n);
  de_msk y = de_msk_create(n);
  de_msk z = de_msk_create(n);

  /* construct two inputs with different patterns */
  for (usize i = 0; i < n; ++i) {
    de_msk_set(&x, i, (i % 2u) == 0u);
    de_msk_set(&y, i, (i % 3u) == 0u);
  }

  /* AND */
  de_msk_copy(&z, &x);
  de_msk_and_msk(&z, &y);
  for (usize i = 0; i < n; ++i) {
    bool e = de__bit_and(de_msk_get(&x, i), de_msk_get(&y, i));
    DE_EXPECT_BIT_EQ(de_msk_get(&z, i), e);
  }

  /* OR */
  de_msk_copy(&z, &x);
  de_msk_or_msk(&z, &y);
  for (usize i = 0; i < n; ++i) {
    bool e = de__bit_or(de_msk_get(&x, i), de_msk_get(&y, i));
    DE_EXPECT_BIT_EQ(de_msk_get(&z, i), e);
  }

  /* XOR */
  de_msk_copy(&z, &x);
  de_msk_xor_msk(&z, &y);
  for (usize i = 0; i < n; ++i) {
    bool e = de__bit_xor(de_msk_get(&x, i), de_msk_get(&y, i));
    DE_EXPECT_BIT_EQ(de_msk_get(&z, i), e);
  }

  /* NOT */
  de_msk_copy(&z, &x);
  de_msk_not(&z);
  for (usize i = 0; i < n; ++i) {
    bool e = de__bit_not(de_msk_get(&x, i));
    DE_EXPECT_BIT_EQ(de_msk_get(&z, i), e);
  }

  de_msk_delete(&x);
  de_msk_delete(&y);
  de_msk_delete(&z);
}

static void de__test_any_all_none_count(void) {
  DE__PRINT("\n[case] any_all_none_count\n");
  de_msk m = de_msk_create(97);

  DE_EXPECT_TRUE(de_msk_none(&m));
  DE_EXPECT_FALSE(de_msk_any(&m));
  DE_EXPECT_FALSE(de_msk_all(&m));
  DE_EXPECT_EQ_USIZE(de_msk_count(&m), 0);

  de_msk_set(&m, 13, true);
  DE_EXPECT_TRUE(de_msk_any(&m));
  DE_EXPECT_FALSE(de_msk_none(&m));
  DE_EXPECT_FALSE(de_msk_all(&m));
  DE_EXPECT_EQ_USIZE(de_msk_count(&m), 1);

  de_msk_fill(&m);
  DE_EXPECT_TRUE(de_msk_all(&m));
  DE_EXPECT_FALSE(de_msk_none(&m));
  DE_EXPECT_EQ_USIZE(de_msk_count(&m), de_msk_info_size(&m));

  de_msk_clear(&m);
  de__msk_fill_pattern_incremental(&m);
  DE_EXPECT_EQ_USIZE(de_msk_count(&m), de__msk_count_by_get(&m));

  de_msk_delete(&m);
}

static void de__test_randomized_consistency(void) {
  DE__PRINT("\n[case] randomized_consistency\n");
  u64 seed = (u64)DE_MSK_TEST_SEED;
  const usize sizes[] = {7,  8,   15,  16,  31,  32,  63,  64,
                         65, 127, 128, 191, 192, 255, 256, DE_MSK_TEST_MAXBITS};
  for (usize k = 0; k < sizeof(sizes) / sizeof(sizes[0]); ++k) {
    usize n = sizes[k];
    if (n == 0)
      n = 1; /* safety */
    de_msk a = de_msk_create(n);
    de_msk b = de_msk_create(n);
    de_msk c = de_msk_create(n);

    /* randomize a and b */
    de__msk_fill_random(&a, &seed);
    de__msk_fill_random(&b, &seed);

    /* sanity: count equals manual */
    DE_EXPECT_EQ_USIZE(de_msk_count(&a), de__msk_count_by_get(&a));
    DE_EXPECT_EQ_USIZE(de_msk_count(&b), de__msk_count_by_get(&b));

    /* z = ((~a & b) ^ a) | b  — compute via ops and rederive bitwise */
    de_msk_copy(&c, &a);
    de_msk_not(&c);
    de_msk_and_msk(&c, &b);
    de_msk_xor_msk(&c, &a);
    de_msk_or_msk(&c, &b);

    for (usize i = 0; i < n; ++i) {
      bool av = de_msk_get(&a, i);
      bool bv = de_msk_get(&b, i);
      bool e =
          de__bit_or(de__bit_xor(de__bit_and(de__bit_not(av), bv), av), bv);
      DE_EXPECT_BIT_EQ(de_msk_get(&c, i), e);
    }

    de_msk_delete(&a);
    de_msk_delete(&b);
    de_msk_delete(&c);
  }
}

static void de__test_create_i_and_in_place_ops(void) {
  DE__PRINT("\n[case] create_i_and_in_place_ops\n");
  /* create_i: initialize an existing object to a given size */
  de_msk x; /* stack storage; rely on public API only */
  de_msk_create_i(&x, 33);
  DE_EXPECT_TRUE(de_msk_info_valid(&x));
  DE_EXPECT_EQ_USIZE(de_msk_info_size(&x), 33);
  DE_EXPECT_TRUE(de_msk_none(&x));

  /* write something */
  for (usize i = 0; i < de_msk_info_size(&x); ++i)
    de_msk_set(&x, i, (i & 1u));
  // DE_EXPECT_EQ_USIZE(de_msk_count(&x), 33 / 2 + (33 % 2));

  /* in‑place logical NOT twice yields original */
  de_msk y = de_msk_create(33);
  de_msk_copy(&y, &x);
  de_msk_not(&y);
  de_msk_not(&y);
  de__msk_expect_equal_bits(&x, &y);

  de_msk_delete(&x);
  de_msk_delete(&y);
}

/* ---------- test driver ---------- */

static int de_msk_run_all_tests(void) {
  const char *mode =
#ifdef NDEBUG
      "RELEASE";
#else
      "DEBUG";
#endif
  printf("de_msk unit tests — mode: %s\n", mode);

  de__test_lifecycle_basic();
  de__test_single_bit_ops();
  de__test_copy_and_move();
  de__test_logic_ops_truth_tables();
  de__test_any_all_none_count();
  de__test_randomized_consistency();
  de__test_create_i_and_in_place_ops();

  printf("\nSummary: total=%zu, passed=%zu, failed=%zu\n", de__msk_stats.total,
         de__msk_stats.passed, de__msk_stats.failed);
  return (de__msk_stats.failed == 0) ? 0 : 1;
}

#ifdef DE_MSK_TEST_MAIN
int main(void) { return de_msk_run_all_tests(); }
#endif

#endif /* DE_BITMASK_UNITTEST_HEADER */
