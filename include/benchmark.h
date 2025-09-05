#ifndef BENCHED_U_ARE
#define BENCHED_U_ARE

#include "common.h"
#include "de_bitmask.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <vadefs.h>

static inline double timespec_diff_sec(struct timespec start,
                                       struct timespec end) {
  return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

#define bench_for_start(iterations, ops_per_sec_output, for_body)              \
  do {                                                                         \
    struct timespec start, end;                                                \
    clock_gettime(CLOCK_MONOTONIC, &start);                                    \
    for (usize i = 0; i < iterations; i++) {                                   \
      for_body                                                                 \
    }                                                                          \
    clock_gettime(CLOCK_MONOTONIC, &end);                                      \
    ops_per_sec_output =                                                       \
        (usize)((double)iterations / timespec_diff_sec(start, end));           \
  } while (0)

usize bench_msk_get(de_msk *const msk, const usize msk_size,
                    const usize iterations) {
  usize ops_per_second;
  volatile bool k;
  bench_for_start(iterations, ops_per_second,
                  { k = de_msk_get(msk, i % msk_size); });

  return ops_per_second;
}

usize bench_msk_set(de_msk *const msk, const usize msk_size,
                    const usize iterations) {
  usize ops_per_second;
  bench_for_start(iterations, ops_per_second,
                  { de_msk_set(msk, i % msk_size, i % 2); });

  return ops_per_second;
}

usize bench_msk_set_range(de_msk *const msk, const usize msk_size,
                          const usize iterations) {
  usize ops_per_second;
  const usize vals[5] = {msk_size, (usize)msk_size / 2, (usize)msk_size / 3,
                         (usize)msk_size / 4, (usize)msk_size / 5};
  const usize vals2[5] = {(usize)msk_size / 2, (usize)msk_size / 3,
                          (usize)msk_size / 4, (usize)msk_size / 5, 0};

  bench_for_start(iterations, ops_per_second,
                  { de_msk_set_range(msk, vals2[i % 5], vals[i % 5], i % 2); });

  return ops_per_second;
}

usize bench_msk_create(const usize msk_size, const usize iterations) {
  usize ops_per_second;
  // i know this leaks memory if not soo :)
  bench_for_start(iterations, ops_per_second,
                  { volatile de_msk msk = de_msk_create(msk_size); });

  return ops_per_second;
}

usize bench_msk_delete(const usize msk_size, const usize iterations) {
  usize ops_per_second;
  volatile de_msk msk = de_msk_create(msk_size);
  // i know this leaks memory if not soo :)
  bench_for_start(iterations, ops_per_second, { de_msk_delete(&msk); });

  return ops_per_second;
}

usize bench_msk_copy(const usize msk_size, const usize iterations) {

  usize ops_per_second;
  volatile de_msk msk1 = de_msk_create(msk_size);
  volatile de_msk msk2 = de_msk_create(msk_size);
  de_msk_flip_range(&msk2, msk_size / 4, msk_size / 2);
  // i know this leaks memory if not soo :)
  bench_for_start(iterations, ops_per_second, { de_msk_copy(&msk1, &msk2); });

  return ops_per_second;
}

usize bench_msk_move(const usize msk_size, const usize iterations) {

  usize ops_per_second;
  volatile de_msk msk1 = de_msk_create(msk_size);
  volatile de_msk msk2 = de_msk_create(msk_size);
  de_msk_flip_range(&msk2, msk_size / 4, msk_size / 2);
  const usize its = iterations / 2;
  bench_for_start(its, ops_per_second, {
    de_msk_move(&msk1, &msk2);
    de_msk_move(&msk2, &msk1);
  });

  // *2 since we do the operation twice
  return ops_per_second * 2;
}

usize bench_msk_fill(de_msk *const msk, const usize msk_size,
                     const usize iterations) {
  usize ops_per_second;
  bench_for_start(iterations, ops_per_second, { de_msk_fill(msk); });

  return ops_per_second;
}

usize bench_msk_clear(de_msk *const msk, const usize msk_size,
                      const usize iterations) {
  usize ops_per_second;
  bench_for_start(iterations, ops_per_second, { de_msk_clear(msk); });

  return ops_per_second;
}

usize bench_msk_any(de_msk *const msk, const usize msk_size,
                    const usize iterations) {
  usize ops_per_second;
  volatile bool res;
  de_msk_clear(msk);
  bench_for_start(iterations, ops_per_second, { res = de_msk_any(msk); });

  return ops_per_second;
}
usize bench_msk_none(de_msk *const msk, const usize msk_size,
                     const usize iterations) {
  usize ops_per_second;
  volatile bool res;
  de_msk_clear(msk);
  bench_for_start(iterations, ops_per_second, { res = de_msk_none(msk); });

  return ops_per_second;
}

usize bench_msk_all(de_msk *const msk, const usize msk_size,
                    const usize iterations) {
  usize ops_per_second;
  volatile bool res;
  de_msk_fill(msk);
  bench_for_start(iterations, ops_per_second, { res = de_msk_all(msk); });

  return ops_per_second;
}
usize bench_msk_count(de_msk *const msk, const usize msk_size,
                      const usize iterations) {
  usize ops_per_second;
  volatile usize res;
  de_msk_fill(msk);
  bench_for_start(iterations, ops_per_second, { res = de_msk_count(msk); });

  return ops_per_second;
}
usize bench_msk_print(de_msk *const msk, const usize msk_size,
                      const usize iterations) {
  usize ops_per_second;

  FILE *old_stdout = stdout; // just for clarity
  fflush(stdout);            // flush anything pending

  freopen("/dev/null", "w", stdout); // redirect stdout to /dev/null

  // run your benchmark
  bench_for_start(iterations, ops_per_second, { de_msk_print(msk); });

  // restore stdout
  fflush(stdout);
  freopen("/dev/tty", "w", stdout); //

  return ops_per_second;
}

typedef struct {
  const float ops;
  const char *name;
} result;

void print_table(usize *count, ...) {
  va_list args;
  va_start(args, *count);

  printf("+---------------------+-----------+\n");
  printf("| %-19s | %-9s |\n", "Name", "MOps/sec");
  printf("+---------------------+-----------+\n");

  for (int i = 0; i < *count; i++) {
    result r = va_arg(args, result);
    printf("| %-19s | %9.4f |\n", r.name, r.ops);
  }

  printf("+---------------------+-----------+\n");

  va_end(args);
}

#define r(_name, _ops)                                                         \
  ((result){.ops = (++arg_count, (_ops / (float)1000000)), .name = #_name})

u0 run_all_benchmarks(const usize msk_size) {
  const usize iterations = 10000000;
  de_msk msk = de_msk_create(msk_size);

  printf("+---------------------+-----------+\n");
  printf("| iters: %-12llu | %-4llu bits |\n", iterations, msk_size);
  // clang-format off
  usize arg_count = 0;
  print_table(&arg_count, 
    r(get, bench_msk_get(&msk, msk_size, iterations)),
    r(set, bench_msk_set(&msk, msk_size, iterations)),
    r(set_range, bench_msk_set_range(&msk, msk_size-1, iterations)),
    r(create, bench_msk_create(msk_size, iterations)),
    r(delete, bench_msk_delete(msk_size, iterations)),
    r(copy, bench_msk_copy(msk_size, iterations)),
    r(move, bench_msk_move(msk_size, iterations)),
    r(fill, bench_msk_fill(&msk, msk_size, iterations)),
    r(clear, bench_msk_clear(&msk, msk_size, iterations)),
    r(any, bench_msk_any(&msk, msk_size, iterations)),
    r(all, bench_msk_all(&msk, msk_size, iterations)),
    r(none, bench_msk_none(&msk, msk_size, iterations)),
    r(count, bench_msk_count(&msk, msk_size, iterations))
    // r(print, bench_msk_print(&msk, msk_size, 100))
  );
  // clang-format on
}

#endif