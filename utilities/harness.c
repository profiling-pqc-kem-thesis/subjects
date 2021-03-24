#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "harness.h"
#include "perf/utilities.h"

int harness_assert_support() {
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

static int prepare_measurement(perf_measurement_t **measurement, perf_measurement_t *parent_measurement) {
  if (*measurement == NULL)
    return 1;

  int status = perf_has_sufficient_privilege(*measurement);
  if (status < 0) {
    perf_print_error(status);
    *measurement = NULL;
    return 1;
  } else if (status == 0) {
    fprintf(stderr, "error: unprivileged user\n");
    *measurement = NULL;
    return 1;
  }

  int group = parent_measurement == NULL ? -1 : parent_measurement->file_descriptor;

  status = perf_open_measurement(*measurement, group, 0);
  if (status < 0) {
    perf_print_error(status);
    *measurement = NULL;
    return 1;
  }

  return 0;
}

int harness_setup(measurement_group_t *measurement_groups, int count) {
  for (int i = 0; i < count; i++) {
    measurement_group_t *measurement_group = &measurement_groups[i];

    // Create a dummy measurement (measures nothing) to act as a group leader
    measurement_group->parent = perf_create_measurement(PERF_TYPE_SOFTWARE, PERF_COUNT_SW_DUMMY, 0, -1);
    prepare_measurement(&measurement_group->parent, NULL);

    // Measure the number of retired instructions
    measurement_group->instruction_count = perf_create_measurement(PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS, 0, -1);
    measurement_group->instruction_count->attribute.exclude_kernel = 1;
    prepare_measurement(&measurement_group->instruction_count, measurement_group->parent);

    // Measure the number of CPU cycles (at least on Intel CPUs, see https://perf.wiki.kernel.org/index.php/Tutorial#Default_event:_cycle_counting)
    measurement_group->cycle_count = perf_create_measurement(PERF_TYPE_HARDWARE, PERF_COUNT_HW_REF_CPU_CYCLES, 0, -1);
    measurement_group->cycle_count->attribute.exclude_kernel = 1;
    prepare_measurement(&measurement_group->cycle_count, measurement_group->parent);

    // Measure the number of context switches
    measurement_group->context_switches = perf_create_measurement(PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CONTEXT_SWITCHES, 0, -1);
    prepare_measurement(&measurement_group->context_switches, measurement_group->parent);

    // Measure the CPU clock related to the task (see https://stackoverflow.com/questions/23965363/linux-perf-events-cpu-clock-and-task-clock-what-is-the-difference)
    measurement_group->cpu_clock = perf_create_measurement(PERF_TYPE_SOFTWARE, PERF_COUNT_SW_TASK_CLOCK, 0, -1);
    prepare_measurement(&measurement_group->cpu_clock, measurement_group->parent);
  }

  return 0;
}

int harness_cleanup(measurement_group_t *measurement_groups, int count) {
  for (int i = 0; i < count; i++) {
    measurement_group_t *measurement_group = &measurement_groups[i];
    if (measurement_group == NULL)
      continue;

    if (measurement_group->parent != NULL) {
      perf_close_measurement(measurement_group->parent);
      free((void *)measurement_group->parent);
    }

    if (measurement_group->instruction_count != NULL) {
      perf_close_measurement(measurement_group->instruction_count);
      free((void *)measurement_group->instruction_count);
    }

    if (measurement_group->cycle_count != NULL) {
      perf_close_measurement(measurement_group->cycle_count);
      free((void *)measurement_group->cycle_count);
    }

    if (measurement_group->context_switches != NULL) {
      perf_close_measurement(measurement_group->context_switches);
      free((void *)measurement_group->context_switches);
    }

    if (measurement_group->cpu_clock != NULL) {
      perf_close_measurement(measurement_group->cpu_clock);
      free((void *)measurement_group->cpu_clock);
    }
  }

  return 0;
}
