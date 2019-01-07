#include "sui-button.h"
#include "sui-graphics.h"
#include "sui.h"

SUIButton::SUIButton() 
    : SUIButton( "")
{

}

SUIButton::SUIButton(char *text) 
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
        graphics()->drawBoxClipped(bounds(), clip(), SUI_BUTTON_FOCUSED_BACKGROUND);
    }

    // if (button->contentRenderer) {
    //     // Call the content renderer
    //     button->contentRenderer(button);
    // }
    // else {
    uint32_t text_color = isFocused() ? SUI_BUTTON_FOCUSED_TEXT_COLOR : SUI_BUTTON_TEXT_COLOR;
    graphics()->drawTextClipped(ui()->font_normal,
                                text_,
                                bounds_.x + bounds_.w / 2,
                                bounds_.y + bounds_.h / 2,
                                clip(),
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

        for (int i = -SUI_BUTTON_FOCUSED_BORDER_WIDTH / 2; i <= SUI_BUTTON_FOCUSED_BORDER_WIDTH / 2; i++)
        {
            graphics()->drawRectangleClipped(bounds_.x - i,
                                             bounds_.y - i,
                                             bounds_.w + 2 * i,
                                             bounds_.h + 2 * i,
                                             clip(),
                                             border_color);
        }
    }
}

bool SUIButton::isFocusable() {
    return true;
}

char *SUIButton::text() { 
    return text_; 
}

void SUIButton::setText(char *text) { 
    text_ = text; 
}