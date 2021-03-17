#include "operations.h"

#include "controlbits.h"
#include "randombytes.h"
#include "crypto_hash.h"
#include "encrypt.h"
#include "decrypt.h"
#include "params.h"
#include "sk_gen.h"
#include "pk_gen.h"
#include "util.h"

#include <stdint.h>
#include <string.h>

#if MCELIECE_PARAMETER_SET == MCELIECE_6960119
/* check if the padding bits of pk are all zero */
static int check_pk_padding(const unsigned char * pk)
{
	unsigned char b;
	int i, ret;

	b = 0;
	for (i = 0; i < PK_NROWS; i++)
		b |= pk[i*PK_ROW_BYTES + PK_ROW_BYTES-1];

	b >>= (PK_NCOLS % 8);
	b -= 1;
	b >>= 7;
	ret = b;

	return ret-1;
}
#endif

#if MCELIECE_PARAMETER_SET == MCELIECE_8192128
int crypto_kem_enc(
       unsigned char *c,
       unsigned char *key,
       const unsigned char *pk
)
{
	unsigned char two_e[ 1 + SYS_N/8 ] = {2};
	unsigned char *e = two_e + 1;
	unsigned char one_ec[ 1 + SYS_N/8 + (SYND_BYTES + 32) ] = {1};

	//

	encrypt(c, pk, e);

	crypto_hash_32b(c + SYND_BYTES, two_e, sizeof(two_e)); 

	memcpy(one_ec + 1, e, SYS_N/8);
	memcpy(one_ec + 1 + SYS_N/8, c, SYND_BYTES + 32);

	crypto_hash_32b(key, one_ec, sizeof(one_ec));

	return 0;
}
#elif MCELIECE_PARAMETER_SET == MCELIECE_6960119
int crypto_kem_enc(
       unsigned char *c,
       unsigned char *key,
       const unsigned char *pk
)
{
	unsigned char two_e[ 1 + SYS_N/8 ] = {2};
	unsigned char *e = two_e + 1;
	unsigned char one_ec[ 1 + SYS_N/8 + (SYND_BYTES + 32) ] = {1};
	unsigned char mask;
	int i, padding_ok;

	//

	padding_ok = check_pk_padding(pk);

	encrypt(c, pk, e);

	crypto_hash_32b(c + SYND_BYTES, two_e, sizeof(two_e)); 

	memcpy(one_ec + 1, e, SYS_N/8);
	memcpy(one_ec + 1 + SYS_N/8, c, SYND_BYTES + 32);

	crypto_hash_32b(key, one_ec, sizeof(one_ec));

	// clear outputs (set to all 0's) if padding bits are not all zero

	mask = padding_ok;
	mask ^= 0xFF;

	for (i = 0; i < SYND_BYTES + 32; i++)
		c[i] &= mask;

	for (i = 0; i < 32; i++)
		key[i] &= mask;

	return padding_ok;
}

/* check if the padding bits of c are all zero */
static int check_c_padding(const unsigned char * c)
{
	unsigned char b;
	int ret;

	b = c[ SYND_BYTES-1 ] >> (PK_NROWS % 8);
	b -= 1;
	b >>= 7;
	ret = b;

	return ret-1;
}
#endif

