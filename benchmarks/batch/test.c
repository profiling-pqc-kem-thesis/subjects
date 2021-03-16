#include <stdio.h>

#include "lib/utilities.h"
#include "lib/worker.h"

const int values[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

int calculate(const int *values, int offset, int count) {
  for (int i = offset; i < offset + count; i++)
    printf("%d: %d * 2 = %d\n", i, values[i], values[i] * 2);
  return 0;
}

void test_single_thread() {
  worker_thread_t *thread = worker_thread_create(&calculate, 0);
  if (thread == NULL) {
    printf("Unable to create thread\n");
    return;
  }

  if (worker_thread_start(thread, values, 0, 10)) {
    printf("Unable to start thread\n");
    return;
  }

  int exit_code = worker_thread_wait_for_exit(thread);
  if (exit_code != 0) {
    printf("Failed to run thread, exited with code %d\n", exit_code);
    return;
  }

  worker_thread_free(thread);
}

void test_single_process() {
  worker_process_t *process = worker_process_create(&calculate, 0, 0);
  if (process == NULL) {
    printf("Unable to create process\n");
    return;
  }

  if (worker_process_start(process, values, 0, 10)) {
    printf("Unable to start process\n");
    return;
  }

  int exit_code = worker_process_wait_for_exit(process);
  if (exit_code != 0) {
    printf("Failed to run process, exited with code %d\n", exit_code);
    return;
  }

  worker_process_free(process);
}

int main(int argc, char **argv) {
  int core_count = get_available_cores();
  printf("There are %d logical cores available\n", core_count);

  test_single_thread();
  test_single_process();
}
