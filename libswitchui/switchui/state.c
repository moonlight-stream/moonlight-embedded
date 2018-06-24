#include "state.h"

SUIState *sui_state_push(SUIState *top, void *state) {
  SUIState *pushed = malloc(sizeof(SUIState));
  pushed->state = state;
  pushed->next = top;
  return pushed;
}

SUIState *sui_state_replace(SUIState *top, void *state) {
  SUIState *next = sui_state_pop(top);

  SUIState *pushed = malloc(sizeof(SUIState));
  pushed->state = state;
  pushed->next = next;
  return pushed;
}

SUIState *sui_state_pop(SUIState *top) {
  SUIState *next = top->next;
  free(top);

  return next;
}
