#ifndef UTILITIES_BENCHMARK_H
#define UTILITIES_BENCHMARK_H

#ifndef BENCHMARK_SUBJECT_NAME
#define BENCHMARK_SUBJECT_NAME "unknown"
#endif

void *get_global_state();

#ifdef INSTRUMENTED
// Called for each sequential benchmarks. Return 0 on success.
int setup_instrumentation();
// Called for each sequential benchmarks. Return 0 on success.
int cleanup_instrumentation();
#endif

int perform_keypair(void *state);
int perform_encrypt(void *state);
int perform_decrypt(void *state);
int perform_exchange(void *state);

int benchmark_sequential(int iterations);
int benchmark_parallel(int thread_count, int duration);

#endif
