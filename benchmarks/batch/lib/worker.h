#ifndef WORKER_H
#define WORKER_H
#include <unistd.h>
#include <pthread.h>

#include "state.h"

typedef struct {
  pthread_t tid;
  int process_index;
  int thread_index;
  int (*benchmark)(int process_index, int thread_index, state_t *state);
} worker_thread_t;

typedef struct {
  pid_t pid;
  int process_index;
  int thread_count;
  worker_thread_t **threads;
  int (*benchmark)(int process_index, int thread_index, state_t *state);
} worker_process_t;

typedef struct {
  int process_count;
  int thread_count;
  int (*benchmark)(int process_index, int thread_index, state_t *state);
  worker_process_t **processes;
  worker_thread_t **threads;
} worker_pool_t;

// Create a worker pool. If the thread and process count is 0, the calling process will perform the work
worker_pool_t *worker_pool_create(int (*benchmark)(int process_index, int thread_index, state_t *state), int process_count, int thread_count);
int worker_pool_start(worker_pool_t *pool, state_t *state);
// Wait for a worker pool to exit, returning its exit code
int worker_pool_wait_for_exit(worker_pool_t *pool);
// Free up all memory from a terminated worker pool
void worker_pool_free(worker_pool_t *pool);

// Create a worker process. If thread count is 0, the process will perform the work
worker_process_t *worker_process_create(int (*benchmark)(int process_index, int thread_index, state_t *state), int process_index, int thread_count);
// Start a worker in a separate process
int worker_process_start(worker_process_t *process, state_t *state);
// Send SIGTERM to the worker, making it exit gracefully as soon as possible
void worker_process_shutdown(worker_process_t *process);
// Send SIGKILL to the worker, exiting it immediately
void worker_process_kill(worker_process_t *process);
// Wait for a worker process to exit, returning its exit code
int worker_process_wait_for_exit(worker_process_t *process);
// Free up all memory from a terminated worker process
void worker_process_free(worker_process_t *process);

// Create a worker thread
worker_thread_t *worker_thread_create(int (*benchmark)(int process_index, int thread_index, state_t *state), int process_index, int thread_index);
// Start a worker in a separate thread
int worker_thread_start(worker_thread_t *thread, state_t *state);
// Close the worker thread, exiting it immediately
void worker_thread_kill(worker_thread_t *thread);
// Wait for a worker thread to exit, returning its exit code or -1 if failed
int worker_thread_wait_for_exit(worker_thread_t *thread);
// Free up all memory from a terminated worker thread
void worker_thread_free(worker_thread_t *thread);
#endif
