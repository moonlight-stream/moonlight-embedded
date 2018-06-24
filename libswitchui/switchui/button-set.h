#pragma once

#include "common.h"
#include "input.h"
#include "button.h"

#define SUI_BUTTON_SET_DEFAULT_CAPACITY 10

enum SUIButtonSetDirection {
  Vertical,
  Horizontal,
  FlowVertical,
  FlowHorizontal
};

typedef struct _SUIButtonSet {
  enum SUIButtonSetDirection direction;
  size_t count;
  size_t capacity;
  size_t flowSize;
  bool wrap;

  SUIButton **buttons;
} SUIButtonSet;

int sui_button_set_init(SUIButtonSet *buttonSet, enum SUIButtonSetDirection direction);
int sui_button_set_add(SUIButtonSet *buttonSet, SUIButton *button);
SUIButton *sui_button_set_update(SUIButtonSet *buttonSet, SUIInput *input, SUIButton **focused);
void sui_button_set_render(SUIButtonSet *buttonSet);
void sui_button_set_cleanup(SUIButtonSet *buttonSet);
