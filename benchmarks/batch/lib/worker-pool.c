#include <stdlib.h>

#include "worker.h"

worker_pool_t *worker_pool_create(int (*benchmark)(int process_index, int thread_index, state_t *state), int process_count, int thread_count) {
  worker_pool_t *pool = malloc(sizeof(worker_pool_t));
  if (pool == NULL)
    return NULL;

  pool->process_count = process_count;
  pool->thread_count = thread_count;
  pool->benchmark = benchmark;
  pool->threads = NULL;
  pool->processes = NULL;

  if (process_count > 0) {
    pool->processes = malloc(sizeof(worker_process_t*) * process_count);
    if (pool->processes == NULL) {
      worker_pool_free(pool);
      return NULL;
    }

    for (int i = 0; i < process_count; i++) {
      pool->processes[i] = worker_process_create(benchmark, i, thread_count);
      if (pool->processes[i] == NULL) {
        worker_pool_free(pool);
        return NULL;
      }
    }
  }

  if (process_count == 0 && thread_count > 0) {
    pool->threads = malloc(sizeof(worker_thread_t*) * thread_count);
    if (pool->threads == NULL) {
      worker_pool_free(pool);
      return NULL;
    }

    for (int i = 0; i < thread_count; i++) {
      pool->threads[i] = worker_thread_create(benchmark, 0, i);
      if (pool->threads[i] == NULL) {
        worker_pool_free(pool);
        return NULL;
      }
    }
  }

  return pool;
}

int worker_pool_start(worker_pool_t *pool, state_t *state) {
  // If there are no processes or threads configured, run the benchmark
  // in this process
  if (pool->process_count == 0 && pool->thread_count == 0)
    return pool->benchmark(0, 0, state);

  // If there are no processes configured, run the benchmark on threads
  // in this process
  if (pool->process_count == 0) {
    // Start the worker threads
    for (int i = 0; i < pool->thread_count; i++) {
      if (worker_thread_start(pool->threads[i], state))
        return EXIT_FAILURE;
    }
  } else {
    // Start the worker processes
    for (int i = 0; i < pool->process_count; i++) {
      if (worker_process_start(pool->processes[i], state))
        return EXIT_FAILURE;
    }
  }

  return 0;
}

int worker_pool_wait_for_exit(worker_pool_t *pool) {
  if (pool->process_count == 0) {
    // Wait for the threads to finish
    for (int i = 0; i < pool->thread_count; i++) {
      int exit_code = worker_thread_wait_for_exit(pool->threads[i]);
      if (exit_code != 0)
        return exit_code;
    }
  } else {
    // Wait for the processes to finish
    for (int i = 0; i < pool->process_count; i++) {
      int exit_code = worker_process_wait_for_exit(pool->processes[i]);
      if (exit_code != 0)
        return exit_code;
    }
  }

  return EXIT_SUCCESS;
}

void worker_pool_free(worker_pool_t *pool) {
  if (pool == NULL)
    return;

  if (pool->processes != NULL) {
    for (int i = 0; i < pool->process_count; i++)
      worker_process_free(pool->processes[i]);
    free(pool->processes);
  }

  if (pool->threads != NULL) {
    for (int i = 0; i < pool->thread_count; i++)
      worker_thread_free(pool->threads[i]);
    free(pool->threads);
  }

  free(pool);
}
