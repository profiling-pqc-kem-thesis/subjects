#include <stdio.h>

#include "lib/utilities.h"
#include "lib/worker.h"

const int values[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

void test_single_thread() {
  worker_thread_t *thread = worker_thread_create(values, 0, 0, 10);
  if (thread == NULL) {
    printf("Unable to create thread\n");
    return;
  }

  if (worker_thread_start(thread)) {
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

int main(int argc, char **argv) {
  int core_count = get_available_cores();
  printf("There are %d logical cores available\n", core_count);

  // Run the benchmark in a single core
  test_single_thread();
}
