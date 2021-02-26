#include <openssl/ec.h>
#include <openssl/ecdh.h>
#include <openssl/bn.h>
#include <openssl/sha.h>

#include "params.h"

#include "dh.h"

int crypto_dh_keypair(unsigned char *pk, unsigned char *sk) {
  EC_KEY *key = EC_KEY_new_by_curve_name(OPENSSL_ECDH_CURVE);
  if (key == NULL)
    return -1;

  if (EC_KEY_generate_key(key) <= 0)
    return -1;

  // Write the private key to the output buffer
  const BIGNUM *private_key = EC_KEY_get0_private_key(key);
  if (BN_bn2bin(private_key, sk) <= 0)
    return -1;

  // Write the public key to the output buffer
  const EC_POINT *public_key = EC_KEY_get0_public_key(key);
  if (EC_POINT_point2oct(EC_KEY_get0_group(key), public_key, POINT_CONVERSION_UNCOMPRESSED, pk, CRYPTO_PUBLICKEYBYTES, NULL) == 0)
    return -1;

  EC_KEY_free(key);

  return 0;
}

int crypto_dh_enc(unsigned char *k, const unsigned char *sk, const unsigned char *pk) {
  EC_KEY *key = EC_KEY_new_by_curve_name(OPENSSL_ECDH_CURVE);
  if (key == NULL)
    return -1;

  // Read the private key from the input buffer
  const BIGNUM *private_key = BN_bin2bn(sk, CRYPTO_SECRETKEYBYTES, NULL);
  if (EC_KEY_set_private_key(key, private_key) == 0)
    return -1;

  // Read the public key from the input buffer
  const EC_GROUP *group = EC_KEY_get0_group(key);
  EC_POINT *public_key = EC_POINT_new(group);
  if (EC_POINT_oct2point(group, public_key, pk, CRYPTO_PUBLICKEYBYTES, NULL) == 0)
    return -1;

  // Perform the exchange, calculating the shared secret
  unsigned char secret[CRYPTO_SECRETBYTES] = {0};
  int secret_size = ECDH_compute_key(&secret, CRYPTO_SECRETBYTES, public_key, key, NULL);
  if (secret_size <= 0)
    return -1;

  // Derive the session key
  SHA256_CTX shasum;
  SHA256_Init(&shasum);
  SHA256_Update(&shasum, secret, CRYPTO_SECRETKEYBYTES);
  SHA256_Final(k, &shasum);

  return 0;
}
