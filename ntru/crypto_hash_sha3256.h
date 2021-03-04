#ifndef CRYPTO_HASH_SHA3256
#define CRYPTO_HASH_SHA3256

#ifdef USE_KECCAK
#include <libkeccak.a.headers/SimpleFIPS202.h>

#define crypto_hash_sha3256 SHA3_256
#define crypto_hash_sha3512 SHA3_512
#define crypto_hash_shake256 SHAKE256

#else
#include "fips202.h"

#define crypto_hash_sha3256 sha3_256
#define crypto_hash_sha3512 sha3_512
#define crypto_hash_shake256 shake256
#endif

#endif
