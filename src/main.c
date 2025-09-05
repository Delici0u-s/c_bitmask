#include <stdbool.h>
#include <stdio.h>
#include <de_bitmask.h>
#include <benchmark.h>
#include <stdlib.h>

int main(void) {
  run_all_benchmarks(((usize)1) << 6);
  run_all_benchmarks(((usize)1) << 7);
  run_all_benchmarks(((usize)1) << 9);
  run_all_benchmarks(((usize)1) << 10);
  run_all_benchmarks(((usize)1) << 14);
  // run_all_benchmarks(128);
  // run_all_benchmarks(128);
}