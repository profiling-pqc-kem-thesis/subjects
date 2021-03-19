#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "time.h"

#include "benchmark.h"

static int benchmark(char *name, int (*benchmark)(unsigned char *state), unsigned char *(*get_state)(), int iterations) {
  fprintf(stderr, "starting benchmark '%s' for '%s'\n", name, BENCHMARK_SUBJECT_NAME);

  unsigned char *state = get_state();

  float sum;
  for (int i = 0; i < iterations; i++) {
    struct timespec start, stop;
    clock_gettime(CLOCK_MONOTONIC, &start);
    if (benchmark(state) < 0) {
      fprintf(stderr, "error: benchmark '%s' for '%s' failed\n", name, BENCHMARK_SUBJECT_NAME);
      return 1;
    }
    clock_gettime(CLOCK_MONOTONIC, &stop);
    sum += timespec_to_duration(&start, &stop) / 1e6;
  }

  printf("%s %s average (of %d iterations): %fms\n", name, BENCHMARK_SUBJECT_NAME, iterations, sum / iterations);
  return 0;
}

int benchmark_sequential(int iterations) {
  int run = 0;
  int failed = 0;

#ifdef BENCHMARK_KEYPAIR
    run++;
    failed += benchmark("keypair", &keypair, &keypair_state, iterations);
#endif

#ifdef BENCHMARK_ENCRYPT
    run++;
    failed += benchmark("encrypt", &encrypt, &encrypt_state, iterations);
#endif

#ifdef BENCHMARK_DECRYPT
    run++;
    failed += benchmark("decrypt", &decrypt, &decrypt_state, iterations);
#endif

#ifdef BENCHMARK_EXCHANGE
    run++;
    failed += benchmark("exchange", &exchange, &exchange_state, iterations);
#endif

  if (failed > 0) {
    fprintf(stderr, "error: %d out of %d benchmark(s) failed\n", failed, run);
    return 1;
  }

  if (run == 0)
    fprintf(stderr, "warning: 0 sequential benchmarks run for '%s'\n", BENCHMARK_SUBJECT_NAME);

  return 0;
}
