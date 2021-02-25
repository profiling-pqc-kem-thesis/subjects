#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/obj_mac.h>

#include "params.h"

#include "dh.h"

#if ECDHE_CURVE == ECDHE_CURVE_P256
#define OPENSSL_ECDHE_CURVE NID_X9_62_prime256v1
#else
#error "incorrect choice for ECDHE_CURVE"
#endif

int crypto_dh_keypair(unsigned char *pk, unsigned char *sk) {
  EC_KEY *key = EC_KEY_new_by_curve_name(OPENSSL_ECDHE_CURVE);
  if (key == NULL)
    return -1;

  if (EC_KEY_generate_key(key) <= 0)
    return -1;

  const BIGNUM *private_key = EC_KEY_get0_private_key(key);
  if (BN_bn2bin(private_key, sk) <= 0)
    return -1;

  const EC_POINT *public_key = EC_KEY_get0_public_key(key);
  if (EC_POINT_point2oct(EC_KEY_get0_group(key), public_key, POINT_CONVERSION_UNCOMPRESSED, pk, ECDHE_PUBLICKEYBYTES, NULL) == 0)
    return -1;

  return 0;
}

int crypto_dh_enc(unsigned char *c, unsigned char *k, unsigned char *ok) {

}

int crypto_dh_dec(unsigned char *k, const unsigned char *c, const unsigned char *sk) {

}
