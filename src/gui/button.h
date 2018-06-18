#pragma once

#include "gui.h"

typedef struct _Button {
  char *text;
  int x;
  int y;
  int width;
  int height;
  bool focused;
} Button;

void button_init(Button *button);
void button_render(Button *button);
