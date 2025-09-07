#include <stdbool.h>
#include <stdio.h>
#include <de_bitmask.h>
#include <benchmark.h>
#include <stdlib.h>

int main(void) {
  de_bvec a = de_bvec_create(64);
  de_bvec_flip_range(&a, 10, 54);
  de_bvec_print(&a);

  run_all_benchmarks(((usize)1) << 6);
  run_all_benchmarks(((usize)1) << 7);
  run_all_benchmarks(((usize)1) << 9);
  // run_all_benchmarks(((usize)1) << 10);
  // run_all_benchmarks(((usize)1) << 14);
}