#ifndef UTILITIES_BENCHMARK_H
#define UTILITIES_BENCHMARK_H

#ifndef BENCHMARK_SUBJECT_NAME
#define BENCHMARK_SUBJECT_NAME "unknown"
#endif

unsigned char *keypair_state();
int keypair();

unsigned char *encrypt_state();
int encrypt();

unsigned char *decrypt_state();
int decrypt();

unsigned char *exchange_state();
int exchange();

int benchmark_sequential();

#endif
