#include <sys/mman.h>
#include <string.h>

#include "state.h"

state_t *state_create(size_t size) {
  state_t *state = malloc(sizeof(state_t));
  if (state == NULL)
    return NULL;

  state->size = size;
  state->memory = NULL;

  // Make the memory readable and writable
  int protection = PROT_READ | PROT_WRITE;
  // Allow for this process tree to access the memory
  int visibility = MAP_SHARED | MAP_ANONYMOUS;

  state->memory = mmap(NULL, size, protection, visibility, -1, 0);
  if (state->memory == NULL) {
    state_free(state);
    return NULL;
  }

  memset(state->memory, 0, size);

  return state;
}

void state_free(state_t *state) {
  if (state->memory != NULL)
    munmap(state->memory, state->size);
  free(state);
}
