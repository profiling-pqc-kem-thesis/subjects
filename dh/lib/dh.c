#include "params.h"

#include <openssl/dh.h>
#include <openssl/sha.h>

#include "dh.h"

int crypto_dh_keypair(unsigned char *pk, unsigned char *sk, unsigned char *p, unsigned char *g) {
  int return_value = -1;

  DH *dh = NULL;

  dh = DH_new();
  if (dh == NULL)
    goto cleanup;
  DH_set0_pqg(dh, BN_bin2bn(p, CRYPTO_PARAM_P_BYTES, NULL), NULL, BN_bin2bn(g, CRYPTO_PARAM_G_BYTES, NULL));

  if (DH_generate_key(dh) == 0)
    goto cleanup;

  BIGNUM *public_key;
  BIGNUM *private_key;
  DH_get0_key(dh, (const BIGNUM **)&public_key, (const BIGNUM **)&private_key);

  if (BN_bn2bin(private_key, sk) <= 0)
    goto cleanup;

  if (BN_bn2bin(public_key, pk) <= 0)
    goto cleanup;

  return_value = 0;

cleanup:
  if (dh != NULL)
    DH_free(dh);

  return return_value;
}

int crypto_dh_enc(unsigned char *k, const unsigned char *sk, const unsigned char *pk, unsigned char *p, unsigned char *g) {
  int return_value = 0;
  DH *dh = NULL;
  BIGNUM *private_key = NULL;
  BIGNUM *public_key = NULL;

  dh = DH_new();
  if (dh == NULL)
    goto cleanup;
  DH_set0_pqg(dh, BN_bin2bn(p, CRYPTO_PARAM_P_BYTES, NULL), NULL, BN_bin2bn(g, CRYPTO_PARAM_G_BYTES, NULL));

  private_key = BN_bin2bn(sk, CRYPTO_SECRETKEYBYTES, NULL);
  if (private_key == NULL)
    goto cleanup;
  if (DH_set0_key(dh, NULL, private_key) != 1)
    goto cleanup;

  public_key = BN_bin2bn(pk, CRYPTO_SECRETKEYBYTES, NULL);
  if (public_key == NULL)
    goto cleanup;

  // Perform the exchange, calculating the shared secret
  unsigned char secret[CRYPTO_SECRETBYTES] = {0};
  if (DH_compute_key(secret, public_key, dh) == -1) {
    goto cleanup;
  }

  // Derive the session key
  SHA256_CTX shasum;
  SHA256_Init(&shasum);
  SHA256_Update(&shasum, secret, CRYPTO_SECRETBYTES);
  SHA256_Final(k, &shasum);

  return_value = 0;

cleanup:
  // dh owns private_key
  if (dh != NULL)
    DH_free(dh);

  if (public_key != NULL)
    BN_free(public_key);

  return return_value;
}
