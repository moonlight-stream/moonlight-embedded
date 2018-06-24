#pragma once

#include "common.h"
#include "input.h"
#include "element.h"

#define SUI_BUTTON_TEXT_COLOR             RGBA8(0x2d, 0x2d, 0x2d, 0xff)
#define SUI_BUTTON_FOCUSED_TEXT_COLOR     RGBA8(0x49, 0x28, 0xf0, 0xff)
#define SUI_BUTTON_FOCUSED_BACKGROUND     RGBA8(0xfd, 0xfd, 0xfd, 0xff)
#define SUI_BUTTON_FOCUSED_BORDER_KEY1    RGBA8(0x00, 0xc0, 0xc0, 0xff)
#define SUI_BUTTON_FOCUSED_BORDER_KEY2    RGBA8(0x00, 0xff, 0xdd, 0xff)
#define SUI_BUTTON_FOCUSED_BORDER_WIDTH   5
#define SUI_BUTTON_FOCUSED_BORDER_PERIOD  1250

#define SUI_BUTTON_MENU_FOCUSED_LINE_WIDTH  4
#define SUI_BUTTON_MENU_FOCUSED_PADDING     9

#define SUI_BUTTON_DEFAULT_WIDTH          376
#define SUI_BUTTON_DEFAULT_HEIGHT         76

struct _SUIButton;
typedef void (*SUIButtonContentRenderer)(struct _SUIButton *button);

typedef struct _SUIButton {
  SUIElement e;
  char *text;
  bool focused;

  SUIButtonContentRenderer contentRenderer;
  void *user;
} SUIButton;

void sui_button_init(SUIButton *button);
void sui_button_render(SUIButton *button);

void sui_button_renderer_content_default(SUIButton *button);
void sui_button_renderer_content_menu(SUIButton *button);
