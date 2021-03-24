#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "harness.h"

#include "lib/api.h"

void *get_global_state() {
  return NULL;
}

#ifdef INSTRUMENTED
int setup_instrumentation() {
  return harness_setup(measurement_groups, NUMBER_OF_MEASUREMENTS);
}

int cleanup_instrumentation() {
  return harness_cleanup(measurement_groups, NUMBER_OF_MEASUREMENTS);
}
#endif

int perform_keypair(unsigned char *state) {
  unsigned char pk[CRYPTO_PUBLICKEYBYTES] = {0};
  unsigned char sk[CRYPTO_SECRETKEYBYTES] = {0};
  return crypto_dh_keypair(pk, sk) < 0;
}

int perform_exchange(unsigned char *state) {
  unsigned char alice_pk[CRYPTO_PUBLICKEYBYTES];
  unsigned char alice_sk[CRYPTO_SECRETKEYBYTES] = {0};
  unsigned char alice_k[CRYPTO_BYTES] = {0};

  unsigned char bob_pk[CRYPTO_PUBLICKEYBYTES] = {0};
  unsigned char bob_sk[CRYPTO_SECRETKEYBYTES] = {0};
  unsigned char bob_k[CRYPTO_BYTES] = {0};

  if (crypto_dh_keypair(alice_pk, alice_sk) < 0)
    return 1;

  if (crypto_dh_keypair(bob_pk, bob_sk) < 0)
    return 1;

  if (crypto_dh_enc(alice_k, alice_sk, bob_pk) < 0)
    return 1;

  if (crypto_dh_enc(bob_k, bob_sk, alice_pk) < 0)
    return 1;

  return 0;
}
