#include <stdio.h>
#include <stdlib.h>

#include <openssl/err.h>

#include "api.h"

void print_byte_string(unsigned char *byte_string, int size) {
  for (int i = 0; i < size; i++)
    printf("%02X", byte_string[i]);
}

int main(int argc, char **argv) {
  unsigned char pk[ECDHE_PUBLICKEYBYTES] = {0};
  unsigned char sk[ECDHE_PRIVATEKEYBYTES] = {0};
  if (crypto_dh_keypair(pk, sk) < 0) {
    unsigned long error = ERR_peek_error();
    printf("Unable to generate keypair: %lu", error);
    ERR_print_errors_fp(stdout);
    return EXIT_FAILURE;
  }

  printf("pk:");
  print_byte_string(pk, ECDHE_PUBLICKEYBYTES);
  printf("\nsk:");
  print_byte_string(sk, ECDHE_PRIVATEKEYBYTES);
  printf("\n");
}
