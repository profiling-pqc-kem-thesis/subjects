#ifndef ECDHE_H
#define ECDHE_H

int crypto_dh_keypair(unsigned char *pk, unsigned char *sk);

int crypto_dh_enc(unsigned char *k, const unsigned char *sk, const unsigned char *pk);

#endif
