#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "lib/api.h"
#include "lib/params.h"

unsigned char *keypair_state() {
  return NULL;
}

int keypair(unsigned char *state) {
  unsigned char pk[CRYPTO_PUBLICKEYBYTES] = {0};
  unsigned char sk[CRYPTO_SECRETKEYBYTES] = {0};
  return crypto_kem_keypair(pk, sk) < 0;
}

unsigned char *encrypt_state() {
  unsigned char *keys = malloc(sizeof(unsigned char) * (CRYPTO_PUBLICKEYBYTES + CRYPTO_SECRETKEYBYTES));
  if (keys == NULL)
    return NULL;

  if (crypto_kem_keypair(keys, keys + CRYPTO_PUBLICKEYBYTES) < 0) {
    free(keys);
    return NULL;
  }

  return keys;
}

int encrypt(unsigned char *state) {
  unsigned char c[CRYPTO_CIPHERTEXTBYTES] = {0};
  unsigned char k[CRYPTO_BYTES] = {0};
  if (crypto_kem_enc(c, k, state + CRYPTO_PUBLICKEYBYTES) < 0)
    return 1;

  return 0;
}

unsigned char *decrypt_state() {
  unsigned char *state = malloc(sizeof(unsigned char) * (CRYPTO_PUBLICKEYBYTES + CRYPTO_SECRETKEYBYTES + CRYPTO_CIPHERTEXTBYTES));
  if (state == NULL)
    return NULL;

  if (crypto_kem_keypair(state, state + CRYPTO_PUBLICKEYBYTES) < 0) {
    free(state);
    return NULL;
  }

  unsigned char k[CRYPTO_BYTES] = {0};
  if (crypto_kem_enc(state + CRYPTO_PUBLICKEYBYTES + CRYPTO_SECRETKEYBYTES, k, state) < 0)
    return NULL;

  return state;
}

int decrypt(unsigned char *state) {
  unsigned char k[CRYPTO_BYTES] = {0};
  if (crypto_kem_dec(k, state + CRYPTO_PUBLICKEYBYTES + CRYPTO_SECRETKEYBYTES, state + CRYPTO_PUBLICKEYBYTES) < 0)
    return 1;

  return 1;
}
