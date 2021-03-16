#include <stdio.h>
#include <stdlib.h>

#include "lib/utilities.h"
#include "lib/worker.h"

// Some static, read-only values that each fork will be able to access (copied)
const int values[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

int main(int argc, char **argv) {
  int worker_count = get_available_cores();
  worker_process_t **workers = malloc(sizeof(worker_process_t*) * worker_count);
  if (workers == NULL) {
    fprintf(stderr, "Unable to setup worker storage\n");
    exit(EXIT_FAILURE);
  }
  printf("Number of workers: %d\n", worker_count);

  // Create all workers
  for (int i = 0; i < worker_count; i++) {
    worker_process_t *worker = worker_process_create(values, i, 0, 16, 5);
    if (worker == NULL) {
      perror("Failed to create worker");
      exit(EXIT_FAILURE);
    }

    workers[i] = worker;
  }

  // Start all workers
  for (int i = 0; i < worker_count; i++) {
    if (worker_process_start(workers[i]) < 0) {
      perror("Failed to start worker");
      exit(EXIT_FAILURE);
    }
  }

  // Wait for all workers to exit
  int status = 0;
  pid_t exited_child = 0;
  while ((exited_child = waitpid(-1, &status, 0)) > 0) {
    int exit_code = WEXITSTATUS(status);
    if (exit_code != 0) {
      fprintf(stderr, "Worker %d failed with code %d\n", exited_child, exit_code);
      exit(EXIT_FAILURE);
    }
  }
}
