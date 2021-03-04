#ifdef USE_F
#ifndef crypto_kem_mceliece6960119f_H
#define crypto_kem_mceliece6960119f_H

#define crypto_kem_mceliece6960119f_ref_PUBLICKEYBYTES 1047319
#define crypto_kem_mceliece6960119f_ref_SECRETKEYBYTES 13948
#define crypto_kem_mceliece6960119f_ref_CIPHERTEXTBYTES 226
#define crypto_kem_mceliece6960119f_ref_BYTES 32
 
#ifdef __cplusplus
extern "C" {
#endif
extern int crypto_kem_mceliece6960119f_ref_keypair(unsigned char *,unsigned char *);
extern int crypto_kem_mceliece6960119f_ref_enc(unsigned char *,unsigned char *,const unsigned char *);
extern int crypto_kem_mceliece6960119f_ref_dec(unsigned char *,const unsigned char *,const unsigned char *);
#ifdef __cplusplus
}
#endif

#define crypto_kem_mceliece6960119f_keypair crypto_kem_mceliece6960119f_ref_keypair
#define crypto_kem_mceliece6960119f_enc crypto_kem_mceliece6960119f_ref_enc
#define crypto_kem_mceliece6960119f_dec crypto_kem_mceliece6960119f_ref_dec
#define crypto_kem_mceliece6960119f_PUBLICKEYBYTES crypto_kem_mceliece6960119f_ref_PUBLICKEYBYTES
#define crypto_kem_mceliece6960119f_SECRETKEYBYTES crypto_kem_mceliece6960119f_ref_SECRETKEYBYTES
#define crypto_kem_mceliece6960119f_BYTES crypto_kem_mceliece6960119f_ref_BYTES
#define crypto_kem_mceliece6960119f_CIPHERTEXTBYTES crypto_kem_mceliece6960119f_ref_CIPHERTEXTBYTES

#endif // crypto_kem_mceliece6960119f_H

#else // no USE_F

#ifndef crypto_kem_mceliece6960119_H
#define crypto_kem_mceliece6960119_H

#define crypto_kem_mceliece6960119_ref_PUBLICKEYBYTES 1047319
#define crypto_kem_mceliece6960119_ref_SECRETKEYBYTES 13948
#define crypto_kem_mceliece6960119_ref_CIPHERTEXTBYTES 226
#define crypto_kem_mceliece6960119_ref_BYTES 32
 
#ifdef __cplusplus
extern "C" {
#endif
extern int crypto_kem_mceliece6960119_ref_keypair(unsigned char *,unsigned char *);
extern int crypto_kem_mceliece6960119_ref_enc(unsigned char *,unsigned char *,const unsigned char *);
extern int crypto_kem_mceliece6960119_ref_dec(unsigned char *,const unsigned char *,const unsigned char *);
#ifdef __cplusplus
}
#endif

#define crypto_kem_mceliece6960119_keypair crypto_kem_mceliece6960119_ref_keypair
#define crypto_kem_mceliece6960119_enc crypto_kem_mceliece6960119_ref_enc
#define crypto_kem_mceliece6960119_dec crypto_kem_mceliece6960119_ref_dec
#define crypto_kem_mceliece6960119_PUBLICKEYBYTES crypto_kem_mceliece6960119_ref_PUBLICKEYBYTES
#define crypto_kem_mceliece6960119_SECRETKEYBYTES crypto_kem_mceliece6960119_ref_SECRETKEYBYTES
#define crypto_kem_mceliece6960119_BYTES crypto_kem_mceliece6960119_ref_BYTES
#define crypto_kem_mceliece6960119_CIPHERTEXTBYTES crypto_kem_mceliece6960119_ref_CIPHERTEXTBYTES

#endif // crypto_kem_mceliece6960119_H
#endif // USE_F
