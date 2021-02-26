#include "params.h"

#if ECDH_CURVE == ECDH_CURVE_P256
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/ecdh.h>
#elif ECDH_CURVE == ECDH_CURVE_25519
#include <openssl/evp.h>
#include <openssl/pem.h>
#endif

#include <openssl/sha.h>

#include "dh.h"

int crypto_dh_keypair(unsigned char *pk, unsigned char *sk) {
#if ECDH_CURVE == ECDH_CURVE_P256
  EC_KEY *key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
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
#elif ECDH_CURVE == ECDH_CURVE_25519
  EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_X25519, NULL);
  if (ctx == NULL)
    return -1;

  if (EVP_PKEY_keygen_init(ctx) == 0)
    return -1;

  EVP_PKEY *key = NULL;
  if (EVP_PKEY_keygen(ctx, &key) == 0)
    return -1;

  EVP_PKEY_CTX_free(ctx);

  size_t private_key_length = CRYPTO_SECRETKEYBYTES;
  if (EVP_PKEY_get_raw_private_key(key, sk, &private_key_length) == 0)
    return -1;

  size_t public_key_length = CRYPTO_PUBLICKEYBYTES;
  if (EVP_PKEY_get_raw_public_key(key, pk, &public_key_length) == 0)
    return -1;

  EVP_PKEY_free(key);

  return 0;
#endif
}

int crypto_dh_enc(unsigned char *k, const unsigned char *sk, const unsigned char *pk) {
#if ECDH_CURVE == ECDH_CURVE_P256
  EC_KEY *key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
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
  SHA256_Update(&shasum, secret, secret_size);
  SHA256_Final(k, &shasum);

  return 0;
#elif ECDH_CURVE == ECDH_CURVE_25519
  // Read the private key from the input buffer
  EVP_PKEY *private_key = EVP_PKEY_new_raw_private_key(NID_X25519, NULL, sk, CRYPTO_SECRETKEYBYTES);
  if (private_key == NULL)
    return -1;

  // Read the public key from the input buffer
  EVP_PKEY *public_key = EVP_PKEY_new_raw_public_key(NID_X25519, NULL, pk, CRYPTO_PUBLICKEYBYTES);
  if (public_key == NULL)
    return -1;

  // Perform the exchange, calculating the shared secret
  unsigned char secret[CRYPTO_SECRETBYTES] = {0};
  size_t secret_size = CRYPTO_SECRETBYTES;
  EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(private_key, NULL);
  if (ctx == NULL)
    return -1;

  if (EVP_PKEY_derive_init(ctx) <= 0)
    return -1;

  if (EVP_PKEY_derive_set_peer(ctx, public_key) <= 0)
    return -1;

  if (EVP_PKEY_derive(ctx, secret, &secret_size) <= 0)
    return -1;

  // Derive the session key
  SHA256_CTX shasum;
  SHA256_Init(&shasum);
  SHA256_Update(&shasum, secret, secret_size);
  SHA256_Final(k, &shasum);

  return 0;
#endif
}
