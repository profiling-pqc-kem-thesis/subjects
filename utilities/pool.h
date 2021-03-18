#ifndef UTILITIES_POOL_H
#define UTILITIES_POOL_H
#include <unistd.h>
#include <pthread.h>

typedef struct {
  pthread_t *threads;
  size_t thread_count;
  int (*benchmark)(int thread_index, void *state);
} pool_t;

// Create a pool of threads
pool_t *pool_create(int (*benchmark)(int thread_index, void *state), int thread_count);
// Start all threads in a pool
int pool_start(pool_t *pool, void *state);
// Close the pool, exiting all threads immediately
void pool_kill(pool_t *pool);
// Wait for a pool to exit. Returns the first exit code on failure, or 0 on success.
int pool_wait_for_exit(pool_t *pool, int *exit_codes);
// Free up all memory from a terminated pool
void pool_free(pool_t *pool);
#endif
