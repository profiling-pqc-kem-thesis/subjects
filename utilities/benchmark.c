#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cpu.h"

#include "benchmark.h"

void die_with_usage(char *name) {
  printf("usage: %s [flags]\n", name);
  printf("flags:\n");
  printf("--help             print this help text\n");
  printf("--sequential       perform the sequential benchmark\n");
  printf("--iterations <n>   iterations of the sequential benchmark\n");
  printf("--parallel         perform the parallel benchmark\n");
  printf("--duration <n>     total time in seconds to perform the parallel benchmark\n");
  printf("--thread-count <n> number of threads to use in the parallel benchmark\n");
  exit(0);
}

int main(int argc, char **argv) {
  int perform_sequential = 0;
  int sequential_iterations = 1000;

  int perform_parallel = 0;
  int duration = 600;
  int thread_count = get_available_cores();

  if (argc <= 1)
    die_with_usage(argv[0]);

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--sequential") == 0)
      perform_sequential = 1;
    else if (strcmp(argv[i], "--iterations") == 0)
      sequential_iterations = atoi(argv[++i]);
    else if (strcmp(argv[i], "--parallel") == 0)
      perform_parallel = 1;
    else if (strcmp(argv[i], "--duration") == 0)
      duration = atoi(argv[++i]);
    else if (strcmp(argv[i], "--thread-count") == 0)
      thread_count = atoi(argv[++i]);
    else if (strcmp(argv[i], "--help") == 0)
      die_with_usage(argv[0]);
    else
      die_with_usage(argv[0]);
  }

  if (perform_sequential)
    benchmark_sequential(sequential_iterations);

  if (perform_parallel)
    benchmark_parallel(thread_count, duration);
}
