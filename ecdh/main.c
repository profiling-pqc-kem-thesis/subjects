#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/err.h>

#include "api.h"

// Print a byte string as hex
void print_byte_string(unsigned char *byte_string, int size) {
  for (int i = 0; i < size; i++)
    printf("%02X", byte_string[i]);
}

void test_keypair() {
  unsigned char pk[CRYPTO_PUBLICKEYBYTES] = {0};
  unsigned char sk[CRYPTO_SECRETKEYBYTES] = {0};
  if (crypto_dh_keypair(pk, sk) < 0) {
    unsigned long error = ERR_peek_error();
    printf("Unable to generate keypair: %lu\n", error);
    ERR_print_errors_fp(stdout);
    exit(EXIT_FAILURE);
  }

  printf("pk:");
  print_byte_string(pk, CRYPTO_PUBLICKEYBYTES);
  printf("\nsk:");
  print_byte_string(sk, CRYPTO_SECRETKEYBYTES);
  printf("\n");
}

void test_roundtrip() {
  unsigned char alice_pk[CRYPTO_PUBLICKEYBYTES] = {0};
  unsigned char alice_sk[CRYPTO_SECRETKEYBYTES] = {0};
  if (crypto_dh_keypair(alice_pk, alice_sk) < 0) {
    unsigned long error = ERR_peek_error();
    printf("Unable to generate keypair: %lu\n", error);
    ERR_print_errors_fp(stdout);
    exit(EXIT_FAILURE);
  }
  printf("alice_pk:");
  print_byte_string(alice_pk, CRYPTO_PUBLICKEYBYTES);
  printf("\nalice_sk:");
  print_byte_string(alice_sk, CRYPTO_SECRETKEYBYTES);
  printf("\n");

  unsigned char bob_pk[CRYPTO_PUBLICKEYBYTES] = {0};
  unsigned char bob_sk[CRYPTO_SECRETKEYBYTES] = {0};
  if (crypto_dh_keypair(bob_pk, bob_sk) < 0) {
    unsigned long error = ERR_peek_error();
    printf("Unable to generate keypair: %lu\n", error);
    ERR_print_errors_fp(stdout);
    exit(EXIT_FAILURE);
  }
  printf("bob_pk:");
  print_byte_string(bob_pk, CRYPTO_PUBLICKEYBYTES);
  printf("\nbob_sk:");
  print_byte_string(bob_sk, CRYPTO_SECRETKEYBYTES);
  printf("\n");

  unsigned char alice_k[CRYPTO_BYTES] = {0};
  if (crypto_dh_enc(alice_k, alice_sk, bob_pk) < 0) {
    unsigned long error = ERR_peek_error();
    printf("Unable to calculate session key: %lu\n", error);
    ERR_print_errors_fp(stdout);
    exit(EXIT_FAILURE);
  }
  printf("alice_k:");
  print_byte_string(alice_k, CRYPTO_BYTES);
  printf("\n");

  unsigned char bob_k[CRYPTO_BYTES] = {0};
  if (crypto_dh_enc(bob_k, bob_sk, alice_pk) < 0) {
    unsigned long error = ERR_peek_error();
    printf("Unable to calculate session key: %lu\n", error);
    ERR_print_errors_fp(stdout);
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
