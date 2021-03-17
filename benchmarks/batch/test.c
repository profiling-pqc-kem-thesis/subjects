#include <stdio.h>

#include "lib/utilities.h"
#include "lib/worker.h"
#include "lib/state.h"

#define INPUT_COUNT 10

typedef struct {
  int processes;
  int threads_per_process;
  int inputs[INPUT_COUNT];
  int outputs[INPUT_COUNT];
} calculate_state_t;

const int input[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

state_t *prepare_state(int processes, int threads_per_process) {
  // Create a shared state
  state_t *state = state_create(sizeof(calculate_state_t));
  if (state == NULL)
    return NULL;

  // Map the shared state to a state we control
  calculate_state_t *calculate_state = (calculate_state_t *)state->memory;
  calculate_state->processes = processes;
  calculate_state->threads_per_process = threads_per_process;

  // Initialize the inputs
  for (int i = 0; i < INPUT_COUNT; i++)
    calculate_state->inputs[i] = i;

  return state;
};

void print_results(state_t *state) {
  // Map the shared state to a state we control
  calculate_state_t *calculate_state = (calculate_state_t *)state->memory;
  for (int i = 0; i < INPUT_COUNT; i++)
    printf("%d: %d -> %d\n", i, calculate_state->inputs[i], calculate_state->outputs[i]);
}

int calculate(int process_index, int thread_index, state_t *state) {
  calculate_state_t *calculate_state = (calculate_state_t *)state->memory;
  int worker_count = 0;
  int worker_index = 0;

  calculate_worker_index(calculate_state->processes, calculate_state->threads_per_process, process_index, thread_index, &worker_count, &worker_index);

  int operations_per_thread = INPUT_COUNT / worker_count;
  int offset = worker_index * operations_per_thread;
  for (int i = offset; i < offset + operations_per_thread; i++) {
    calculate_state->outputs[i] = calculate_state->inputs[i] * 2;
  }
  return 0;
}

void test_single_thread() {
  worker_thread_t *thread = worker_thread_create(&calculate, 0, 0);
  if (thread == NULL) {
    printf("Unable to create thread\n");
    return;
  }

  state_t *state = prepare_state(0, 1);
  if (state == NULL) {
    printf("Unable to create state for thread\n");
    worker_thread_free(thread);
    return;
  }

  if (worker_thread_start(thread, state)) {
    printf("Unable to start thread\n");
    return;
  }

  int exit_code = worker_thread_wait_for_exit(thread);
  if (exit_code != 0) {
    printf("Failed to run thread, exited with code %d\n", exit_code);
    return;
  }

  worker_thread_free(thread);
  print_results(state);
  state_free(state);
}

void test_single_process() {
  worker_process_t *process = worker_process_create(&calculate, 0, 0);
  if (process == NULL) {
    printf("Unable to create process\n");
    return;
  }

  state_t *state = prepare_state(1, 0);
  if (state == NULL) {
    printf("Unable to create state for process\n");
    worker_process_free(process);
    return;
  }

  if (worker_process_start(process, state)) {
    printf("Unable to start process\n");
    return;
  }

  int exit_code = worker_process_wait_for_exit(process);
  if (exit_code != 0) {
    printf("Failed to run process, exited with code %d\n", exit_code);
    return;
  }

  worker_process_free(process);
  print_results(state);
  state_free(state);
}

void test_multiple_processes() {
  state_t *state = prepare_state(5, 0);
  if (state == NULL) {
    printf("Unable to create state for processes\n");
    return;
  }

  worker_process_t *processes[5] = {0};
  for (int i = 0; i < 5; i++) {
    processes[i] = worker_process_create(&calculate, i, 0);
    if (processes[i] == NULL) {
      printf("Unable to create process\n");
      return;
    }

    if (worker_process_start(processes[i], state)) {
      printf("Unable to start process\n");
      return;
    }
  }

  for (int i = 0; i < 5; i++) {
    int exit_code = worker_process_wait_for_exit(processes[i]);
    if (exit_code != 0) {
      printf("Failed to run process, exited with code %d\n", exit_code);
      return;
    }

    worker_process_free(processes[i]);
  }
  print_results(state);
  state_free(state);
}

int main(int argc, char **argv) {
  int core_count = get_available_cores();
  printf("There are %d logical cores available\n", core_count);

  test_single_thread();
  test_single_process();
  test_multiple_processes();
}
