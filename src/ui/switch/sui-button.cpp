#include "sui-button.h"
#include "sui-graphics.h"
#include "sui.h"

SUIButton::SUIButton() 
    : SUIButton( "")
{

}

SUIButton::SUIButton(std::string text) 
    : SUIElement()
{
    bounds_.w = SUI_BUTTON_DEFAULT_WIDTH;
    bounds_.h = SUI_BUTTON_DEFAULT_HEIGHT;
    text_ = text;
}

SUIButton::~SUIButton() {

}

void SUIButton::update(SUIInput *input) {

}

void SUIButton::render() {
    // Draw the button background
    if (isFocused()) {
        graphics()->drawBox(0, 0, bounds().w, bounds().h, SUI_BUTTON_FOCUSED_BACKGROUND);
    }

    // if (button->contentRenderer) {
    //     // Call the content renderer
    //     button->contentRenderer(button);
    // }
    // else {
    uint32_t text_color = isFocused() ? SUI_BUTTON_FOCUSED_TEXT_COLOR : SUI_BUTTON_TEXT_COLOR;
    graphics()->drawText(ui()->font_normal,
                         text_,
                         bounds_.w / 2,
                         bounds_.h / 2,
                         text_color,
                         true,
                         -1);
    //}

    // Draw the button border
    if (isFocused()) {
        double tlinear = 1.0 * ((svcGetSystemTick() / 19200) % SUI_BUTTON_FOCUSED_BORDER_PERIOD) / SUI_BUTTON_FOCUSED_BORDER_PERIOD;
        double theta = 2 * M_PI * tlinear - (M_PI / 2);
        double tcycle = sin(theta) * 0.5 + 0.5;

        uint32_t border_color = graphics()->interpolate(SUI_BUTTON_FOCUSED_BORDER_KEY1, SUI_BUTTON_FOCUSED_BORDER_KEY2, tcycle);

        for (int i = 0; i <= SUI_BUTTON_FOCUSED_BORDER_WIDTH; i++)
        {
            graphics()->drawRectangle(i,
                                      i,
                                      bounds_.w - 2*i,
                                      bounds_.h - 2*i,
                                      border_color);
        }
    }
}

bool SUIButton::isFocusable() {
    return true;
}

std::string SUIButton::text() { 
    return text_; 
}

void SUIButton::setText(std::string text) { 
    text_ = text; 
}