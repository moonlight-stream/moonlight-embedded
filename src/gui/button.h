#pragma once

#include "gui.h"

#define BUTTON_TEXT_COLOR             RGBA8(0x2d, 0x2d, 0x2d, 0xff)
#define BUTTON_FOCUSED_TEXT_COLOR     RGBA8(0x49, 0x28, 0xf0, 0xff)
#define BUTTON_FOCUSED_BACKGROUND     RGBA8(0xfd, 0xfd, 0xfd, 0xff)
#define BUTTON_FOCUSED_BORDER_KEY1    RGBA8(0x00, 0xc0, 0xc0, 0xff)
#define BUTTON_FOCUSED_BORDER_KEY2    RGBA8(0x00, 0xff, 0xdd, 0xff)
#define BUTTON_FOCUSED_BORDER_WIDTH   5
#define BUTTON_FOCUSED_BORDER_PERIOD  1250

#define BUTTON_DEFAULT_WIDTH          376
#define BUTTON_DEFAULT_HEIGHT         76

struct _Button;
typedef void (*ButtonRenderer)(struct _Button *button);

typedef struct _Button {
  char *text;
  int x;
  int y;
  int width;
  int height;
  bool focused;
  void *user;

  ButtonRenderer renderer;
} Button;

void button_init(Button *button);
void button_render(Button *button);
