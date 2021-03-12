#include <stdio.h>

#include "benchmark.h"

int benchmark(const int *values, int worker_id, int thread_id) {
  int result = values[worker_id + thread_id] * 2;
  printf("%d, %d: %d\n", worker_id, thread_id, result);

  return 0;
}
