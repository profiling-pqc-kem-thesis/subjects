#ifdef __APPLE__
#include <sys/sysctl.h>
#else
#include <sys/sysinfo.h>
#endif

#include <stddef.h>

#include "utilities.h"

int get_available_cores() {
#ifdef __APPLE__
  int count;
  size_t count_size = sizeof(count);
  sysctlbyname("hw.logicalcpu", &count, &count_size, NULL, 0);
  return count;
#else
  return get_nprocs();
#endif
}

int calculate_worker_index(int processes, int threads_per_process, int process_index, int thread_index) {
  // Support multi-process, multi-threading and multi-threading in multi-process
  if (processes == 0)
    return thread_index;
  if (threads_per_process == 0)
    return process_index;
  return process_index * threads_per_process + thread_index;
}


int calculate_worker_count(int processes, int threads_per_process) {
  // Support multi-process, multi-threading and multi-threading in multi-process
  int worker_count = 0;
  if (processes == 0)
    worker_count = threads_per_process;
  else if (threads_per_process == 0)
    worker_count = processes;
  else
    worker_count = processes * threads_per_process;
  if (worker_count == 0)
    worker_count = 1;
  return worker_count;
}
