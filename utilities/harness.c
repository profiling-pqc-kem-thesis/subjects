#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "harness.h"
#include "perf/utilities.h"

int assert_support() {
  // Print the kernel version
  int major, minor, patch;
  int status = perf_get_kernel_version(&major, &minor, &patch);
  if (status < 0) {
    perf_print_error(status);
    return 1;
  }

  // Exit if the API is unsupported
  status = perf_is_supported();
  if (status < 0) {
    perf_print_error(status);
    return 1;
  } else if (status == 0) {
    fprintf(stderr, "error: perf not supported\n");
    return 1;
  }

  return 0;
}

int prepare_measurement(perf_measurement_t *measurement, perf_measurement_t *parent_measurement) {
  int status = perf_has_sufficient_privilege(measurement);
  if (status < 0) {
    perf_print_error(status);
    return 1;
  } else if (status == 0) {
    fprintf(stderr, "error: unprivileged user\n");
    return 1;
  }

  int group = parent_measurement == NULL ? -1 : parent_measurement->file_descriptor;

  status = perf_open_measurement(measurement, group, 0);
  if (status < 0) {
    perf_print_error(status);
    return 1;
  }

  return 0;
}

int harness_setup() {
  // Fail if the perf API is unsupported
  if (assert_support())
    return 1;

  // Create a dummy measurement (measures nothing) to act as a group leader
  all_measurements = perf_create_measurement(PERF_TYPE_SOFTWARE, PERF_COUNT_SW_DUMMY, 0, -1);
  prepare_measurement(all_measurements, NULL);

  // Measure the number of retired instructions
  measure_instruction_count = perf_create_measurement(PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS, 0, -1);
  measure_instruction_count->attribute.exclude_kernel = 1;
  prepare_measurement(measure_instruction_count, all_measurements);

  // Measure the number of CPU cycles (at least on Intel CPUs, see https://perf.wiki.kernel.org/index.php/Tutorial#Default_event:_cycle_counting)
  measure_cycle_count = perf_create_measurement(PERF_TYPE_HARDWARE, PERF_COUNT_HW_REF_CPU_CYCLES, 0, -1);
  measure_cycle_count->attribute.exclude_kernel = 1;
  prepare_measurement(measure_cycle_count, all_measurements);

  // Measure the number of context switches
  measure_context_switches = perf_create_measurement(PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CONTEXT_SWITCHES, 0, -1);
  prepare_measurement(measure_context_switches, all_measurements);

  // Measure the CPU clock related to the task (see https://stackoverflow.com/questions/23965363/linux-perf-events-cpu-clock-and-task-clock-what-is-the-difference)
  measure_cpu_clock = perf_create_measurement(PERF_TYPE_SOFTWARE, PERF_COUNT_SW_TASK_CLOCK, 0, -1);
  prepare_measurement(measure_cpu_clock, all_measurements);

  return 0;
}

void harness_print_measurements() {
  printf("    instructions           cycles  context switches           clock\n");
  uint64_t values[5] = {0};
  perf_measurement_t *taken_measurements[] = {all_measurements, measure_instruction_count, measure_cycle_count, measure_context_switches, measure_cpu_clock};

  for (uint64_t i = 0; i < measurements.recorded_values; i++) {
    for (int j = 0; j < 5; j++) {
      if (measurements.values[i].id == taken_measurements[j]->id) {
        values[j] = measurements.values[i].value;
        break;
      }
    }
  }

  // Ignore the results from the dummy counter
  printf("%16" PRIu64 "%16" PRIu64 "%16" PRIu64 "%16" PRIu64 "\n", values[1], values[2], values[3], values[4]);
}

void harness_cleanup() {
  if (all_measurements != NULL)
    free((void *)all_measurements);
  if (measure_instruction_count != NULL)
    free((void *)measure_instruction_count);
  if (measure_cycle_count != NULL)
    free((void *)measure_cycle_count);
  if (measure_context_switches != NULL)
    free((void *)measure_context_switches);
  if (measure_cpu_clock != NULL)
    free((void *)measure_cpu_clock);
}
