#include "button-set.h"
#include "ui.h"

static int next_index(SUIButtonSet *buttonSet, int index, int delta) {
  if (buttonSet->wrap) {
    return (index + buttonSet->count + delta) % buttonSet->count;
  }
  else {
    int next = index + delta;

    if (next < 0) {
      return 0;
    }
    else if (next > buttonSet->count - 1) {
      return buttonSet->count - 1;
    }

    return next;
  }
}

int sui_button_set_init(SUIButtonSet *buttonSet, enum SUIButtonSetDirection direction) {
  buttonSet->direction = direction;
  buttonSet->wrap = true;
  buttonSet->flowSize = 1;
  buttonSet->count = 0;
  buttonSet->capacity = SUI_BUTTON_SET_DEFAULT_CAPACITY;
  buttonSet->buttons = malloc(buttonSet->capacity * sizeof(SUIButton *));

  if (!buttonSet->buttons) {
    fprintf(stderr, "[GUI] Could not allocate button set memory\n");
    return -1;
  }

  return 0;
}

int sui_button_set_add(SUIButtonSet *buttonSet, SUIButton *button) {
  if (buttonSet->count == buttonSet->capacity) {
    buttonSet->capacity += SUI_BUTTON_SET_DEFAULT_CAPACITY;
    buttonSet->buttons = realloc(buttonSet->buttons, buttonSet->capacity * sizeof(SUIButton *));

    if (!buttonSet->buttons) {
      fprintf(stderr, "[GUI] Could not increase button set capacity memory\n");
      return -1;
    }
  }

  buttonSet->buttons[buttonSet->count] = button;
  buttonSet->count++;

  return 0;
}

SUIButton *sui_button_set_update(SUIButtonSet *buttonSet, SUIInput *input, SUIButton **focused) {
  int focusIndex = -1, nextFocusIndex = 0;

  for (int i = 0; i < buttonSet->count; i++) {
    if (buttonSet->buttons[i]->focused) {
      focusIndex = i;
      nextFocusIndex = i;
      break;
    }
  }

  if (focusIndex != -1) {
    // Regular button layouts
    if ((buttonSet->direction == Vertical && input->buttons.down & KEY_DOWN) ||
        (buttonSet->direction == FlowVertical && input->buttons.down & KEY_DOWN) ||
        (buttonSet->direction == Horizontal && input->buttons.down & KEY_RIGHT) ||
        (buttonSet->direction == FlowHorizontal && input->buttons.down & KEY_RIGHT))
    {
      nextFocusIndex = next_index(buttonSet, focusIndex, +1);
      buttonSet->buttons[focusIndex]->focused = false;
    }

    if ((buttonSet->direction == Vertical && input->buttons.down & KEY_UP) ||
        (buttonSet->direction == FlowVertical && input->buttons.down & KEY_UP) ||
        (buttonSet->direction == Horizontal && input->buttons.down & KEY_LEFT) ||
        (buttonSet->direction == FlowHorizontal && input->buttons.down & KEY_LEFT))
    {
      nextFocusIndex = next_index(buttonSet, focusIndex, -1);
      buttonSet->buttons[focusIndex]->focused = false;
    }

    // Flow button layouts
    if ((buttonSet->direction == FlowVertical && input->buttons.down & KEY_RIGHT) ||
        (buttonSet->direction == FlowHorizontal && input->buttons.down & KEY_DOWN))
    {
      nextFocusIndex = next_index(buttonSet, focusIndex, +buttonSet->flowSize);
      buttonSet->buttons[focusIndex]->focused = false;
    }

    if ((buttonSet->direction == FlowVertical && input->buttons.down & KEY_LEFT) ||
        (buttonSet->direction == FlowHorizontal && input->buttons.down & KEY_UP))
    {
      nextFocusIndex = next_index(buttonSet, focusIndex, -buttonSet->flowSize);
      buttonSet->buttons[focusIndex]->focused = false;
    }
  }

  buttonSet->buttons[nextFocusIndex]->focused = true;

  if (focused) {
    *focused = buttonSet->buttons[nextFocusIndex];
  }

  if (input->buttons.down & KEY_A && focusIndex >= 0) {
      return buttonSet->buttons[focusIndex];
  }

  return NULL;
}

void sui_button_set_render(SUIButtonSet *buttonSet) {
  for (int i = 0; i < buttonSet->count; i++) {
    sui_button_render(buttonSet->buttons[i]);
  }
}

void sui_button_set_cleanup(SUIButtonSet *buttonSet) {
  if (buttonSet->buttons) {
    free(buttonSet->buttons);
    buttonSet->buttons = NULL;
  }
}
