#include <stddef.h>
#include <stdlib.h>

#include "lib/api.h"

typedef struct {
  unsigned char pk[CRYPTO_PUBLICKEYBYTES];
  unsigned char sk[CRYPTO_SECRETKEYBYTES];
} global_state_t;

static unsigned char p[] = {
    0x00, 0xf4, 0xb3, 0xe8, 0xce, 0x2a, 0xd6, 0xbd, 0xce, 0xe1, 0x72, 0x7c, 0xdf, 0xc9, 0x11, 0x0e, 0x36, 0x6b, 0x41,
    0x46, 0x60, 0x1a, 0x01, 0xaa, 0x17, 0x3f, 0xbc, 0xc6, 0x74, 0x61, 0xbd, 0x03, 0x8a, 0xa2, 0xe0, 0x08, 0xd8, 0x91,
    0xbb, 0xcc, 0x9e, 0xc2, 0xb5, 0x4c, 0xad, 0x17, 0xae, 0x23, 0xee, 0x95, 0x03, 0x19, 0x13, 0xe6, 0x7d, 0x1c, 0xe6,
    0x05, 0xb6, 0xe7, 0x97, 0x62, 0x45, 0xdb, 0x50, 0x01, 0x25, 0x4a, 0x20, 0xde, 0x3b, 0x3a, 0x81, 0xdc, 0xe5, 0xac,
    0xd6, 0x89, 0xe1, 0xed, 0xc2, 0xa0, 0x17, 0x12, 0xfe, 0xcb, 0xb7, 0xa2, 0x6a, 0xf5, 0x22, 0x0c, 0xca, 0x44, 0xce,
    0xde, 0x95, 0x76, 0x92, 0x18, 0xf5, 0xbd, 0xa8, 0x30, 0xc0, 0x02, 0xa0, 0x63, 0xc6, 0xbf, 0x1d, 0x9d, 0x85, 0xc7,
    0x2b, 0xb7, 0xce, 0x41, 0x4d, 0x62, 0xee, 0xfb, 0xcb, 0x3e, 0x97, 0x21, 0x1e, 0x88, 0x69, 0xe8, 0x29, 0x2d, 0x37,
    0x25, 0x6a, 0x61, 0x22, 0x95, 0xbc, 0x22, 0xcc, 0x38, 0xd0, 0x66, 0xa3, 0x86, 0x14, 0x2c, 0x57, 0xc9, 0x8d, 0x51,
    0xc7, 0x8b, 0xa6, 0xc2, 0x5e, 0xc8, 0x88, 0x92, 0x41, 0xe8, 0x29, 0xf0, 0x61, 0x6f, 0x15, 0xf8, 0xbc, 0x61, 0xce,
    0xa8, 0x1e, 0x9e, 0xa7, 0xe9, 0x35, 0x12, 0xb2, 0x21, 0x80, 0x19, 0x0e, 0xb2, 0xfd, 0x89, 0xbf, 0xf2, 0x53, 0x96,
    0x01, 0xcb, 0x70, 0xc9, 0xb2, 0x2d, 0x1d, 0xe5, 0x3e, 0xcc, 0x03, 0xcb, 0x71, 0xc3, 0xed, 0x13, 0x73, 0xd0, 0xae,
    0x05, 0x3b, 0x90, 0xf3, 0x18, 0x78, 0xf0, 0x66, 0xae, 0x7e, 0xcb, 0x85, 0x9a, 0x70, 0x01, 0xab, 0x2a, 0xb2, 0xe7,
    0xad, 0x2c, 0xf5, 0x24, 0xa5, 0xf9, 0x4a, 0xfb, 0x5e, 0xf5, 0x86, 0xc2, 0x99, 0xb5, 0x9a, 0x13, 0x9f, 0x39, 0x2a,
    0x47, 0x74, 0xd8, 0x3b, 0xdb, 0x40, 0xde, 0xcd, 0x2d, 0xdb};
static unsigned char g[] = {0x02};

void *get_global_state() {
  global_state_t *state = malloc(sizeof(global_state_t));
  if (state == NULL)
    return NULL;

  // Generate a keypair for use when exchaning keys
  if (crypto_dh_keypair(state->pk, state->sk, p, g) < 0) {
    free(state);
    return NULL;
  }

  return state;
}

int perform_keypair(unsigned char *state) {
  unsigned char pk[CRYPTO_PUBLICKEYBYTES] = {0};
  unsigned char sk[CRYPTO_SECRETKEYBYTES] = {0};
  return crypto_dh_keypair(pk, sk, p, g) < 0;
}

int perform_exchange(unsigned char *state) {
  if (state == NULL)
    return 1;

  // We generate a new key here since we're testing DHE, not DH
  unsigned char alice_pk[CRYPTO_PUBLICKEYBYTES];
  unsigned char alice_sk[CRYPTO_SECRETKEYBYTES] = {0};
  unsigned char alice_k[CRYPTO_BYTES] = {0};

  if (crypto_dh_keypair(alice_pk, alice_sk, p, g) < 0)
    return 1;

  if (crypto_dh_enc(alice_k, alice_sk, ((global_state_t *)state)->pk, p, g) < 0)
    return 1;

  return 0;
}
