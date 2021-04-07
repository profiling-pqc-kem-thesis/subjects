#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include "pool.h"
#include "time.h"

#include "benchmark.h"

typedef struct {
  // The number of iterations to perform
  int iterations_per_thread;
  // The number of threads to use
  int thread_count;
  // Whether or not to run the algorithm (break condition of for loop)
  int perform;
  void *state;
  // The benchmark function to execute
  int (*benchmark)(void *state);
  // The number of iterations run for each thread
  unsigned long long *thread_iterations;
  // The start of each thread
  unsigned long long *thread_start;
  // The stop of each thread
  unsigned long long *thread_stop;
} benchmark_state_t;

static int benchmark_loader(int thread_index, void *state) {
  benchmark_state_t *benchmark_state = (benchmark_state_t *)state;
  benchmark_state->thread_start[thread_index] = timespec_now();
  for (int i = 0; i < benchmark_state->iterations_per_thread && benchmark_state->perform; i++) {
    if (benchmark_state->benchmark(benchmark_state->state)) {
      benchmark_state->thread_stop[thread_index] = timespec_now();
      return 1;
    }

    benchmark_state->thread_iterations[thread_index] += 1;
  }

  benchmark_state->thread_stop[thread_index] = timespec_now();
  return 0;
}

static int perform_benchmark(char *name, int (*benchmark)(void *state), int iterations_per_thread, int thread_count, int timeout) {
  printf("=== Running benchmark ===\n");
  int return_value = 1;
  benchmark_state_t *benchmark_state = NULL;
  pool_t *pool = NULL;
  pool_t *progress_pool = NULL;
  void *state = NULL;

  benchmark_state = malloc(sizeof(benchmark_state_t));
  if (benchmark_state == NULL) {
    fprintf(stderr, "error: benchmark '%s' for '%s' failed to allocate memory\n", name, BENCHMARK_SUBJECT_NAME);
    goto cleanup;
  }

  benchmark_state->thread_iterations = malloc(sizeof(unsigned long long) * thread_count);
  if (benchmark_state == NULL) {
    fprintf(stderr, "error: benchmark '%s' for '%s' failed to allocate memory\n", name, BENCHMARK_SUBJECT_NAME);
    goto cleanup;
  }
  memset(benchmark_state->thread_iterations, 0, sizeof(unsigned long long) * thread_count);

  benchmark_state->thread_start = malloc(sizeof(unsigned long long) * thread_count);
  if (benchmark_state == NULL) {
    fprintf(stderr, "error: benchmark '%s' for '%s' failed to allocate memory\n", name, BENCHMARK_SUBJECT_NAME);
    goto cleanup;
  }
  memset(benchmark_state->thread_start, 0, sizeof(unsigned long long) * thread_count);

  benchmark_state->thread_stop = malloc(sizeof(unsigned long long) * thread_count);
  if (benchmark_state == NULL) {
    fprintf(stderr, "error: benchmark '%s' for '%s' failed to allocate memory\n", name, BENCHMARK_SUBJECT_NAME);
    goto cleanup;
  }
  memset(benchmark_state->thread_stop, 0, sizeof(unsigned long long) * thread_count);

  pool = pool_create(&benchmark_loader, thread_count);
  if (pool == NULL) {
    fprintf(stderr, "error: benchmark '%s' for '%s' failed to create\n", name, BENCHMARK_SUBJECT_NAME);
    goto cleanup;
  }

  fprintf(stderr, "fething global state for benchmark '%s', '%s'\n", name, BENCHMARK_SUBJECT_NAME);
  state = get_global_state();

  benchmark_state->iterations_per_thread = iterations_per_thread;
  benchmark_state->thread_count = thread_count;
  benchmark_state->perform = 1;
  benchmark_state->state = state;
  benchmark_state->benchmark = benchmark;

  struct timespec outer_start, outer_stop;

  fprintf(stderr, "running benchmark '%s' for '%s'\n", name, BENCHMARK_SUBJECT_NAME);
  if (pool_start(pool, (void *)benchmark_state)) {
    fprintf(stderr, "error: benchmark '%s' for '%s' failed to start\n", name, BENCHMARK_SUBJECT_NAME);
    goto cleanup;
  }

  // Keep track of progress in the main thread
  fprintf(stderr, "progress: 0s");
  int timed_out = 0;
  for (int i = 0; i < timeout; i++) {
    int completed_iterations = 0;
    for (int j = 0; j < thread_count; j++)
      completed_iterations += benchmark_state->thread_iterations[j];

    if (completed_iterations >= iterations_per_thread * thread_count)
      break;

    fprintf(stderr, "\rprogress: % 3.f%%", ((float)completed_iterations / (iterations_per_thread * thread_count)) * 100);
    sleep(1);
  }
  if (!timed_out)
    fprintf(stderr, "\rprogress: 100%%\n");

  // Stop all threads gracefully (stop after next iteration)
  benchmark_state->perform = 0;

  if (pool_wait_for_exit(pool, NULL)) {
    fprintf(stderr, "error: benchmark '%s' for '%s' failed to run\n", name, BENCHMARK_SUBJECT_NAME);
    goto cleanup;
  }

  printf("=== Results ===\n");
  printf("thread,iterations,duration (ns)\n");
  unsigned long long total_iterations = 0;
  unsigned long long first_start_time = -1;
  unsigned long long last_stop_time = 0;
  for (int i = 0; i < thread_count; i++) {
    unsigned long long start_time = benchmark_state->thread_start[i];
    unsigned long long stop_time = benchmark_state->thread_stop[i];
    if (start_time < first_start_time)
      first_start_time = start_time;
    if (stop_time > last_stop_time)
      last_stop_time = stop_time;
    printf("%d,%llu,%llu\n", i, benchmark_state->thread_iterations[i], stop_time - start_time);
    total_iterations += benchmark_state->thread_iterations[i];
  }

  printf("=== Summary ===\n");
  unsigned long long total_outer_time = last_stop_time - first_start_time;
  if (total_iterations < (iterations_per_thread * thread_count))
    fprintf(stderr, "warning: timed out after %.2fs\n", total_outer_time / 1e9);
  printf("average iterations per thread (%llu total iterations, %d threads): %.4f\n", total_iterations, thread_count, total_iterations / (float)thread_count);
  printf("average duration per iteration: %.4fms\n", ((float)total_outer_time / 1e6) / total_iterations);
  printf("outer total: %.4fms\n", total_outer_time / 1e6);
  printf("throughput: %.4f/s\n", total_iterations / (total_outer_time / 1e9));

  return_value = 0;

cleanup:
  if (state != NULL)
    free(state);
  if (pool != NULL)
    pool_free(pool);
  if (progress_pool != NULL)
    pool_free(progress_pool);
  if (benchmark_state != NULL) {
    if (benchmark_state->thread_iterations != NULL)
      free(benchmark_state->thread_iterations);
    if (benchmark_state->thread_start != NULL)
      free(benchmark_state->thread_start);
    if (benchmark_state->thread_stop != NULL)
      free(benchmark_state->thread_stop);
    free(benchmark_state);
  }
  return return_value;
}

