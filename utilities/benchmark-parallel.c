#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "pool.h"
#include "time.h"

#include "benchmark.h"

typedef struct {
  int duration;
  int thread_count;
  int perform;
  void *state;
  int (*benchmark)(void *state);
  unsigned long long iterations[];
} benchmark_state_t;
#define BENCHMARK_STATE_SIZE(thread_count) (sizeof(benchmark_state_t) + sizeof(unsigned long long) * thread_count)

static int benchmark_loader(int thread_index, void *state) {
  benchmark_state_t *benchmark_state = (benchmark_state_t *)state;
  while (benchmark_state->perform) {
    if (benchmark_state->benchmark(benchmark_state->state))
      return 1;
    benchmark_state->iterations[thread_index] += 1;
  }
  return 0;
}

static int perform_benchmark(char *name, int (*benchmark)(void *state), int thread_count, int duration) {
  int return_value = 1;
  benchmark_state_t *benchmark_state = NULL;
  pool_t *pool = NULL;
  void *state = NULL;

  benchmark_state = malloc(BENCHMARK_STATE_SIZE(thread_count));
  if (benchmark_state == NULL) {
    fprintf(stderr, "error: benchmark '%s' for '%s' failed to allocate memory\n", name, BENCHMARK_SUBJECT_NAME);
    goto cleanup;
  }

  pool = pool_create(&benchmark_loader, thread_count);
  if (pool == NULL) {
    fprintf(stderr, "error: benchmark '%s' for '%s' failed to create\n", name, BENCHMARK_SUBJECT_NAME);
    goto cleanup;
  }

  fprintf(stderr, "fething global state for benchmark '%s', '%s'\n", name, BENCHMARK_SUBJECT_NAME);
  state = get_global_state();

  benchmark_state->duration = duration;
  benchmark_state->thread_count = thread_count;
  benchmark_state->perform = 1;
  benchmark_state->state = state;
  benchmark_state->benchmark = benchmark;
  memset(benchmark_state->iterations, 0, sizeof(unsigned long long) * thread_count);

  struct timespec outer_start, outer_stop;

  fprintf(stderr, "running benchmark '%s' for '%s'\n", name, BENCHMARK_SUBJECT_NAME);
  clock_gettime(CLOCK_MONOTONIC, &outer_start);
  if (pool_start(pool, (void *)benchmark_state)) {
    fprintf(stderr, "error: benchmark '%s' for '%s' failed to start\n", name, BENCHMARK_SUBJECT_NAME);
    goto cleanup;
  }

  fprintf(stderr, "progress: 0s");
  for (int i = 0; i < duration; i++) {
    fprintf(stderr, "\rprogress: % 3.f%%", (i / (float)duration) * 100);
    sleep(1);
  }
  fprintf(stderr, "\rprogress: 100%%\n");

  benchmark_state->perform = 0;

  fprintf(stderr, "Waiting for threads to finish\n");
  if (pool_wait_for_exit(pool, NULL)) {
    fprintf(stderr, "error: benchmark '%s' for '%s' failed to run\n", name, BENCHMARK_SUBJECT_NAME);
    goto cleanup;
  }
  clock_gettime(CLOCK_MONOTONIC, &outer_stop);

  unsigned long long total_iterations = 0;
  for (int i = 0; i < thread_count; i++)
    total_iterations += benchmark_state->iterations[i];
  unsigned long long total_outer_time = timespec_to_duration(&outer_start, &outer_stop);
  printf("average iterations per thread (%llu total iterations, %d threads): %.2f\n", total_iterations, thread_count, total_iterations / (float)thread_count);
  printf("average duration per iteration: %fms\n", ((float)total_outer_time / 1e6) / total_iterations);
  printf("outer total: %fms\n", total_outer_time / 1e6);
  printf("throughput: %f/s\n", total_iterations / (total_outer_time / 1e9));

  return_value = 0;

cleanup:
  if (state != NULL)
    free(state);
  if (pool != NULL)
    pool_free(pool);
  if (benchmark_state != NULL)
    free(benchmark_state);
  return return_value;
}

int benchmark_parallel(int thread_count, int duration, int benchmark_keypair, int benchmark_encrypt, int benchmark_decrypt, int benchmark_exchange) {
  if (thread_count < 1) {
    printf("expected thread count to be 1 or larger, was %d\n", thread_count);
    return EXIT_FAILURE;
  }

  int run = 0;
  int failed = 0;

#ifdef BENCHMARK_KEYPAIR
  if (benchmark_keypair) {
    run++;
    failed += perform_benchmark("keypair", &perform_keypair, thread_count, duration);
  }
#endif

#ifdef BENCHMARK_ENCRYPT
  if (benchmark_encrypt) {
    run++;
    failed += perform_benchmark("encrypt", &perform_encrypt, thread_count, duration);
  }
#endif

#ifdef BENCHMARK_DECRYPT
  if (benchmark_decrypt) {
    run++;
    failed += perform_benchmark("decrypt", &perform_decrypt, thread_count, duration);
  }
#endif

#ifdef BENCHMARK_EXCHANGE
  if (benchmark_exchange) {
    run++;
    failed += perform_benchmark("exchange", &perform_exchange, thread_count, duration);
  }
#endif

  if (failed > 0) {
    fprintf(stderr, "error: %d out of %d benchmark(s) failed\n", failed, run);
    return 1;
  }

  if (run == 0)
    fprintf(stderr, "warning: 0 parallel benchmarks run for '%s'\n", BENCHMARK_SUBJECT_NAME);

  return 0;
}