#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "worker.h"

int worker_process_main(const worker_process_t *process, state_t *state) {
  if (process->thread_count == 0)
    return process->benchmark(process->process_index, 0, state);

  // Start the worker threads
  for (int i = 0; i < process->thread_count; i++) {
    if (worker_thread_start(process->threads[i], state))
      return EXIT_FAILURE;
  }

  // Wait for the threads to finish
  for (int i = 0; i < process->thread_count; i++) {
    int exit_code = worker_thread_wait_for_exit(process->threads[i]);
    if (exit_code != 0)
      return exit_code;
  }

  return EXIT_SUCCESS;
}

worker_process_t *worker_process_create(int (*benchmark)(int process, int thread, state_t *state), int process_index, int thread_count) {
  worker_process_t *process = malloc(sizeof(worker_process_t));
  if (process == NULL)
    return NULL;

  process->threads = malloc(sizeof(pthread_t) * thread_count);
  if (process->threads == NULL) {
    worker_process_free(process);
    return NULL;
  }

  process->benchmark = benchmark;
  process->process_index = process_index;
  process->thread_count = thread_count;

  for (int i = 0; i < thread_count; i++) {
    process->threads[i] = worker_thread_create(benchmark, process_index, i);
    if (process->threads[i] == NULL) {
      worker_process_free(process);
      return NULL;
    }
  }

  return process;
}

int worker_process_start(worker_process_t *process, state_t *state) {
  pid_t pid = fork();

  // Fork failed
  if (pid < 0)
    return -1;

  process->pid = pid;

  // Return parent
  if (pid > 0)
    return 0;

  int exit_code = worker_process_main(process, state);
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
