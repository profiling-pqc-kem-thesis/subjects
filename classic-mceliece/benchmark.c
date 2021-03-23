#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "lib/api.h"
#include "lib/params.h"

typedef struct {
  unsigned char pk[CRYPTO_PUBLICKEYBYTES];
  unsigned char sk[CRYPTO_SECRETKEYBYTES];
  unsigned char c[CRYPTO_CIPHERTEXTBYTES];
} global_state_t;

void *get_global_state() {
  global_state_t *state = malloc(sizeof(global_state_t));
  if (state == NULL)
    return NULL;

  // Generate a keypair for use when decrypting and encrypting
  if (crypto_kem_keypair(state->pk, state->sk) < 0) {
    free(state);
    return NULL;
  }

  // Perform encryption to get a ciphertext for decryption
  unsigned char k[CRYPTO_BYTES] = {0};
  if (crypto_kem_enc(state->c, k, state->pk) < 0)
    return NULL;

  return state;
}

int perform_keypair(void *state) {
  unsigned char pk[CRYPTO_PUBLICKEYBYTES] = {0};
  unsigned char sk[CRYPTO_SECRETKEYBYTES] = {0};
  return crypto_kem_keypair(pk, sk) != 0;
}

int perform_encrypt(void *state) {
  if (state == NULL)
    return 1;

  unsigned char c[CRYPTO_CIPHERTEXTBYTES] = {0};
  unsigned char k[CRYPTO_BYTES] = {0};
  return crypto_kem_enc(c, k, ((global_state_t *)state)->pk) != 0;
}

int perform_decrypt(unsigned char *state) {
  if (state == NULL)
    return 1;

  unsigned char k[CRYPTO_BYTES] = {0};
  return crypto_kem_dec(k, ((global_state_t *)state)->c, ((global_state_t *)state)->sk) != 0;
}
