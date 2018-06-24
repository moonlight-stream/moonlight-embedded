#pragma once

#include "common.h"

typedef struct {
  struct {
    uint64_t down;
    uint64_t held;
    uint64_t up;
  } buttons;

  struct {
    JoystickPosition left;
    JoystickPosition right;
  } joysticks;

  struct {
    bool touched;
    touchPosition position;
  } touch;
} SUIInput;

int sui_input_init();
SUIInput *sui_input_poll(HidControllerID id);
bool sui_input_test(SUIInput *input);
void sui_input_cleanup();
