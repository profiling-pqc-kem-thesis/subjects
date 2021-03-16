#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#include "benchmark.h"

#include "worker.h"

int worker_thread_main(const worker_thread_t *thread) {
  struct timespec start, stop;
  clock_gettime(CLOCK_MONOTONIC, &start);

  benchmark(thread->values, thread->offset, thread->count);

  clock_gettime(CLOCK_MONOTONIC, &stop);

  // Print the duration of the worker
  unsigned long long duration = (stop.tv_sec - start.tv_sec) * 1e9 + (stop.tv_nsec - start.tv_nsec);
  printf("%d: %llu\n", thread->id, duration);

  return EXIT_SUCCESS;
}

void *worker_thread_loader(void *parameter) {
  int exit_code = worker_thread_main((const worker_thread_t*)parameter);
  return (void *)exit_code;
}

worker_thread_t *worker_thread_create(const int *values, int id, int offset, int count) {
  worker_thread_t *thread = malloc(sizeof(worker_thread_t));
  if (thread == NULL)
    return NULL;

  thread->values = values;
  thread->id = id;
  thread->offset = offset;
  thread->count = count;

  return thread;
}

int worker_thread_start(worker_thread_t *thread) {
  if (pthread_create(&thread->tid, NULL, worker_thread_loader, thread))
    return 1;

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
