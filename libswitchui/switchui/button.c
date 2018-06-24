#include "button.h"
#include "ui.h"

#include <SDL2/SDL2_gfxPrimitives.h>
#include <math.h>

void sui_button_init(SUIButton *button) {
  sui_element_init(button);

  button->e.bounds.w = SUI_BUTTON_DEFAULT_WIDTH;
  button->e.bounds.h = SUI_BUTTON_DEFAULT_HEIGHT;
  button->e.renderer = (SUIRenderer)&sui_button_render;

  button->text = "";
  button->focused = false;
  button->contentRenderer = (SUIButtonContentRenderer)&sui_button_renderer_content_default;
  button->user = NULL;
}

void sui_button_render(SUIButton *button) {
  SUIRect clip = sui_element_get_clip(button);

  // Draw the button background
  if (button->focused) {
    uint32_t bgColor = SUI_BUTTON_FOCUSED_BACKGROUND;
    sui_draw_clipped_box_bounds(&button->e.bounds, &clip, bgColor);
  }

  if (button->contentRenderer) {
    // Call the content renderer
    button->contentRenderer(button);
  }

  // Draw the button border
  if (button->focused) {
    double tlinear = 1.0 * (milliseconds() % SUI_BUTTON_FOCUSED_BORDER_PERIOD) / SUI_BUTTON_FOCUSED_BORDER_PERIOD;
    double theta = 2*M_PI * tlinear - M_PI_2;
    double tcycle = sin(theta) * 0.5 + 0.5;

    uint32_t borderColor = interpolate(SUI_BUTTON_FOCUSED_BORDER_KEY1, SUI_BUTTON_FOCUSED_BORDER_KEY2, tcycle);

    for (int i = -SUI_BUTTON_FOCUSED_BORDER_WIDTH/2; i <= SUI_BUTTON_FOCUSED_BORDER_WIDTH/2; i++) {
      sui_draw_clipped_rectangle(button->e.bounds.x - i,
                                 button->e.bounds.y - i,
                                 button->e.bounds.w + 2*i,
                                 button->e.bounds.h + 2*i,
                                 &clip,
                                 borderColor);
    }
  }
}

void sui_button_renderer_content_default(SUIButton *button) {
  // Simply draw the button text in the center of the button
  uint32_t textColor = button->focused ? SUI_BUTTON_FOCUSED_TEXT_COLOR : SUI_BUTTON_TEXT_COLOR;
  sui_draw_text(ui.fontNormal,
                button->text,
                button->e.bounds.x + button->e.bounds.w/2,
                button->e.bounds.y + button->e.bounds.h/2,
                textColor,
                true,
                -1);
}

void sui_button_renderer_content_menu(SUIButton *button) {
  uint32_t textColor;

  if (button->focused) {
    textColor = SUI_BUTTON_FOCUSED_TEXT_COLOR;

    for (int i = 0; i < SUI_BUTTON_MENU_FOCUSED_LINE_WIDTH; i++) {
      vlineColor(ui.renderer,
                 button->e.bounds.x + SUI_BUTTON_MENU_FOCUSED_PADDING + i + 1,
                 button->e.bounds.y + SUI_BUTTON_MENU_FOCUSED_PADDING,
                 button->e.bounds.y + button->e.bounds.h - SUI_BUTTON_MENU_FOCUSED_PADDING,
                 SUI_BUTTON_FOCUSED_TEXT_COLOR);
    }
  }
  else {
    textColor = SUI_BUTTON_TEXT_COLOR;
  }

  int textWidth, textHeight = sui_text_ascent(ui.fontNormal);
  sui_measure_text(ui.fontNormal, button->text, &textWidth, NULL);

  sui_draw_text(ui.fontNormal,
                button->text,
                button->e.bounds.x + 2*SUI_BUTTON_MENU_FOCUSED_PADDING + SUI_BUTTON_MENU_FOCUSED_LINE_WIDTH,
                button->e.bounds.y + button->e.bounds.h/2 - textHeight/2,
                textColor,
                false,
                -1);
}
