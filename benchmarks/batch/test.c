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
  int worker_count = calculate_worker_count(calculate_state->processes, calculate_state->threads_per_process);
  int worker_index = calculate_worker_index(calculate_state->processes, calculate_state->threads_per_process, process_index, thread_index);

  int operations_per_thread = INPUT_COUNT / worker_count;
  int offset = worker_index * operations_per_thread;
  for (int i = offset; i < offset + operations_per_thread; i++) {
    calculate_state->outputs[i] = calculate_state->inputs[i] * 2;
  }
  return 0;
}

void test_pool(int process_count, int thread_count) {
  worker_pool_t *pool = worker_pool_create(&calculate, 0, 1);
  if (pool == NULL) {
    printf("Unable to create pool\n");
    return;
  }

  state_t *state = prepare_state(0, 1);
  if (state == NULL) {
    printf("Unable to create state for thread\n");
    worker_pool_free(pool);
    return;
  }

  if (worker_pool_start(pool, state)) {
    printf("Unable to start pool\n");
    return;
  }

  int exit_code = worker_pool_wait_for_exit(pool);
  if (exit_code != 0) {
    printf("Failed to run pool, exited with code %d\n", exit_code);
    return;
  }

  worker_pool_free(pool);
  print_results(state);
  state_free(state);
}

int main(int argc, char **argv) {
  int core_count = get_available_cores();
  printf("There are %d logical cores available\n", core_count);

  printf("===== 0 processes, 0 threads (raw) =====\n");
  test_pool(0, 0);
  printf("===== 0 processes, 1 thread        =====\n");
  test_pool(0, 1);
  printf("===== 1 process,   0 threads       =====\n");
  test_pool(1, 0);
  printf("===== 4 processes, 2 threads       =====\n");
  test_pool(1, 1);
}
