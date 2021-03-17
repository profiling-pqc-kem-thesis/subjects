#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/reference/api.h"

// Print a byte string as hex
void print_byte_string(unsigned char *byte_string, int size) {
  for (int i = 0; i < size; i++)
    printf("%02X", byte_string[i]);
}

void test_keypair() {
  unsigned char pk[CRYPTO_PUBLICKEYBYTES] = {0};
  unsigned char sk[CRYPTO_SECRETKEYBYTES] = {0};
  if (crypto_kem_keypair(pk, sk) < 0) {
    printf("Unable to generate keypair\n");
    exit(EXIT_FAILURE);
  }

  printf("pk:");
  print_byte_string(pk, 64);
  printf("...\nsk:");
  print_byte_string(sk, 64);
  printf("...\n");
}

void test_roundtrip() {
  unsigned char alice_pk[CRYPTO_PUBLICKEYBYTES] = {0};
  unsigned char alice_sk[CRYPTO_SECRETKEYBYTES] = {0};
  if (crypto_kem_keypair(alice_pk, alice_sk) < 0) {
    printf("Unable to generate keypair\n");
    exit(EXIT_FAILURE);
  }
  printf("alice_pk:");
  print_byte_string(alice_pk, 64);
  printf("...\nalice_sk:");
  print_byte_string(alice_sk, 64);
  printf("...\n");

  unsigned char bob_pk[CRYPTO_PUBLICKEYBYTES] = {0};
  unsigned char bob_sk[CRYPTO_SECRETKEYBYTES] = {0};
  if (crypto_kem_keypair(bob_pk, bob_sk) < 0) {
    printf("Unable to generate keypair\n");
    exit(EXIT_FAILURE);
  }
  printf("bob_pk:");
  print_byte_string(bob_pk, 64);
  printf("...\nbob_sk:");
  print_byte_string(bob_sk, 64);
  printf("...\n");

  unsigned char c[CRYPTO_CIPHERTEXTBYTES] = {0};
  unsigned char alice_k[CRYPTO_BYTES] = {0};
  if (crypto_kem_enc(c, alice_k, bob_pk) < 0) {
    printf("Unable to generate keypair\n");
    exit(EXIT_FAILURE);
  }
  printf("ciphertext:");
  print_byte_string(c, CRYPTO_CIPHERTEXTBYTES);
  printf("\n");
  printf("alice_k:");
  print_byte_string(alice_k, CRYPTO_BYTES);
  printf("\n");

  unsigned char bob_k[CRYPTO_BYTES] = {0};
  if (crypto_kem_dec(bob_k, c, bob_sk) < 0) {
    printf("Unable to generate keypair\n");
    exit(EXIT_FAILURE);
  }
  printf("bob_k:");
  print_byte_string(bob_k, CRYPTO_BYTES);
  printf("\n");

  for (int i = 0; i < CRYPTO_BYTES; i++) {
    if (alice_k[i] != bob_k[i]) {
      printf("The session keys do not match\n");
      exit(EXIT_FAILURE);
    }
  }
}

int main(int argc, char **argv) {
  test_keypair();
  test_roundtrip();
}
