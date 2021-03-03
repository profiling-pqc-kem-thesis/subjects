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
  DH *dh = DH_new();
  if (crypto_dh_keypair(dh) < 0) {
    unsigned long error = ERR_peek_error();
    printf("Unable to generate keypair: %lu\n", error);
    ERR_print_errors_fp(stdout);
    exit(EXIT_FAILURE);
  }

  const BIGNUM *exportPublic = DH_get0_pub_key(dh);
  printf("pk: %s\n", BN_bn2hex(exportPublic));
  const BIGNUM *exportPrivate = DH_get0_priv_key(dh);
  printf("sk: %s\n", BN_bn2hex(exportPrivate));

  DH_free(dh);
}

void test_roundtrip() {
  DH *alice_dh = DH_new();
  if (crypto_dh_keypair(alice_dh) < 0) {
    unsigned long error = ERR_peek_error();
    printf("Unable to generate keypair: %lu\n", error);
    ERR_print_errors_fp(stdout);
    exit(EXIT_FAILURE);
  }
  const BIGNUM *exportPublic = DH_get0_pub_key(alice_dh);
  printf("alice_pk: %s\n", BN_bn2hex(exportPublic));
  const BIGNUM *exportPrivate = DH_get0_priv_key(alice_dh);
  printf("alice_sk: %s\n", BN_bn2hex(exportPrivate));

  DH *bob_dh = DH_new();
  if (crypto_dh_keypair(bob_dh) < 0) {
    unsigned long error = ERR_peek_error();
    printf("Unable to generate keypair: %lu\n", error);
    ERR_print_errors_fp(stdout);
    exit(EXIT_FAILURE);
  }
  exportPublic = DH_get0_pub_key(bob_dh);
  printf("bob_pk: %s\n", BN_bn2hex(exportPublic));
  exportPrivate = DH_get0_priv_key(bob_dh);
  printf("bob_sk: %s\n", BN_bn2hex(exportPrivate));

  unsigned char alice_k[CRYPTO_BYTES] = {0};
  if (crypto_dh_enc(alice_k, alice_dh, DH_get0_pub_key(bob_dh)) < 0) {
    unsigned long error = ERR_peek_error();
    printf("Unable to calculate session key: %lu\n", error);
    ERR_print_errors_fp(stdout);
    exit(EXIT_FAILURE);
  }
  printf("alice_k:");
  print_byte_string(alice_k, CRYPTO_BYTES);
  printf("\n");

  unsigned char bob_k[CRYPTO_BYTES] = {0};
  if (crypto_dh_enc(bob_k, bob_dh, DH_get0_pub_key(alice_dh)) < 0) {
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

  DH_free(alice_dh);
  DH_free(bob_dh);
}

int main(int argc, char **argv) {
  test_keypair();
  test_roundtrip();
}
