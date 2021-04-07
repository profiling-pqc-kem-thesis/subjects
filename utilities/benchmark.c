#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cpu.h"

#include "benchmark.h"

void die_with_usage(char *name) {
  printf("\033[0;1musage:\033[0m %s <benchmark> [flags]\n\n", name);
  printf("\033[0;1mbenchmarks:\033[0m\n");
  printf("sequential\n");
  printf("parallel\n");
  printf("\n");
  printf("\033[0;1mflags:\033[0m\n");
  printf("--help             print this help text\n");
  printf("--iterations <n>   iterations of the benchmark (per thread if parallel)\n");
  printf("--timeout <n>      maximum time in seconds to perform the benchmark\n");
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
  int perform_parallel = 0;

  int iterations = 1000;
  // One hour
  int timeout = 1 * 60 * 60;
  int thread_count = get_available_cores();

  int benchmark_keypair = 0;
  int benchmark_encrypt = 0;
  int benchmark_decrypt = 0;
  int benchmark_exchange = 0;

  if (argc < 2)
    die_with_usage(argv[0]);

  if (strcmp(argv[1], "sequential") == 0)
    perform_sequential = 1;
  else if (strcmp(argv[1], "parallel") == 0)
    perform_parallel = 1;
  else
    die_with_usage(argv[0]);

  for (int i = 2; i < argc; i++) {
    if (strcmp(argv[i], "--iterations") == 0)
      iterations = atoi(argv[++i]);
    else if (strcmp(argv[i], "--timeout") == 0)
      timeout = atoi(argv[++i]);
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
    else
      die_with_usage(argv[0]);
    }

  if (perform_sequential)
    benchmark_sequential(iterations, timeout, benchmark_keypair, benchmark_encrypt, benchmark_decrypt, benchmark_exchange);

  if (perform_parallel)
    benchmark_parallel(iterations, thread_count, timeout, benchmark_keypair, benchmark_encrypt, benchmark_decrypt, benchmark_exchange);
}
