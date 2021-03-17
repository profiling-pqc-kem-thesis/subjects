#ifndef STATE_H
#define STATE_H

#include <stdlib.h>

typedef struct {
  void *memory;
  size_t size;
} state_t;

state_t *state_create(size_t size);
void state_free(state_t *state);

#endif
