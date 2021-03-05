#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "lib/api.h"

void benchmark_keypair() {
  struct timespec start, stop;

  DH *dh = DH_new();

  clock_gettime(CLOCK_MONOTONIC, &start);
  if (crypto_dh_keypair(dh) < 0)
    exit(EXIT_FAILURE);
  clock_gettime(CLOCK_MONOTONIC, &stop);

  printf("%s keypair: %fms\n", CRYPTO_SUBJECT_NAME, ((stop.tv_sec - start.tv_sec) * 1e9 + (stop.tv_nsec - start.tv_nsec)) / 1e6);
}

void benchmark_roundtrip() {
  struct timespec start, stop;

  DH *alice_dh = DH_new();
  unsigned char alice_k[CRYPTO_BYTES] = {0};

  DH *bob_dh = DH_new();
  unsigned char bob_k[CRYPTO_BYTES] = {0};

  clock_gettime(CLOCK_MONOTONIC, &start);
  if (crypto_dh_keypair(alice_dh) < 0)
    exit(EXIT_FAILURE);

  if (crypto_dh_keypair(bob_dh) < 0)
    exit(EXIT_FAILURE);

  if (crypto_dh_enc(alice_k, alice_dh, DH_get0_pub_key(bob_dh)) < 0)
    exit(EXIT_FAILURE);

  if (crypto_dh_enc(bob_k, bob_dh, DH_get0_pub_key(alice_dh)) < 0)
    exit(EXIT_FAILURE);
  clock_gettime(CLOCK_MONOTONIC, &stop);

  printf("%s roundtrip: %fms\n", CRYPTO_SUBJECT_NAME, ((stop.tv_sec - start.tv_sec) * 1e9 + (stop.tv_nsec - start.tv_nsec)) / 1e6);
}

int main(int argc, char **argv) {
  benchmark_keypair();
  benchmark_roundtrip();
}
