#include "time.h"

unsigned long long timespec_to_nanoseconds(const struct timespec *spec) {
  return spec->tv_sec * 1e9 + spec->tv_nsec;
}

unsigned long long timespec_to_duration(const struct timespec *start, const struct timespec *stop) {
  return timespec_to_nanoseconds(stop) - timespec_to_nanoseconds(start);
}
