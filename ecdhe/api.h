#ifndef ECDHE_H
#define ECDHE_H

#include "params.h"

int crypto_dh_keypair(unsigned char *pk, unsigned char *sk);

int crypto_dh_enc(unsigned char *c, unsigned char *k, unsigned char *ok);

int crypto_dh_dec(unsigned char *k, const unsigned char *c, const unsigned char *sk);

#endif
