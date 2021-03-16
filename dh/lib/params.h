#ifndef PARAMS_H
#define PARAMS_H

#include <openssl/obj_mac.h>

#define CRYPTO_ALGNAME "DH"
#define CRYPTO_SUBJECT_NAME "DH"

// The number of bytes required to store the derived session key
#define CRYPTO_BYTES 32
// The number of bytes required to store the plain session key
#define CRYPTO_SECRETBYTES 256
// The number of bytes required to store a public key
#define CRYPTO_PUBLICKEYBYTES 255
// The number of bytes required to store a private key
#define CRYPTO_SECRETKEYBYTES 255

#define CRYPTO_PARAM_P_BYTES 256
#define CRYPTO_PARAM_G_BYTES 1

#endif
