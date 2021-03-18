#include <stdio.h>
#include <stdlib.h>

#include "lib/utilities.h"
#include "lib/worker.h"

void calculate(const int *input, int *output, int offset, int count) {
  for (int i = offset; i < offset + count; i++) {
    int sum = 0;
    for (int j = 0; j <= input[i]; j++)
      sum += j;
    output[i] = sum;
  }
}

int calculate_kernel(int process_index, int thread_index, state_t *state) {
  struct timespec start, stop;

  // Unpack state
  int *state_values = (int *)state->memory;

  int processes = state_values[0];
  int threads_per_process = state_values[1];
  int dimension = state_values[2];

  int *input = state_values + 1 + 1 + 1;
  int *output = state_values + 1 + 1 + 1 + dimension;

  int worker_count = calculate_worker_count(processes, threads_per_process);
  int worker_index = calculate_worker_index(processes, threads_per_process, process_index, thread_index);

  int operations_per_thread = dimension / worker_count;
  int offset = worker_index * operations_per_thread;

  float *durations = (float *)state->memory + 1 + 1 + 1 + dimension + dimension;

  clock_gettime(CLOCK_MONOTONIC, &start);
  calculate(input, output, offset, operations_per_thread);
  clock_gettime(CLOCK_MONOTONIC, &stop);

  durations[worker_index] = ((stop.tv_sec - start.tv_sec) * 1e9 + (stop.tv_nsec - start.tv_nsec)) / 1e6;
  return 0;
}

void benchmark_raw(int dimension, int print) {
  struct timespec start, stop;

  int *input = malloc(sizeof(int) * dimension);
  int *output = malloc(sizeof(int) * dimension);
  for (int i = 0; i < dimension; i++)
    input[i] = i;

  clock_gettime(CLOCK_MONOTONIC, &start);
  calculate(input, output, 0, dimension);
  clock_gettime(CLOCK_MONOTONIC, &stop);

  if (print) {
    for (int i = 0; i < dimension; i++)
      printf("%d ", output[i]);
    printf("\n");
  }

  printf("raw took: %fms\n", ((stop.tv_sec - start.tv_sec) * 1e9 + (stop.tv_nsec - start.tv_nsec)) / 1e6);
  free(input);
  free(output);
}

void benchmark_kernel(int dimension, int print, int processes, int threads_per_process) {
  struct timespec start, stop;

  // Create a shared state for the following values
  // number of processes (int)
  // number of threads per process (int)
  // dimension (int)
  // dimension inputs (int)
  // dimension outputs (int)
  // worker_count durations (float)
  int worker_count = calculate_worker_count(processes, threads_per_process);
  state_t *state = state_create(sizeof(int) * (1 + 1 + 1 + dimension + dimension) + sizeof(float) * worker_count);
  if (state == NULL)
    return;

  int *state_values = (int *)state->memory;

  state_values[0] = processes;
  state_values[1] = threads_per_process;
  state_values[2] = dimension;

  int *input = state_values + 1 + 1 + 1;
  int *output = state_values + 1 + 1 + 1 + dimension;
  for (int i = 0; i < dimension; i++)
    input[i] = i;
  float *durations = (float*)state->memory + 1 + 1 + 1 + dimension + dimension;

  worker_pool_t *pool = worker_pool_create(&calculate_kernel, processes, threads_per_process);
  if (pool == NULL) {
    state_free(state);
    printf("Failed to allocate memory for managing workers\n");
    return;
  }

  // Start the workers
  clock_gettime(CLOCK_MONOTONIC, &start);
  worker_pool_start(pool, state);
  int exit_code = worker_pool_wait_for_exit(pool);
  if (exit_code != 0) {
    printf("Failed to run process, exited with code %d\n", exit_code);
    return;
  }
  clock_gettime(CLOCK_MONOTONIC, &stop);

  if (print) {
    for (int i = 0; i < dimension; i++)
      printf("%d ", output[i]);
    printf("\n");
  }

  int sum = 0;
  for (int i = 0; i < dimension; i++) {
    sum += i;
    if (output[i] != sum) {
      printf("Warning: Got unexpected output from kernel. At %d, got %d expected %d\n", i, output[i], sum);
      break;
    }
  }

  float duration = 0;
  for (int i = 0; i < worker_count; i++) {
    printf("worker %d took: %fms\n", i, durations[i]);
    duration += durations[i];
  }
  printf("%d processes, %d threads per process took: %fms (outer)\n", processes, threads_per_process, ((stop.tv_sec - start.tv_sec) * 1e9 + (stop.tv_nsec - start.tv_nsec)) / 1e6);
  printf("%d processes, %d threads per process took: %fms (inner)\n", processes, threads_per_process, duration);

  state_free(state);
  worker_pool_free(pool);
}

int main(int argc, char **argv) {
  if (argc != 5) {
    printf("usage: %s <dimension n> <processes n> <threads n> <print {0,1}>\n", argv[0]);
    return EXIT_FAILURE;
  }

  int dimension = atoi(argv[1]);
  int processes = atoi(argv[2]);
  int threads_per_process = atoi(argv[3]);
  int print = atoi(argv[4]);

  int worker_count = calculate_worker_count(processes, threads_per_process);

  if (dimension % worker_count != 0) {
    printf("expected dimension to be evenly divisible by worker count (%d)\n", worker_count);
    return EXIT_FAILURE;
  }

  benchmark_raw(dimension, print);
  benchmark_kernel(dimension, print, processes, threads_per_process);
}
