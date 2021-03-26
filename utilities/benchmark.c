#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cpu.h"

#include "benchmark.h"

void die_with_usage(char *name) {
  printf("usage: %s [flags]\n", name);
  printf("flags:\n");
  printf("--help             print this help text\n");
  printf("--parallel         perform the parallel benchmark\n");
  printf("--sequential       perform the sequential benchmark\n");
  printf("--iterations <n>   iterations of the sequential benchmark\n");
  printf("--duration <n>     total time in seconds to perform the parallel benchmark\n");
  printf("--thread-count <n> number of threads to use in the parallel benchmark\n");
#ifdef BENCHMARK_KEYPAIR
  printf("--keypair          benchmark keypair generation\n");
#endif
#ifdef BENCHMARK_ENCRYPT
  printf("--encrypt          benchmark KEM encapsulation\n");
#endif
#ifdef BENCHMARK_DECRYPT
  printf("--decrypt          benchmark KEM decapsulation\n");
#endif
#ifdef BENCHMARK_EXCHANGE
  printf("--exchange         benchmark KEX exchanges\n");
#endif
  exit(0);
}

int main(int argc, char **argv) {
  int perform_sequential = 0;
  int sequential_iterations = 1000;

  int perform_parallel = 0;
  int duration = 600;
  int thread_count = get_available_cores();

  int benchmark_keypair = 0;
  int benchmark_encrypt = 0;
  int benchmark_decrypt = 0;
  int benchmark_exchange = 0;

  if (argc <= 1)
    die_with_usage(argv[0]);

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--sequential") == 0)
      perform_sequential = 1;
    else if (strcmp(argv[i], "--iterations") == 0)
      sequential_iterations = atoi(argv[++i]);
    else if (strcmp(argv[i], "--parallel") == 0)
      perform_parallel = 1;
    else if (strcmp(argv[i], "--duration") == 0)
      duration = atoi(argv[++i]);
    else if (strcmp(argv[i], "--thread-count") == 0)
      thread_count = atoi(argv[++i]);
    else if (strcmp(argv[i], "--keypair") == 0)
      benchmark_keypair = 1;
    else if (strcmp(argv[i], "--encrypt") == 0)
      benchmark_encrypt = 1;
    else if (strcmp(argv[i], "--decrypt") == 0)
      benchmark_decrypt = 1;
    else if (strcmp(argv[i], "--exchange") == 0)
      benchmark_exchange = 1;
    else if (strcmp(argv[i], "--help") == 0)
      die_with_usage(argv[0]);
    else
      die_with_usage(argv[0]);
  }

  if (perform_sequential)
    benchmark_sequential(sequential_iterations, benchmark_keypair, benchmark_encrypt, benchmark_decrypt, benchmark_exchange);

  if (perform_parallel)
    benchmark_parallel(thread_count, duration, benchmark_keypair, benchmark_encrypt, benchmark_decrypt, benchmark_exchange);
}
