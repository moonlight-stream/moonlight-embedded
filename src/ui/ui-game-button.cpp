#include "ui-game-button.h"

UiGameButton::UiGameButton(std::string name) 
    : SUIButton(name),
      image_(nullptr)
{
    bounds_.w = GAME_BUTTON_WIDTH;
    bounds_.h = GAME_BUTTON_HEIGHT;
}

UiGameButton::~UiGameButton() {

}

void UiGameButton::renderContent() {
    int text_width;
    int text_height = graphics()->measureTextAscent(ui()->font_small);
    graphics()->measureText(ui()->font_small, text_, &text_width, NULL);

    // Draw a small white border
    for (int i = 0; i < 3; i++) {
        graphics()->drawRectangle(i,
                                  i,
                                  bounds_.w - 2 * i,
                                  bounds_.h - 2 * i,
                                  SUI_BUTTON_FOCUSED_BACKGROUND);
    }

    // Draw the caption box
    graphics()->drawBox(0,
                        bounds_.h - 16 - text_height,
                        bounds_.w,
                        16 + text_height,
                        SUI_BUTTON_FOCUSED_BACKGROUND);

    // Draw the caption
    uint32_t text_color = isFocused() ? SUI_BUTTON_FOCUSED_TEXT_COLOR : SUI_BUTTON_TEXT_COLOR;
    graphics()->drawText(ui()->font_small,
                         text_,
                         8,
                         bounds_.h - 8 - text_height,
                         text_color,
                         false,
                         bounds_.w - 16);
}