#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "pool.h"
#include "time.h"

#include "benchmark.h"

typedef struct {
  int batch_size;
  int thread_count;
  void *state;
  int (*benchmark)(void *state);
  unsigned long long durations[];
} benchmark_state_t;

static int benchmark_loader(int thread_index, void *state) {
  benchmark_state_t *benchmark_state = (benchmark_state_t *)state;

  int operations_per_thread = benchmark_state->batch_size / benchmark_state->thread_count;
  int offset = thread_index * operations_per_thread;
  struct timespec inner_start, inner_stop;
  for (int i = offset; i < offset + operations_per_thread; i++) {
    clock_gettime(CLOCK_MONOTONIC, &inner_start);
    if (benchmark_state->benchmark(benchmark_state->state))
      return 1;
    clock_gettime(CLOCK_MONOTONIC, &inner_stop);
    benchmark_state->durations[i] = timespec_to_duration(&inner_start, &inner_stop);
  }
  return 0;
}

static int perform_benchmark(char *name, int (*benchmark)(void *state), int thread_count, int batch_size) {
  fprintf(stderr, "starting benchmark '%s' for '%s'\n", name, BENCHMARK_SUBJECT_NAME);

  int return_value = 1;

  void *state = get_global_state();

  benchmark_state_t *benchmark_state = malloc(sizeof(unsigned long long) * batch_size + sizeof(int) * 2 + sizeof(void *) + sizeof(int (*)(void *)));
  benchmark_state->batch_size = batch_size;
  benchmark_state->thread_count = thread_count;
  benchmark_state->state = state;
  benchmark_state->benchmark = benchmark;
  memset(benchmark_state->durations, 0, sizeof(unsigned long long) * batch_size);

  pool_t *pool = pool_create(&benchmark_loader, thread_count);
  if (pool == NULL) {
    fprintf(stderr, "error: benchmark '%s' for '%s' failed to create\n", name, BENCHMARK_SUBJECT_NAME);
    goto cleanup;
  }

  struct timespec outer_start, outer_stop;

  clock_gettime(CLOCK_MONOTONIC, &outer_start);
  if (pool_start(pool, (void *)benchmark_state)) {
    fprintf(stderr, "error: benchmark '%s' for '%s' failed to start\n", name, BENCHMARK_SUBJECT_NAME);
    goto cleanup;
  }

  if (pool_wait_for_exit(pool, NULL)) {
    fprintf(stderr, "error: benchmark '%s' for '%s' failed to run\n", name, BENCHMARK_SUBJECT_NAME);
    goto cleanup;
  }
  clock_gettime(CLOCK_MONOTONIC, &outer_stop);

  unsigned long long total_inner_time = 0;
  for (int i = 0; i < batch_size; i++)
    total_inner_time += benchmark_state->durations[i];
  unsigned long long total_outer_time = timespec_to_duration(&outer_start, &outer_stop);
  printf("inner average (of %d iterations): %fms\n", batch_size, (total_inner_time / (float)batch_size) / 1e6);
  printf("outer total: %fms\n", total_outer_time / 1e6);
  printf("throughput: %f/s\n", batch_size / (total_outer_time / 1e9));

  return_value = 0;

cleanup:
  if (state != NULL)
    free(state);
  pool_free(pool);
  free(benchmark_state);
  return 0;
}

int benchmark_parallel(int thread_count, int batch_size) {
  if (thread_count < 1) {
    printf("expected thread count to be 1 or larger, was %d\n", thread_count);
    return EXIT_FAILURE;
  }

  if (batch_size % thread_count != 0) {
    printf("expected batch size (%d) to be evenly divisible by thread count (%d)\n", batch_size, thread_count);
    return EXIT_FAILURE;
  }

  int run = 0;
  int failed = 0;
#ifdef BENCHMARK_KEYPAIR
  run++;
  failed += perform_benchmark("keypair", &perform_keypair, thread_count, batch_size);
#endif

#ifdef BENCHMARK_ENCRYPT
  run++;
  failed += perform_benchmark("encrypt", &perform_encrypt, thread_count, batch_size);
#endif

#ifdef BENCHMARK_DECRYPT
  run++;
  failed += perform_benchmark("decrypt", &perform_decrypt, thread_count, batch_size);
#endif

#ifdef BENCHMARK_EXCHANGE
  run++;
  failed += perform_benchmark("exchange", &perform_exchange, thread_count, batch_size);
#endif

  if (failed > 0) {
    fprintf(stderr, "error: %d out of %d benchmark(s) failed\n", failed, run);
    return 1;
  }

  if (run == 0)
    fprintf(stderr, "warning: 0 parallel benchmarks run for '%s'\n", BENCHMARK_SUBJECT_NAME);

  return 0;
}
