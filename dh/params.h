#ifndef PARAMS_H
#define PARAMS_H

#include <openssl/obj_mac.h>

#define CRYPTO_ALGNAME "DH"

// The number of bytes required to store the derived session key
#define CRYPTO_BYTES 32
// The number of bytes required to store the plain session key
#define CRYPTO_SECRETBYTES 256

#endif
