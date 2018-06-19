#include "button.h"
#include "text.h"

#include <SDL2/SDL2_gfxPrimitives.h>
#include <math.h>

#define BUTTON_TEXT_COLOR             RGBA8(0x2d, 0x2d, 0x2d, 0xff)
#define BUTTON_FOCUSED_TEXT_COLOR     RGBA8(0x49, 0x28, 0xf0, 0xff)
#define BUTTON_FOCUSED_BACKGROUND     RGBA8(0xfd, 0xfd, 0xfd, 0xff)
#define BUTTON_FOCUSED_BORDER_KEY1    RGBA8(0x00, 0xc0, 0xc0, 0xff)
#define BUTTON_FOCUSED_BORDER_KEY2    RGBA8(0x00, 0xff, 0xdd, 0xff)
#define BUTTON_FOCUSED_BORDER_WIDTH   5
#define BUTTON_FOCUSED_BORDER_PERIOD  1250

#define BUTTON_DEFAULT_WIDTH          376
#define BUTTON_DEFAULT_HEIGHT         76

void button_init(Button *button) {
  button->width = BUTTON_DEFAULT_WIDTH;
  button->height = BUTTON_DEFAULT_HEIGHT;
  button->focused = false;
  button->text = "";
}

void button_render(Button *button) {
  uint32_t textColor = BUTTON_TEXT_COLOR;

  // Draw the button background and border
  if (button->focused) {
    double tlinear = 1.0 * (milliseconds() % BUTTON_FOCUSED_BORDER_PERIOD) / BUTTON_FOCUSED_BORDER_PERIOD ;
    double theta = 2*M_PI * tlinear - M_PI_2;
    double tcycle = sin(theta) * 0.5 + 0.5;

    uint32_t bgColor = BUTTON_FOCUSED_BACKGROUND;
    uint32_t borderColor = interpolate(BUTTON_FOCUSED_BORDER_KEY1, BUTTON_FOCUSED_BORDER_KEY2, tcycle);
    textColor = BUTTON_FOCUSED_TEXT_COLOR;

    boxColor(gui.renderer, button->x, button->y, button->x + button->width, button->y + button->height, bgColor);

    for (int i = -BUTTON_FOCUSED_BORDER_WIDTH/2; i <= BUTTON_FOCUSED_BORDER_WIDTH/2; i++) {
      rectangleColor(gui.renderer, button->x - i, button->y - i, button->x + button->width + i, button->y + button->height + i, borderColor);
    }
  }

  draw_text(gui.fontNormal, button->text, button->x + button->width/2, button->y + button->height/2, textColor, true);
}
