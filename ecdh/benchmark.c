#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "lib/api.h"

unsigned char *keypair_state() {
  return NULL;
}

int keypair(unsigned char *state) {
  unsigned char pk[CRYPTO_PUBLICKEYBYTES] = {0};
  unsigned char sk[CRYPTO_SECRETKEYBYTES] = {0};
  return crypto_dh_keypair(pk, sk) < 0;
}

unsigned char *exchange_state() {
  return NULL;
}

int exchange(unsigned char *state) {
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
