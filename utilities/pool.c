#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pool.h"

typedef struct {
  int (*benchmark)(int thread_index, void *state);
  int thread_index;
  void *state;
} worker_parameters_t;

static void *worker_main(void *parameters) {
  worker_parameters_t *thread_parameters = (worker_parameters_t *)parameters;

  int exit_code = thread_parameters->benchmark(thread_parameters->thread_index, thread_parameters->state);

  free((void*)thread_parameters);

  // Considered a hack on some platforms, but not the ones we target
  // where sizeof(void *) >= sizeof(int). We know there will always
  // be an int returned from this method, hence we don't need to handle overflows
  return (void *)exit_code;
}

pool_t *pool_create(int (*benchmark)(int thread_index, void *state), int thread_count) {
  pool_t *pool = malloc(sizeof(pool_t));
  if (pool == NULL)
    return NULL;

  pool->threads = malloc(sizeof(pthread_t) * thread_count);
  if (pool->threads == NULL) {
    pool_free(pool);
    return NULL;
  }
  memset(pool->threads, 0, sizeof(pthread_t) * thread_count);

  pool->benchmark = benchmark;
  pool->thread_count = thread_count;

  return pool;
}

int pool_start(pool_t *pool, void *state) {
  for (size_t i = 0; i < pool->thread_count; i++) {
    // The thread parameters are owned by the thread itself
    worker_parameters_t *thread_parameters = malloc(sizeof(worker_parameters_t));
    if (thread_parameters == NULL)
      return 1;

    thread_parameters->thread_index = i;
    thread_parameters->state = state;
    thread_parameters->benchmark = pool->benchmark;

    if (pthread_create(&pool->threads[i], NULL, worker_main, (void *)thread_parameters)) {
      free(thread_parameters);
      return 1;
    }
  }

  return 0;
}

void worker_kill(pool_t *pool) {
  for (size_t i = 0; i < pool->thread_count; i++)
    pthread_cancel(pool->threads[i]);
}

int pool_wait_for_exit(pool_t *pool, int *exit_codes) {
  int first_failed_exit_code = 0;
  for (size_t i = 0; i < pool->thread_count; i++) {
    void *return_value = NULL;
    if (pthread_join(pool->threads[i], &return_value))
      return -1;

    // Considered a hack on some platforms, but not the ones we target
    // where sizeof(void *) >= sizeof(int). We know there will always
    // be an int returned, hence we don't need to handle overflows
    int exit_code = return_value;
    if (exit_codes != NULL)
      exit_codes[i] = exit_code;

    if (first_failed_exit_code == 0 && exit_code != 0)
      first_failed_exit_code = exit_code;
  }

  return first_failed_exit_code;
}

void pool_free(pool_t *pool) {
  if (pool == NULL)
    return;

  if (pool->threads != NULL)
    free(pool->threads);
  free(pool);
}
