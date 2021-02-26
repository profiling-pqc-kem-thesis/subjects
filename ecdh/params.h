#ifndef PARAMS_H
#define PARAMS_H

#include <openssl/obj_mac.h>

#define CRYPTO_ALGNAME "ECDH"

// Enum for available eliptic curves
#define ECDH_CURVE_P256 1

// The eliptic curve to use
#define ECDH_CURVE ECDH_CURVE_P256

// Define the OpenSSL equivalent of the chosen curve
#if ECDH_CURVE == ECDH_CURVE_P256
#define OPENSSL_ECDH_CURVE NID_X9_62_prime256v1
#else
#error "incorrect choice for ECDH_CURVE"
#endif

// The number of bytes required to store a public key
#define CRYPTO_PUBLICKEYBYTES 65
// The number of bytes required to store a secret key
#define CRYPTO_SECRETKEYBYTES 32
// The number of bytes required to store the derived session key
#define CRYPTO_BYTES 32
// The number of bytes required to store the plain session key
#define CRYPTO_SECRETBYTES 32

#endif
