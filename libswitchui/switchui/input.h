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
} Input;

int switch_input_init();
Input *switch_input_poll(HidControllerID id);
bool switch_input_test(Input *input);
void switch_input_cleanup();
