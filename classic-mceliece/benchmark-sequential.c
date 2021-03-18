#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "lib/api.h"

void benchmark_keypair() {
  struct timespec start, stop;

  unsigned char pk[CRYPTO_PUBLICKEYBYTES] = {0};
  unsigned char sk[CRYPTO_SECRETKEYBYTES] = {0};

  clock_gettime(CLOCK_MONOTONIC, &start);
  if (crypto_kem_keypair(pk, sk) < 0)
    exit(EXIT_FAILURE);
  clock_gettime(CLOCK_MONOTONIC, &stop);

  printf("%s keypair: %fms\n", CRYPTO_SUBJECT_NAME, ((stop.tv_sec - start.tv_sec) * 1e9 + (stop.tv_nsec - start.tv_nsec)) / 1e6);
}

void benchmark_roundtrip() {
  struct timespec start, stop;

  unsigned char bob_pk[CRYPTO_PUBLICKEYBYTES] = {0};
  unsigned char bob_sk[CRYPTO_SECRETKEYBYTES] = {0};
  if (crypto_kem_keypair(bob_pk, bob_sk) < 0)
    exit(EXIT_FAILURE);

  unsigned char alice_k[CRYPTO_BYTES] = {0};
  unsigned char bob_k[CRYPTO_BYTES] = {0};
  unsigned char c[CRYPTO_CIPHERTEXTBYTES] = {0};

  clock_gettime(CLOCK_MONOTONIC, &start);
  if (crypto_kem_enc(c, bob_k, bob_pk) < 0)
    exit(EXIT_FAILURE);

  if (crypto_kem_dec(bob_k, c, bob_sk) < 0)
    exit(EXIT_FAILURE);
  clock_gettime(CLOCK_MONOTONIC, &stop);

  printf("%s roundtrip: %fms\n", CRYPTO_SUBJECT_NAME, ((stop.tv_sec - start.tv_sec) * 1e9 + (stop.tv_nsec - start.tv_nsec)) / 1e6);
}

int main(int argc, char **argv) {
  benchmark_keypair();
  benchmark_roundtrip();
}
