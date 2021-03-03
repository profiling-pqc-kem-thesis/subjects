#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "api.h"

void benchmark_keypair() {
  struct timespec start, stop;

  unsigned char pk[CRYPTO_PUBLICKEYBYTES] = {0};
  unsigned char sk[CRYPTO_SECRETKEYBYTES] = {0};

  clock_gettime(CLOCK_MONOTONIC, &start);
  if (crypto_dh_keypair(pk, sk) < 0)
    exit(EXIT_FAILURE);
  clock_gettime(CLOCK_MONOTONIC, &stop);

  printf("keypair: %fms\n", ((stop.tv_sec - start.tv_sec) * 1e9 + (stop.tv_nsec - start.tv_nsec)) / 1e6);
}

void benchmark_roundtrip() {
  struct timespec start, stop;

  unsigned char alice_pk[CRYPTO_PUBLICKEYBYTES] = {0};
  unsigned char alice_sk[CRYPTO_SECRETKEYBYTES] = {0};
  unsigned char alice_k[CRYPTO_BYTES] = {0};

  unsigned char bob_pk[CRYPTO_PUBLICKEYBYTES] = {0};
  unsigned char bob_sk[CRYPTO_SECRETKEYBYTES] = {0};
  unsigned char bob_k[CRYPTO_BYTES] = {0};

  clock_gettime(CLOCK_MONOTONIC, &start);
  if (crypto_dh_keypair(alice_pk, alice_sk) < 0)
    exit(EXIT_FAILURE);

  if (crypto_dh_keypair(bob_pk, bob_sk) < 0)
    exit(EXIT_FAILURE);

  if (crypto_dh_enc(alice_k, alice_sk, bob_pk) < 0)
    exit(EXIT_FAILURE);

  if (crypto_dh_enc(bob_k, bob_sk, alice_pk) < 0)
    exit(EXIT_FAILURE);
  clock_gettime(CLOCK_MONOTONIC, &stop);

  printf("roundtrip: %fms\n", ((stop.tv_sec - start.tv_sec) * 1e9 + (stop.tv_nsec - start.tv_nsec)) / 1e6);
}

int main(int argc, char **argv) {
  benchmark_keypair();
  benchmark_roundtrip();
}
