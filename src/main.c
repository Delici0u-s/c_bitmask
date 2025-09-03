#include <stdbool.h>
#include <stdio.h>
#include <unittests.h>
#include <benchmarks.h>
#include <de_bitmask.h>
int main(void) {
  de_msk msk = de_msk_create(64);
  de_msk_print(&msk);
  de_msk_set_range(&msk, 4, 20, true);
  de_msk_print(&msk);
  de_msk_flip_range(&msk, 16, 30);
  de_msk_print(&msk);
  de_msk_not(&msk);
  de_msk_print(&msk);

  // de_msk_run_all_tests();
  // const usize n = 10000;
  // de_bench_run_all();
  // printf("%i", 13 / 2);
  // puts("hi");
}