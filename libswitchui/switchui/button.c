#include "button.h"
#include "ui.h"

#include <SDL2/SDL2_gfxPrimitives.h>
#include <math.h>

void button_init(Button *button) {
  element_init(button);

  button->e.bounds.w = BUTTON_DEFAULT_WIDTH;
  button->e.bounds.h = BUTTON_DEFAULT_HEIGHT;
  button->e.renderer = (Renderer)&button_render;

  button->text = "";
  button->focused = false;
  button->contentRenderer = (ButtonContentRenderer)&button_renderer_content_default;
  button->user = NULL;
}

void button_render(Button *button) {
  Rect clip = get_clip(button);

  // Draw the button background
  if (button->focused) {
    uint32_t bgColor = BUTTON_FOCUSED_BACKGROUND;
    draw_clipped_box_bounds(&button->e.bounds, &clip, bgColor);
  }

  if (button->contentRenderer) {
    // Call the content renderer
    button->contentRenderer(button);
  }

  // Draw the button border
  if (button->focused) {
    double tlinear = 1.0 * (milliseconds() % BUTTON_FOCUSED_BORDER_PERIOD) / BUTTON_FOCUSED_BORDER_PERIOD;
    double theta = 2*M_PI * tlinear - M_PI_2;
    double tcycle = sin(theta) * 0.5 + 0.5;

    uint32_t borderColor = interpolate(BUTTON_FOCUSED_BORDER_KEY1, BUTTON_FOCUSED_BORDER_KEY2, tcycle);

    for (int i = -BUTTON_FOCUSED_BORDER_WIDTH/2; i <= BUTTON_FOCUSED_BORDER_WIDTH/2; i++) {
      draw_clipped_rectangle(button->e.bounds.x - i,
                             button->e.bounds.y - i,
                             button->e.bounds.w + 2*i,
                             button->e.bounds.h + 2*i,
                             &clip,
                             borderColor);
    }
  }
}

void button_renderer_content_default(Button *button) {
  // Simply draw the button text in the center of the button
  uint32_t textColor = button->focused ? BUTTON_FOCUSED_TEXT_COLOR : BUTTON_TEXT_COLOR;
  draw_text(ui.fontNormal,
            button->text,
            button->e.bounds.x + button->e.bounds.w/2,
            button->e.bounds.y + button->e.bounds.h/2,
            textColor,
            true,
            -1);
}

void button_renderer_content_menu(Button *button) {
  uint32_t textColor;

  if (button->focused) {
    textColor = BUTTON_FOCUSED_TEXT_COLOR;

    for (int i = 0; i < BUTTON_MENU_FOCUSED_LINE_WIDTH; i++) {
      vlineColor(ui.renderer,
                 button->e.bounds.x + BUTTON_MENU_FOCUSED_PADDING + i + 1,
                 button->e.bounds.y + BUTTON_MENU_FOCUSED_PADDING,
                 button->e.bounds.y + button->e.bounds.h - BUTTON_MENU_FOCUSED_PADDING,
                 BUTTON_FOCUSED_TEXT_COLOR);
    }
  }
  else {
    textColor = BUTTON_TEXT_COLOR;
  }

  int textWidth, textHeight = text_ascent(ui.fontNormal);
  measure_text(ui.fontNormal, button->text, &textWidth, NULL);

  draw_text(ui.fontNormal,
            button->text,
            button->e.bounds.x + 2*BUTTON_MENU_FOCUSED_PADDING + BUTTON_MENU_FOCUSED_LINE_WIDTH,
            button->e.bounds.y + button->e.bounds.h/2 - textHeight/2,
            textColor,
            false,
            -1);
}
