#ifndef ECDH_H
#define ECDH_H

#include "params.h"
#include <openssl/dh.h>

// Generate a keypair
// in dh: an empty allocated DH struct.
// out dh: a DH struct containing dh parameters, public kay, and privet key.
// returns 0 for success and -1 for error.
int crypto_dh_keypair(DH *dh);

// Calculate the shared session key.
// out k: a buffer of at least CRYPTO_BYTES bytes. Will receive the derived session key.
// in dh: a DH struct containing dh parameters, public kay, and privet key.
// in pk: a BIGNUM holding a peer's public key to use for the exchange.
// returns 0 for success and -1 for error.
int crypto_dh_enc(unsigned char *k, DH *dh, const BIGNUM *pk);

#endif
