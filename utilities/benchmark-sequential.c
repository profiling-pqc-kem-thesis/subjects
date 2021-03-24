#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "time.h"

#include "benchmark.h"

static int perform_benchmark(char *name, int (*benchmark)(void *state), int iterations) {
  fprintf(stderr, "fething global state for benchmark '%s', '%s'\n", name, BENCHMARK_SUBJECT_NAME);
  void *state = get_global_state();

  fprintf(stderr, "running benchmark '%s' for '%s'\n", name, BENCHMARK_SUBJECT_NAME);
  float sum = 0;
  fprintf(stderr, "progress: 0");
  for (int i = 0; i < iterations; i++) {
    struct timespec start, stop;
    clock_gettime(CLOCK_MONOTONIC, &start);
    if (benchmark(state)) {
      fprintf(stderr, "error: benchmark '%s' for '%s' failed\n", name, BENCHMARK_SUBJECT_NAME);
      if (state != NULL)
        free(state);
      return 1;
    }
    clock_gettime(CLOCK_MONOTONIC, &stop);
    sum += timespec_to_duration(&start, &stop) / 1e6;
    fprintf(stderr, "\rprogress: % 3.f%%", ((float)i / iterations) * 100);
  }
  fprintf(stderr, "\rprogress:  100%%\n");

  printf("%s %s average (of %d iterations): %fms\n", name, BENCHMARK_SUBJECT_NAME, iterations, sum / iterations);
  if (state != NULL)
    free(state);
  return 0;
}

int benchmark_sequential(int iterations) {
  int run = 0;
  int failed = 0;

#ifdef BENCHMARK_KEYPAIR
    run++;
    failed += perform_benchmark("keypair", &perform_keypair, iterations);
#endif

#ifdef BENCHMARK_ENCRYPT
    run++;
    failed += perform_benchmark("encrypt", &perform_encrypt, iterations);
#endif

#ifdef BENCHMARK_DECRYPT
    run++;
    failed += perform_benchmark("decrypt", &perform_decrypt, iterations);
#endif

#ifdef BENCHMARK_EXCHANGE
    run++;
    failed += perform_benchmark("exchange", &perform_exchange, iterations);
#endif

  if (failed > 0) {
    fprintf(stderr, "error: %d out of %d benchmark(s) failed\n", failed, run);
    return 1;
  }

  if (run == 0)
    fprintf(stderr, "warning: 0 sequential benchmarks run for '%s'\n", BENCHMARK_SUBJECT_NAME);

  return 0;
}
