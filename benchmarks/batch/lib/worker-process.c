#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "benchmark.h"

#include "worker.h"

int worker_process_main(const worker_process_t *process) {
  struct timespec start, stop;
  clock_gettime(CLOCK_MONOTONIC, &start);

  if (process->thread_count == 0) {
    benchmark(process->values, process->offset, process->count);
  } else {
    // Start the worker threads
    for (int i = 0; i < process->thread_count; i++) {
      if (worker_thread_start(process->threads[i]))
        return EXIT_FAILURE;
    }

    // Wait for the threads to finish
    for (int i = 0; i < process->thread_count; i++) {
      int exit_code = worker_thread_wait_for_exit(process->threads[i]);
      if (exit_code != 0)
        return exit_code;
    }
  }

  clock_gettime(CLOCK_MONOTONIC, &stop);

  // Print the duration of the worker
  unsigned long long duration = (stop.tv_sec - start.tv_sec) * 1e9 + (stop.tv_nsec - start.tv_nsec);
  printf("%d: %llu\n", process->id, duration);

  return EXIT_SUCCESS;
}

worker_process_t *worker_process_create(const int *values, int id, int offset, int count, int thread_count) {
  worker_process_t *process = malloc(sizeof(worker_process_t));
  if (process == NULL)
    return NULL;

  process->threads = malloc(sizeof(pthread_t) * thread_count);
  if (process->threads == NULL) {
    worker_process_free(process);
    return NULL;
  }

  process->id = id;
  process->thread_count = thread_count;
  process->values = values;
  process->offset = offset;
  process->count = count;

  for (int i = 0; i < thread_count; i++) {
    // TODO: rework id and offset?
    process->threads[i] = worker_thread_create(values, i, offset, count);
    if (process->threads[i] == NULL) {
      worker_process_free(process);
      return NULL;
    }
  }

  return process;
}

int worker_process_start(worker_process_t *process) {
  pid_t pid = fork();

  // Fork failed
  if (pid < 0)
    return -1;

  process->pid = pid;

  // Return parent
  if (pid > 0)
    return 0;

  int exit_code = worker_process_main(process);
  exit(exit_code);
}

void worker_process_shutdown(worker_process_t *worker) {
  kill(worker->pid, SIGTERM);
}

void worker_process_kill(worker_process_t *worker) {
  kill(worker->pid, SIGKILL);
}

int worker_process_wait_for_exit(worker_process_t *worker) {
  int status = 0;
  while (waitpid(worker->pid, &status, 0) > 0);
  int exit_code = WEXITSTATUS(status);
  return exit_code;
}

void worker_process_free(worker_process_t *process) {
  if (process == NULL)
    return;

  if (process->threads != NULL) {
    for (int i = 0; i < process->thread_count; i++)
      worker_thread_free(process->threads[i]);
    free(process->threads);
  }

  free(process);
}
