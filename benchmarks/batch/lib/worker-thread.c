#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#include "worker.h"

typedef struct {
  const worker_thread_t *thread;
  const int *values;
  int offset;
  int count;
} worker_thread_parameters_t;

int worker_thread_main(const worker_thread_t *thread, const int *values, int offset, int count) {
  struct timespec start, stop;
  clock_gettime(CLOCK_MONOTONIC, &start);

  thread->benchmark(values, offset, count);

  clock_gettime(CLOCK_MONOTONIC, &stop);

  // Print the duration of the worker
  unsigned long long duration = (stop.tv_sec - start.tv_sec) * 1e9 + (stop.tv_nsec - start.tv_nsec);
  printf("%d: %lluns\n", thread->id, duration);

  return EXIT_SUCCESS;
}

void *worker_thread_loader(void *parameters) {
  worker_thread_parameters_t *thread_parameters = (worker_thread_parameters_t *)parameters;
  int exit_code = worker_thread_main(thread_parameters->thread, thread_parameters->values, thread_parameters->offset, thread_parameters->count);
  return (void *)exit_code;
}

worker_thread_t *worker_thread_create(int (*benchmark)(const int *, int, int), int id) {
  worker_thread_t *thread = malloc(sizeof(worker_thread_t));
  if (thread == NULL)
    return NULL;

  thread->benchmark = benchmark;
  thread->id = id;

  return thread;
}

int worker_thread_start(worker_thread_t *thread, const int *values, int offset, int count) {
  worker_thread_parameters_t *thread_parameters = malloc(sizeof(worker_thread_parameters_t));
  if (thread_parameters == NULL)
    return 1;

  thread_parameters->thread = thread;
  thread_parameters->values = values;
  thread_parameters->offset = offset;
  thread_parameters->count = count;

  if (pthread_create(&thread->tid, NULL, worker_thread_loader, (void *)thread_parameters)) {
    free(thread_parameters);
    return 1;
  }

  free(thread_parameters);

  return 0;
}

void worker_thread_kill(worker_thread_t *thread) {
  pthread_cancel(thread->tid);
}

int worker_thread_wait_for_exit(worker_thread_t *thread) {
  void *return_value = NULL;
  if (pthread_join(thread->tid, &return_value))
    return -1;

  return (int)return_value;
}

void worker_thread_free(worker_thread_t *thread) {
  free(thread);
}
