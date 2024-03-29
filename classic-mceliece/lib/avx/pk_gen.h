/*
  This file is for public-key generation
*/

#ifndef PK_GEN_H
#define PK_GEN_H

#include <stdint.h>
#ifdef USE_F
int pk_gen(unsigned char *, const unsigned char *, uint32_t *, int16_t *, uint64_t *);
#else
int pk_gen(unsigned char *, const unsigned char *, uint32_t *, int16_t *);
#endif
#endif
