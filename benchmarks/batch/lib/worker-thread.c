#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#include "worker.h"

typedef struct {
  const worker_thread_t *thread;
  state_t *state;
} worker_thread_parameters_t;

int worker_thread_main(const worker_thread_t *thread, state_t *state) {
  return thread->benchmark(thread->process_index, thread->thread_index, state);
}

void *worker_thread_loader(void *parameters) {
  worker_thread_parameters_t *thread_parameters = (worker_thread_parameters_t *)parameters;
  int exit_code = worker_thread_main(thread_parameters->thread, thread_parameters->state);
  free((void*)thread_parameters);
  return (void *)exit_code;
}

worker_thread_t *worker_thread_create(int (*benchmark)(int process_index, int thread_index, state_t *state), int process_index, int thread_index) {
  worker_thread_t *thread = malloc(sizeof(worker_thread_t));
  if (thread == NULL)
    return NULL;

  thread->benchmark = benchmark;
  thread->process_index = process_index;
  thread->thread_index = thread_index;

  return thread;
}

int worker_thread_start(worker_thread_t *thread, state_t *state) {
  // The thread parameters are owned by the thread itself
  worker_thread_parameters_t *thread_parameters = malloc(sizeof(worker_thread_parameters_t));
  if (thread_parameters == NULL)
    return 1;

  thread_parameters->thread = thread;
  thread_parameters->state = state;

  if (pthread_create(&thread->tid, NULL, worker_thread_loader, (void *)thread_parameters)) {
    free(thread_parameters);
    return 1;
  }

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