#if MCELIECE_PARAMETER_SET == MCELIECE_8192128
int crypto_kem_dec(
       unsigned char *key,
       const unsigned char *c,
       const unsigned char *sk
)
{
	int i;

	unsigned char ret_confirm = 0;
	unsigned char ret_decrypt = 0;

	uint16_t m;

	unsigned char conf[32];
	unsigned char two_e[ 1 + SYS_N/8 ] = {2};
	unsigned char *e = two_e + 1;
	unsigned char preimage[ 1 + SYS_N/8 + (SYND_BYTES + 32) ];
	unsigned char *x = preimage;
	const unsigned char *s = sk + 40 + IRR_BYTES + COND_BYTES;

	//

	ret_decrypt = decrypt(e, sk + 40, c);

	crypto_hash_32b(conf, two_e, sizeof(two_e)); 

	for (i = 0; i < 32; i++) 
		ret_confirm |= conf[i] ^ c[SYND_BYTES + i];

	m = ret_decrypt | ret_confirm;
	m -= 1;
	m >>= 8;

	*x++ = m & 1;
	for (i = 0; i < SYS_N/8; i++) 
		*x++ = (~m & s[i]) | (m & e[i]);

	for (i = 0; i < SYND_BYTES + 32; i++) 
		*x++ = c[i];

	crypto_hash_32b(key, preimage, sizeof(preimage)); 

	return 0;
}
#elif MCELIECE_PARAMETER_SET == MCELIECE_6960119
int crypto_kem_dec(
       unsigned char *key,
       const unsigned char *c,
       const unsigned char *sk
)
{
	int i, padding_ok;

	unsigned char mask;
	unsigned char ret_confirm = 0;
	unsigned char ret_decrypt = 0;

	uint16_t m;

	unsigned char conf[32];
	unsigned char two_e[ 1 + SYS_N/8 ] = {2};
	unsigned char *e = two_e + 1;
	unsigned char preimage[ 1 + SYS_N/8 + (SYND_BYTES + 32) ];
	unsigned char *x = preimage;
	const unsigned char *s = sk + 40 + IRR_BYTES + COND_BYTES;

	//

	padding_ok = check_c_padding(c);

	ret_decrypt = decrypt(e, sk + 40, c);

	crypto_hash_32b(conf, two_e, sizeof(two_e)); 

	for (i = 0; i < 32; i++) 
		ret_confirm |= conf[i] ^ c[SYND_BYTES + i];

	m = ret_decrypt | ret_confirm;
	m -= 1;
	m >>= 8;

	*x++ = m & 1;
	for (i = 0; i < SYS_N/8; i++) 
		*x++ = (~m & s[i]) | (m & e[i]);

	for (i = 0; i < SYND_BYTES + 32; i++) 
		*x++ = c[i];

	crypto_hash_32b(key, preimage, sizeof(preimage)); 

	// clear outputs (set to all 1's) if padding bits are not all zero

	mask = padding_ok;

	for (i = 0; i < 32; i++)
		key[i] |= mask;

	return padding_ok;
}
#endif

int crypto_kem_keypair
(
       unsigned char *pk,
       unsigned char *sk 
)
{
	int i;
	unsigned char seed[ 33 ] = {64};
	unsigned char r[ SYS_N/8 + (1 << GFBITS)*sizeof(uint32_t) + SYS_T*2 + 32 ];
	unsigned char *rp, *skp;
	#ifdef USE_F
	uint64_t pivots;
	#endif

	gf f[ SYS_T ]; // element in GF(2^mt)
	gf irr[ SYS_T ]; // Goppa polynomial
	uint32_t perm[ 1 << GFBITS ]; // random permutation as 32-bit integers
	int16_t pi[ 1 << GFBITS ]; // random permutation

	randombytes(seed+1, 32);

	while (1)
	{
		rp = &r[ sizeof(r)-32 ];
		skp = sk;

		// expanding and updating the seed

		shake(r, sizeof(r), seed, 33);
		memcpy(skp, seed+1, 32);
		skp += 32 + 8;
		memcpy(seed+1, &r[ sizeof(r)-32 ], 32);

		// generating irreducible polynomial

		rp -= sizeof(f); 

		for (i = 0; i < SYS_T; i++) 
			f[i] = load_gf(rp + i*2); 

		if (genpoly_gen(irr, f)) 
			continue;

		for (i = 0; i < SYS_T; i++)
			store_gf(skp + i*2, irr[i]);

		skp += IRR_BYTES;

		// generating permutation

		rp -= sizeof(perm);

		for (i = 0; i < (1 << GFBITS); i++) 
			perm[i] = load4(rp + i*4); 

		#ifdef USE_F
		if (pk_gen(pk, skp - IRR_BYTES, perm, pi, &pivots))
		#else
		if (pk_gen(pk, skp - IRR_BYTES, perm, pi))
		#endif
			continue;

		controlbitsfrompermutation(skp, pi, GFBITS, 1 << GFBITS);
		skp += COND_BYTES;

		// storing the random string s

		rp -= SYS_N/8;
		memcpy(skp, rp, SYS_N/8);

		// storing positions of the 32 pivots

		#ifdef USE_F
		store8(sk + 32, pivots);
		#else
		store8(sk + 32, 0xFFFFFFFF);
		#endif

		break;
	}

	return 0;
}

