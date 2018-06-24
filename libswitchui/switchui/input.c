#include "input.h"

SUIInput input;

int sui_input_init() {
  return 0;
}

SUIInput *sui_input_poll(HidControllerID id) {
  hidScanInput();

  // Read the buttons
  input.buttons.down = hidKeysDown(id);
  input.buttons.held = hidKeysHeld(id);
  input.buttons.up = hidKeysUp(id);

  // Read both joysticks
  hidJoystickRead(&input.joysticks.left, id, JOYSTICK_LEFT);
  hidJoystickRead(&input.joysticks.right, id, JOYSTICK_RIGHT);

  // Read any touch positions
  if (hidTouchCount() > 0) {
    input.touch.touched = true;
    hidTouchRead(&input.touch.position, 0);
  }
  else {
    input.touch.touched = false;
  }

  return &input;
}

bool sui_input_test(SUIInput *input) {
  return (
    input->buttons.down ||
    input->buttons.held ||
    input->buttons.up ||
    input->joysticks.left.dx ||
    input->joysticks.left.dy ||
    input->joysticks.right.dx ||
    input->joysticks.right.dy ||
    input->touch.touched
  );
}

void sui_input_cleanup() {

}
