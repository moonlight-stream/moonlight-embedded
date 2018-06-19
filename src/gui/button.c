#include "button.h"
#include "text.h"

#include <SDL2/SDL2_gfxPrimitives.h>
#include <math.h>

void button_init(Button *button) {
  button->width = BUTTON_DEFAULT_WIDTH;
  button->height = BUTTON_DEFAULT_HEIGHT;
  button->focused = false;
  button->text = "";
  button->renderer = NULL;
}

void button_render(Button *button) {
  // Draw the button background
  if (button->focused) {
    uint32_t bgColor = BUTTON_FOCUSED_BACKGROUND;
    boxColor(gui.renderer, button->x, button->y, button->x + button->width, button->y + button->height, bgColor);
  }

  if (button->renderer) {
    // Call the custom renderer
    button->renderer(button);
  }
  else {
    uint32_t textColor = button->focused ? BUTTON_FOCUSED_TEXT_COLOR : BUTTON_TEXT_COLOR;
    text_draw(gui.fontNormal, button->text, button->x + button->width/2, button->y + button->height/2, textColor, true, -1);
  }

  // Draw the button border
  if (button->focused) {
    double tlinear = 1.0 * (milliseconds() % BUTTON_FOCUSED_BORDER_PERIOD) / BUTTON_FOCUSED_BORDER_PERIOD;
    double theta = 2*M_PI * tlinear - M_PI_2;
    double tcycle = sin(theta) * 0.5 + 0.5;

    uint32_t borderColor = interpolate(BUTTON_FOCUSED_BORDER_KEY1, BUTTON_FOCUSED_BORDER_KEY2, tcycle);

    for (int i = -BUTTON_FOCUSED_BORDER_WIDTH/2; i <= BUTTON_FOCUSED_BORDER_WIDTH/2; i++) {
      rectangleColor(gui.renderer, button->x - i, button->y - i, button->x + button->width + i, button->y + button->height + i, borderColor);
    }
  }
}
