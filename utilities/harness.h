#ifndef UTILITIES_HARNESS_H
#define UTILITIES_HARNESS_H

#ifdef INSTRUMENTED

#include <stdint.h>

#include <perf/utilities.h>

// This structure is returned each time the main measurement is read.
// It contains the result of all the measurements, taken simultaneously.
// The order of the measurements is unknown - use the ID.
typedef struct {
  uint64_t recorded_values;
  struct {
    uint64_t value;
    uint64_t id;
  } values[5];
} measurement_t;

typedef struct {
  // The group parent
  perf_measurement_t *parent;
  // Retired instructions. Be careful, these can be affected by various issues, most notably hardware interrupt counts.
  perf_measurement_t *instruction_count;
  // Total cycles; not affected by CPU frequency scaling.
  perf_measurement_t *cycle_count;
  // This counts context switches.  Until Linux 2.6.34, these were all reported as user-space events, after that they are reported as happening in the kernel.
  perf_measurement_t *context_switches;
  // This reports the CPU clock, a high-resolution per-CPU timer.
  // See also: https://stackoverflow.com/questions/23965363/linux-perf-events-cpu-clock-and-task-clock-what-is-the-difference.
  perf_measurement_t *cpu_clock;
} measurement_group_t;

measurement_t measurements;

int harness_assert_support();

// Called before each use
int harness_setup(measurement_group_t *measurement_groups, int count);
// Called after each use
int harness_cleanup(measurement_group_t *measurement_groups, int count);

#define IF_INSTRUMENTED_start_measurement(X) perf_start_measurement(X->parent)
#define IF_INSTRUMENTED_stop_measurement(X)                         \
  do {                                                              \
    perf_stop_measurement(X->parent);                                       \
    perf_read_measurement(X->parent, &measurements, sizeof(measurement_t)); \
  } while (0)

#else
#define IF_INSTRUMENTED_start_measurement(X)
#define IF_INSTRUMENTED_stop_measurement(X)
#endif

#endif
