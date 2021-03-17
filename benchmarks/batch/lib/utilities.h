#ifndef UTILITIES_H
#define UTILITIES_H

// Returns the number of available logical cores
int get_available_cores();

void calculate_worker_index(int processes, int threads_per_process, int process_index, int thread_index, int *worker_count, int *worker_index);

#endif
