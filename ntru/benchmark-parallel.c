#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "../utilities/pool.h"
#include "../utilities/time.h"

#include "lib/api.h"

typedef struct {
  int dimension;
  int thread_count;
  unsigned long long durations[];
} benchmark_state_t;

int generate_keypair() {
  unsigned char pk[CRYPTO_PUBLICKEYBYTES] = {0};
  unsigned char sk[CRYPTO_SECRETKEYBYTES] = {0};
  return crypto_kem_keypair(pk, sk) < 0;
}

int benchmark_loader(int thread_index, void *state) {
  benchmark_state_t *benchmark_state = (benchmark_state_t *)state;

  int operations_per_thread = benchmark_state->dimension / benchmark_state->thread_count;
  int offset = thread_index * operations_per_thread;
  struct timespec inner_start, inner_stop;
  for (int i = offset; i < offset + operations_per_thread; i++) {
    clock_gettime(CLOCK_MONOTONIC, &inner_start);
    if (generate_keypair())
      return EXIT_FAILURE;
    clock_gettime(CLOCK_MONOTONIC, &inner_stop);
    benchmark_state->durations[i] = timespec_to_duration(&inner_start, &inner_stop);
  }
  return 0;
}

int main(int argc, char **argv) {
  if (argc != 3) {
    printf("usage: %s <thread count n> <dimension n>\n", argv[0]);
    return EXIT_FAILURE;
  }

  int thread_count = atoi(argv[1]);
  int dimension = atoi(argv[2]);

  if (thread_count < 1) {
    printf("expected thread count to be 1 or larger, was %d\n", thread_count);
    return EXIT_FAILURE;
  }

  if (dimension % thread_count != 0) {
    printf("expected dimension (%d) to be evenly divisible by thread count (%d)\n", dimension, thread_count);
    return EXIT_FAILURE;
  }


  benchmark_state_t *benchmark_state = malloc(sizeof(unsigned long long) * dimension + sizeof(int) * 2);
  benchmark_state->dimension = dimension;
  benchmark_state->thread_count = thread_count;
  memset(benchmark_state->durations, 0, sizeof(unsigned long long) * dimension);

  pool_t *pool = pool_create(&benchmark, thread_count);
  if (pool == NULL) {
    printf("failed to create worker pool\n");
    return EXIT_FAILURE;
  }

  struct timespec outer_start, outer_stop;

  clock_gettime(CLOCK_MONOTONIC, &outer_start);
  if (pool_start(pool, (void*)benchmark_state)) {
    printf("failed to start worker pool\n");
    return EXIT_FAILURE;
  }

  if (pool_wait_for_exit(pool, NULL)) {
    printf("one or more threads failed\n");
    return EXIT_FAILURE;
  }
  clock_gettime(CLOCK_MONOTONIC, &outer_stop);

  unsigned long long sum = 0;
  for (int i = 0; i < dimension; i++) {
    sum += benchmark_state->durations[i];
    printf("inner %d: %fms\n", i, benchmark_state->durations[i] / 1e6);
  }
  printf("inner average: %fms\n", (sum / (float)dimension) / 1e6);
  printf("outer: %fms\n", timespec_to_duration(&outer_start, &outer_stop) / 1e6);

  pool_free(pool);
  free(benchmark_state);
}