int benchmark_parallel(int iterations_per_thread, int thread_count, int timeout, int benchmark_keypair, int benchmark_encrypt, int benchmark_decrypt, int benchmark_exchange) {
  if (thread_count < 1) {
    printf("expected thread count to be 1 or larger, was %d\n", thread_count);
    return EXIT_FAILURE;
  }

  int run = 0;
  int failed = 0;

#ifdef BENCHMARK_KEYPAIR
  if (benchmark_keypair) {
    run++;
    failed += perform_benchmark("keypair", &perform_keypair, iterations_per_thread, thread_count, timeout);
  }
#endif

#ifdef BENCHMARK_ENCRYPT
  if (benchmark_encrypt) {
    run++;
    failed += perform_benchmark("encrypt", &perform_encrypt, iterations_per_thread, thread_count, timeout);
  }
#endif

#ifdef BENCHMARK_DECRYPT
  if (benchmark_decrypt) {
    run++;
    failed += perform_benchmark("decrypt", &perform_decrypt, iterations_per_thread, thread_count, timeout);
  }
#endif

#ifdef BENCHMARK_EXCHANGE
  if (benchmark_exchange) {
    run++;
    failed += perform_benchmark("exchange", &perform_exchange, iterations_per_thread, thread_count, timeout);
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
