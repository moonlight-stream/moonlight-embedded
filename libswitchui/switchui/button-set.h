#pragma once

#include "common.h"
#include "input.h"
#include "button.h"

#define BUTTON_SET_DEFAULT_CAPACITY 10

enum ButtonSetDirection {
  Vertical,
  Horizontal,
  FlowVertical,
  FlowHorizontal
};

typedef struct _ButtonSet {
  enum ButtonSetDirection direction;
  size_t count;
  size_t capacity;
  size_t flowSize;
  bool wrap;

  Button **buttons;
} ButtonSet;

int button_set_init(ButtonSet *buttonSet, enum ButtonSetDirection direction);
int button_set_add(ButtonSet *buttonSet, Button *button);
Button *button_set_update(ButtonSet *buttonSet, Input *input, Button **focused);
void button_set_render(ButtonSet *buttonSet);
void button_set_cleanup(ButtonSet *buttonSet);
