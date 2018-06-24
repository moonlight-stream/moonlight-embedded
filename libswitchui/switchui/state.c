#include "state.h"

State *state_push(State *top, int state) {
  State *pushed = malloc(sizeof(State));
  pushed->state = state;
  pushed->next = top;
  return pushed;
}

State *state_replace(State *top, int state) {
  State *next = state_pop(top);

  State *pushed = malloc(sizeof(State));
  pushed->state = state;
  pushed->next = next;
  return pushed;
}

State *state_pop(State *top) {
  State *next = top->next;
  free(top);

  return next;
}
