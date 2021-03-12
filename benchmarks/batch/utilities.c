#include <sys/sysctl.h>

#include "utilities.h"

int get_available_cores() {
  int count;
  size_t count_size = sizeof(count);
  sysctlbyname("hw.logicalcpu", &count, &count_size, NULL, 0);
  return count;
}
