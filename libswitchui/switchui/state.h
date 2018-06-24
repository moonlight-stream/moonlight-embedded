#pragma once

#include "common.h"

typedef struct _SUIState {
  void *state;
  struct _SUIState *next;
} SUIState;

SUIState *sui_state_push(SUIState *top, void *state);
SUIState *sui_state_replace(SUIState *top, void *state);
SUIState *sui_state_pop(SUIState *top);
