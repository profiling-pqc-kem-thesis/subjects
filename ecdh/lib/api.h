#ifndef ECDH_H
#define ECDH_H

#include "params.h"

// Generate a keypair
// out pk: a buffer of at least ECDH_PUBLICKEYBYTES bytes. Will receive the generated public key.
// out sk: a buffer of at least ECDH_PRIVATEKEYBYTES bytes. Will receive the generated private key.
// returns 0 for success and -1 for error.
int crypto_dh_keypair(unsigned char *pk, unsigned char *sk);

// Calculate the shared session key.
// out k: a buffer of at least CRYPTO_BYTES bytes. Will receive the derived session key.
// in sk: a buffer holding a secret key to use for the exchange.
// in pk: a buffer holding a peer's public key to use for the exchange.
// returns 0 for success and -1 for error.
int crypto_dh_enc(unsigned char *k, const unsigned char *sk, const unsigned char *pk);

#endif
