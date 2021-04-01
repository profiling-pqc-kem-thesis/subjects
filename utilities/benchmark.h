#ifndef UTILITIES_BENCHMARK_H
#define UTILITIES_BENCHMARK_H

#ifndef BENCHMARK_SUBJECT_NAME
#define BENCHMARK_SUBJECT_NAME "unknown"
#endif

void *get_global_state();
int perform_keypair(void *state);
int perform_encrypt(void *state);
int perform_decrypt(void *state);
int perform_exchange(void *state);

int benchmark_sequential(int iterations, int timeout, int benchmark_keypair, int benchmark_encrypt, int benchmark_decrypt, int benchmark_exchange);
int benchmark_parallel(int iterations, int thread_count, int timeout, int benchmark_keypair, int benchmark_encrypt, int benchmark_decrypt, int benchmark_exchange);

#endif
