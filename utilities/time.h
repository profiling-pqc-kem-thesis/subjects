#ifndef UTILITIES_TIME_H
#define UTILITIES_TIME_H

#include <time.h>

unsigned long long timespec_to_nanoseconds(const struct timespec *spec);
unsigned long long timespec_to_duration(const struct timespec *start, const struct timespec *stop);
unsigned long long timespec_now();

#endif
