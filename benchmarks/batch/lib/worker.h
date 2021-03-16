#ifndef WORKER_H
#define WORKER_H
#include <unistd.h>
#include <pthread.h>

typedef struct {
  pthread_t tid;
  int id;
  const int *values;
  int offset;
  int count;
} worker_thread_t;

typedef struct {
  pid_t pid;
  int id;
  int thread_count;
  worker_thread_t **threads;
  const int *values;
  int offset;
  int count;
} worker_process_t;

// Create a worker process. If thread count is 0, the process will perform the work
worker_process_t *worker_process_create(const int *values, int id, int offset, int count, int thread_count);
// Start a worker in a separate process
int worker_process_start(worker_process_t *process);
// Send SIGTERM to the worker, making it exit gracefully as soon as possible
void worker_process_shutdown(worker_process_t *process);
// Send SIGKILL to the worker, exiting it immediately
void worker_process_kill(worker_process_t *process);
// Wait for a worker process to exit, returning its exit code
int worker_process_wait_for_exit(worker_process_t *process);
// Free up all memory from a terminated worker process
void worker_process_free(worker_process_t *process);

// Create a worker thread
worker_thread_t *worker_thread_create(const int *values, int id, int offset, int count);
// Start a worker in a separate thread
int worker_thread_start(worker_thread_t *thread);
// Close the worker thread, exiting it immediately
void worker_thread_kill(worker_thread_t *thread);
// Wait for a worker thread to exit, returning its exit code or -1 if failed
int worker_thread_wait_for_exit(worker_thread_t *thread);
// Free up all memory from a terminated worker thread
void worker_thread_free(worker_thread_t *thread);
#endif
