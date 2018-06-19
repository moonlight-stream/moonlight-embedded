#include "button-set.h"

int button_set_init(ButtonSet *buttonSet, enum ButtonSetDirection direction) {
  buttonSet->direction = direction;
  buttonSet->count = 0;
  buttonSet->capacity = BUTTON_SET_DEFAULT_CAPACITY;
  buttonSet->buttons = malloc(buttonSet->capacity * sizeof(Button *));

  if (!buttonSet->buttons) {
    fprintf(stderr, "[GUI] Could not allocate button set memory\n");
    return -1;
  }

  return 0;
}

int button_set_add(ButtonSet *buttonSet, Button *button) {
  if (buttonSet->count == buttonSet->capacity) {
    buttonSet->capacity += BUTTON_SET_DEFAULT_CAPACITY;
    buttonSet->buttons = realloc(buttonSet->buttons, buttonSet->capacity * sizeof(Button *));

    if (!buttonSet->buttons) {
      fprintf(stderr, "[GUI] Could not increase button set capacity memory\n");
      return -1;
    }
  }

  buttonSet->buttons[buttonSet->count] = button;
  buttonSet->count++;

  return 0;
}

Button *button_set_update(ButtonSet *buttonSet, Input *input) {
  int focusIndex = -1, nextFocusIndex = 0;

  for (int i = 0; i < buttonSet->count; i++) {
    if (buttonSet->buttons[i]->focused) {
      focusIndex = i;
      break;
    }
  }

  if ((buttonSet->direction == Vertical && input->keys & KEY_DOWN) ||
      (buttonSet->direction == Horizontal && input->keys & KEY_RIGHT))
  {
    if (focusIndex != -1) {
      nextFocusIndex = (focusIndex + buttonSet->count + 1) % buttonSet->count;
      buttonSet->buttons[focusIndex]->focused = false;
    }

    buttonSet->buttons[nextFocusIndex]->focused = true;
  }

  if ((buttonSet->direction == Vertical && input->keys & KEY_UP) ||
      (buttonSet->direction == Horizontal && input->keys & KEY_LEFT))
  {
    if (focusIndex != -1) {
      nextFocusIndex = (focusIndex + buttonSet->count - 1) % buttonSet->count;
      buttonSet->buttons[focusIndex]->focused = false;
    }

    buttonSet->buttons[nextFocusIndex]->focused = true;
  }

  if (input->keys & KEY_A && focusIndex >= 0) {
    return buttonSet->buttons[focusIndex];
  }

  return NULL;
}

void button_set_render(ButtonSet *buttonSet) {
  for (int i = 0; i < buttonSet->count; i++) {
    button_render(buttonSet->buttons[i]);
  }
}

void button_set_cleanup(ButtonSet *buttonSet) {
  if (buttonSet->buttons) {
    free(buttonSet->buttons);
    buttonSet->buttons = NULL;
  }
}
