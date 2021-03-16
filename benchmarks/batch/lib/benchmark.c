#include <stdio.h>

#include "benchmark.h"

int benchmark(const int *values, int offset, int count) {
  for (int i = 0; i < count; i++) {
    int result = values[offset + i] * 2;
    printf("%d, %d: %d\n", offset, count, result);
  }

  return 0;
}
