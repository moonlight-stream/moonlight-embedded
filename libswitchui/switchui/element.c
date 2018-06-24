#include "element.h"
#include "ui.h"

void sui_element_init(SUIElement *element) {
  element->updater = NULL;
  element->renderer = NULL;
  element->fixed = false;

  element->_scene = NULL;
  element->_previous = NULL;
  element->_next = NULL;
}

void sui_element_layout_vertical(int sx, int sy, int spacing, int n, ...) {
  va_list arg;
  va_start(arg, n);

  for (int i = 0; i < n; i++) {
    SUIElement *element = va_arg(arg, SUIElement *);

    // Position the current element
    element->bounds.x = sx;
    element->bounds.y = sy;

    // Move the position for the next element
    sy = sy + element->bounds.h + spacing;
    element = element->_next;
  }

  va_end(arg);
}

void sui_element_layout_horizontal(int sx, int sy, int spacing, int n, ...) {
  va_list arg;
  va_start(arg, n);

  for (int i = 0; i < n; i++) {
    SUIElement *element = va_arg(arg, SUIElement *);

    // Position the current element
    element->bounds.x = sx;
    element->bounds.y = sy;

    // Move the position for the next element
    sx = sx + element->bounds.w + spacing;
    element = element->_next;
  }

  va_end(arg);
}

SUIRect sui_element_get_clip(SUIElement *element) {
  if (element->_scene) {
    return element->_scene->clip;
  }

  SUIRect clip = {
    .x = 0,
    .y = 0,
    .w = ui.width,
    .h = ui.height
  };
  return clip;
}
