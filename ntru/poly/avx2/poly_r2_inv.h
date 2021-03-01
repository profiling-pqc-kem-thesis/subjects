#ifndef POLY_R2_INV_H
#define POLY_R2_INV_H

#include "../../poly.h"

void poly_R2_tobytes(unsigned char *out, const poly *a);
void poly_R2_frombytes(poly *a, const unsigned char *in);

#if NTRU_PARAMETER_SET == NTRU_HRSS_701
extern void square_1_701(unsigned char* out, const unsigned char* a);
extern void square_3_701(unsigned char* out, const unsigned char* a);
extern void square_6_701(unsigned char* out, const unsigned char* a);
extern void square_12_701(unsigned char* out, const unsigned char* a);
extern void square_15_701(unsigned char* out, const unsigned char* a);
extern void square_27_701(unsigned char* out, const unsigned char* a);
extern void square_42_701(unsigned char* out, const unsigned char* a);
extern void square_84_701(unsigned char* out, const unsigned char* a);
extern void square_168_701(unsigned char* out, const unsigned char* a);
extern void square_336_701(unsigned char* out, const unsigned char* a);
#elif NTRU_PARAMETER_SET == NTRU_HPS_4096821
extern void square_1_821(unsigned char *out, const unsigned char *a);
extern void square_3_821(unsigned char *out, const unsigned char *a);
extern void square_6_821(unsigned char *out, const unsigned char *a);
extern void square_12_821(unsigned char *out, const unsigned char *a);
extern void square_24_821(unsigned char *out, const unsigned char *a);
extern void square_51_821(unsigned char *out, const unsigned char *a);
extern void square_102_821(unsigned char *out, const unsigned char *a);
extern void square_204_821(unsigned char *out, const unsigned char *a);
extern void square_408_821(unsigned char *out, const unsigned char *a);
#else
#error "Unsupported NTRU_PARAMETER_SET"
#endif

extern void poly_R2_mul(unsigned char* out, const unsigned char* a,
                                            const unsigned char* b);
#endif
