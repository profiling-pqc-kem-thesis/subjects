#include <sys/sysctl.h>

#include "utilities.h"

int get_available_cores() {
  int count;
  size_t count_size = sizeof(count);
  sysctlbyname("hw.logicalcpu", &count, &count_size, NULL, 0);
  return count;
}

void calculate_worker_index(int processes, int threads_per_process, int process_index, int thread_index, int *worker_count, int *worker_index) {
  // Support multi-process, multi-threading and multi-threading in multi-process
  if (processes == 0 && threads_per_process == 0) {
    *worker_count = 1;
    *worker_index = 0;
  } else if (processes == 0) {
    *worker_count = threads_per_process;
    *worker_index = thread_index;
  } else if (threads_per_process == 0) {
    *worker_count = processes;
    *worker_index = process_index;
  } else {
    *worker_count = processes * threads_per_process;
    *worker_index = process_index * threads_per_process + thread_index;
  }
  if (*worker_count == 0)
    *worker_count = 1;
}
