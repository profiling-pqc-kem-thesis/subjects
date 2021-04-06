#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "time.h"

#include "benchmark.h"

static int perform_benchmark(char *name, int (*benchmark)(void *state), int iterations, int timeout) {
  fprintf(stderr, "fething global state for benchmark '%s', '%s'\n", name, BENCHMARK_SUBJECT_NAME);
  void *state = get_global_state();

  fprintf(stderr, "running benchmark '%s' for '%s'\n", name, BENCHMARK_SUBJECT_NAME);
  float sum = 0;
  fprintf(stderr, "progress: 0");
  struct timespec timeout_start, timeout_stop;
  unsigned long long duration = 0;
  clock_gettime(CLOCK_MONOTONIC, &timeout_start);
  int completed_iterations = 0;
  for (; completed_iterations < iterations; completed_iterations++) {
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
    fprintf(stderr, "\rprogress: % 3.f%%", ((float)completed_iterations / iterations) * 100);

    clock_gettime(CLOCK_MONOTONIC, &timeout_stop);
    duration = timespec_to_duration(&timeout_start, &timeout_stop);
    if (duration >= timeout * 1e9)
      break;
  }
  if (duration >= timeout * 1e9)
    fprintf(stderr, "\nwarning: timed out after %.2fs\n", duration / 1e9);
  else
    fprintf(stderr, "\rprogress:  100%%\n");

  printf("%s %s %d iterations took %.4fms\n", name, BENCHMARK_SUBJECT_NAME, completed_iterations, sum);
  printf("%s %s average (of %d iterations): %.4fms\n", name, BENCHMARK_SUBJECT_NAME, completed_iterations, sum / completed_iterations);
  if (state != NULL)
    free(state);
  return 0;
}

int benchmark_sequential(int iterations, int timeout, int benchmark_keypair, int benchmark_encrypt, int benchmark_decrypt, int benchmark_exchange) {
  int run = 0;
  int failed = 0;

#ifdef BENCHMARK_KEYPAIR
  if (benchmark_keypair) {
    run++;
    failed += perform_benchmark("keypair", &perform_keypair, iterations, timeout);
  }
#endif

#ifdef BENCHMARK_ENCRYPT
  if (benchmark_encrypt) {
    run++;
    failed += perform_benchmark("encrypt", &perform_encrypt, iterations, timeout);
  }
#endif

#ifdef BENCHMARK_DECRYPT
  if (benchmark_decrypt) {
    run++;
    failed += perform_benchmark("decrypt", &perform_decrypt, iterations, timeout);
  }
#endif

#ifdef BENCHMARK_EXCHANGE
  if (benchmark_exchange) {
    run++;
    failed += perform_benchmark("exchange", &perform_exchange, iterations, timeout);
  }
#endif

  if (failed > 0) {
    fprintf(stderr, "error: %d out of %d benchmark(s) failed\n", failed, run);
    return 1;
  }

  if (run == 0)
    fprintf(stderr, "warning: 0 sequential benchmarks run for '%s'\n", BENCHMARK_SUBJECT_NAME);

  return 0;
}
