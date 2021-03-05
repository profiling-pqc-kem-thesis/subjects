#ifndef PARAMS_H
#define PARAMS_H

#include <openssl/obj_mac.h>

#define CRYPTO_ALGNAME "ECDH"

// Enum for available elliptic curves
#define ECDH_CURVE_P256 1
#define ECDH_CURVE_25519 2

// The elliptic curve to use.
#ifndef ECDH_CURVE
#define ECDH_CURVE ECDH_CURVE_25519
#endif

// The number of bytes required to store a public key
#if ECDH_CURVE == ECDH_CURVE_P256
#define CRYPTO_PUBLICKEYBYTES 65
#define CRYPTO_SUBJECT_NAME "ECDH P-256"
#elif ECDH_CURVE == ECDH_CURVE_25519
#define CRYPTO_PUBLICKEYBYTES 32
#define CRYPTO_SUBJECT_NAME "ECDH Curve25519"
#else
#error "Unsupported ECDH_CURVE"
#endif

// The number of bytes required to store a secret key
#define CRYPTO_SECRETKEYBYTES 32
// The number of bytes required to store the derived session key
#define CRYPTO_BYTES 32
// The number of bytes required to store the plain session key
#define CRYPTO_SECRETBYTES 32

#endif
