#pragma once

#include "common.h"

typedef struct _State {
  int state;
  struct _State *next;
} State;

State *state_push(State *top, int state);
State *state_replace(State *top, int state);
State *state_pop(State *top);
