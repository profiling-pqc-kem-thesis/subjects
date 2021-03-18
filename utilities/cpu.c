#ifdef __APPLE__
#include <sys/sysctl.h>
#else
#include <sys/sysinfo.h>
#endif

#include <stddef.h>

#include "cpu.h"

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
