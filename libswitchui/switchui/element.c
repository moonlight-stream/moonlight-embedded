#include "element.h"

void element_init(Element *element) {
  element->updater = NULL;
  element->renderer = NULL;
  element->fixed = false;

  element->_scene = NULL;
  element->_previous = NULL;
  element->_next = NULL;
}

void element_layout_vertical(int sx, int sy, int spacing, int n, ...) {
  va_list arg;
  va_start(arg, n);

  for (int i = 0; i < n; i++) {
    Element *element = va_arg(arg, Element *);

    // Position the current element
    element->bounds.x = sx;
    element->bounds.y = sy;

    // Move the position for the next element
    sy = sy + element->bounds.h + spacing;
    element = element->_next;
  }

  va_end(arg);
}

void element_layout_horizontal(int sx, int sy, int spacing, int n, ...) {
  va_list arg;
  va_start(arg, n);

  for (int i = 0; i < n; i++) {
    Element *element = va_arg(arg, Element *);

    // Position the current element
    element->bounds.x = sx;
    element->bounds.y = sy;

    // Move the position for the next element
    sx = sx + element->bounds.w + spacing;
    element = element->_next;
  }

  va_end(arg);
}
