#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include "worker.h"
#include "benchmark.h"

int worker_main(const int *values, int worker_id) {
  struct timespec start, stop;
  clock_gettime(CLOCK_MONOTONIC, &start);

  // TODO: Spawn threads
  for (int i = 0; i < 2; i++) {
    benchmark(values, worker_id, i);
  }

  clock_gettime(CLOCK_MONOTONIC, &stop);

  // Print the duration of the worker
  unsigned long long duration = (stop.tv_sec - start.tv_sec) * 1e9 + (stop.tv_nsec - start.tv_nsec);
  printf("%d: %llu\n", worker_id, duration);

  return 0;
}

pid_t spawn_worker(const int *values, int worker_id) {
  pid_t pid = fork();

  // Fork failed
  if (pid < 0)
    return pid;

  // Return parent
  if (pid > 0)
    return 0;

  int exit_code = worker_main(values, worker_id);
  exit(exit_code);
}
