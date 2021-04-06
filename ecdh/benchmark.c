#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "lib/api.h"

typedef struct {
  unsigned char pk[CRYPTO_PUBLICKEYBYTES];
  unsigned char sk[CRYPTO_SECRETKEYBYTES];
} global_state_t;

void *get_global_state() {
  global_state_t *state = malloc(sizeof(global_state_t));
  if (state == NULL)
    return NULL;

  // Generate a keypair for use when exchaning keys
  if (crypto_dh_keypair(state->pk, state->sk) < 0) {
    free(state);
    return NULL;
  }

  return state;
}

int perform_keypair(unsigned char *state) {
  unsigned char pk[CRYPTO_PUBLICKEYBYTES] = {0};
  unsigned char sk[CRYPTO_SECRETKEYBYTES] = {0};
  return crypto_dh_keypair(pk, sk) < 0;
}

int perform_exchange(unsigned char *state) {
  if (state == NULL)
    return 1;

  // We generate a new key here since we're testing ECDHE, not ECDH
  unsigned char alice_pk[CRYPTO_PUBLICKEYBYTES];
  unsigned char alice_sk[CRYPTO_SECRETKEYBYTES] = {0};
  unsigned char alice_k[CRYPTO_BYTES] = {0};

  if (crypto_dh_keypair(alice_pk, alice_sk) < 0)
    return 1;

  if (crypto_dh_enc(alice_k, alice_sk, ((global_state_t *)state)->pk) < 0)
    return 1;

  return 0;
}
