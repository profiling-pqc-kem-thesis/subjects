#include "params.h"

#include <openssl/dh.h>
#include <openssl/sha.h>

#include "dh.h"

int crypto_dh_keypair(DH *dh) {
  // todo The pseudo-random number generator "must" be seeded before calling it
  if (DH_generate_parameters_ex(dh, 2024, 2, NULL) == 0) // todo: alice and bob do not get the same p and g now
    return -1;

  /*if ((dh = DH_get_2048_256()) == NULL) // todo: why this not work. should replace code above
    return -1;*/

  if (DH_generate_key(dh) == 0)
    return -1;

  return 0;
}

int crypto_dh_enc(unsigned char *k, DH *dh, const BIGNUM *pk) {
  // Perform the exchange, calculating the shared secret
  size_t secret_size = 0;
  unsigned char secret[256] = {0}; // todo: should this be hard coded size?
  if ((secret_size = DH_compute_key(secret, pk, dh)) == -1) {
    return -1;
  }

  // Derive the session key
  SHA256_CTX shasum;
  SHA256_Init(&shasum);
  SHA256_Update(&shasum, secret, secret_size);
  SHA256_Final(k, &shasum);

  return 0;
}
