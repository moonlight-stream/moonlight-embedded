#include "sui-scene.h"

#define _USE_MATH_DEFINES
#include <math.h>

/**
 * SUI Scene
 */

SUIScene::SUIScene(SUI *sui)
    : ui_(sui)
{
    bounds_ = sui->clip;
}

SUIScene::~SUIScene() {

}

void SUIScene::update(SUIInput *input) {
    for (auto element : elements_) {
        element->update(input);
    }
}

void SUIScene::render() {
    for (auto element : elements_) {
        element->render();
    }
}

void SUIScene::addElement(SUIElement *element) {
    elements_.push_back(element);
}

void SUIScene::scroll(SUIElement *element) {
    SUIRect in = ui_->clipBounds(element->bounds(), this->bounds());
    int dx = 0, dy = 0;

    if (in.x != element->bounds()->x || in.y != element->bounds()->y) {
        // Element is clipped off the top or the left of the scene's clip
        int targetx = (int)fmaxf(element->bounds()->x, this->bounds()->x);
        int targety = (int)fmaxf(element->bounds()->y, this->bounds()->y);

        dx = (int)fminf(SUI_SCROLL_SPEED, targetx - element->bounds()->x);
        dy = (int)fminf(SUI_SCROLL_SPEED, targety - element->bounds()->y);
    }
    else if (in.w != element->bounds()->w || in.h != element->bounds()->h)
    {
        // Element is clipped off the bottom or right of the scene's clip
        int targetx = (int)fminf(element->bounds()->x, this->bounds()->x + (this->bounds()->w - element->bounds()->w));
        int targety = (int)fminf(element->bounds()->y, this->bounds()->y + (this->bounds()->h - element->bounds()->h));

        dx = -1 * (int)fminf(SUI_SCROLL_SPEED, element->bounds()->x - targetx);
        dy = -1 * (int)fminf(SUI_SCROLL_SPEED, element->bounds()->y - targety);
    }

    if (dx || dy) {
        // Update the location of every element in the scene by the delta
        for (auto e : elements_) {
            if (!e->isFixedPosition()) {
                e->bounds()->x += dx;
                e->bounds()->y += dy;
            }
        }
    }
}

SUIRect *SUIScene::bounds() { 
    return &bounds_; 
}

void SUIScene::setBounds(SUIRect *bounds) { 
    bounds_.x = bounds->x;
    bounds_.y = bounds->y;
    bounds_.w = bounds->w;
    bounds_.h = bounds->h;
}

/**
 * SUI Element
 */

SUIElement::SUIElement(SUIScene *scene) 
    : scene_(scene),
      ui_(scene->ui_),
      fixed_position_(false)
{

}

SUIElement::~SUIElement() {

}

void SUIElement::update(SUIInput *input) {

}

void SUIElement::render() {

}

SUIRect *SUIElement::clip() {
    return scene_->bounds();
}

SUIRect *SUIElement::bounds() { 
    return &bounds_; 
}

void SUIElement::setBounds(SUIRect *bounds) { 
    bounds_.x = bounds->x;
    bounds_.y = bounds->y;
    bounds_.w = bounds->w;
    bounds_.h = bounds->h;
}

bool SUIElement::isFixedPosition() { 
    return fixed_position_; 
}

void SUIElement::setFixedPosition(bool val) { 
    fixed_position_ = val; 
}    

/**
 * SUI Button
 */

SUIButton::SUIButton(SUIScene *scene) 
    : SUIButton(scene, "")
{

}

SUIButton::SUIButton(SUIScene *scene, char *text) 
    : SUIElement(scene),
      focused_(false)
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
        ui_->drawBoxClipped(bounds(), clip(), SUI_BUTTON_FOCUSED_BACKGROUND);
    }

    // if (button->contentRenderer) {
    //     // Call the content renderer
    //     button->contentRenderer(button);
    // }
    // else {
    uint32_t text_color = isFocused() ? SUI_BUTTON_FOCUSED_TEXT_COLOR : SUI_BUTTON_TEXT_COLOR;
    ui_->drawTextClipped(ui_->font_normal,
                         text_,
                         bounds_.x + bounds_.w/2,
                         bounds_.y + bounds_.h/2,
                         clip(),
                         text_color,
                         true,
                         -1);
    //}

    // Draw the button border
    if (isFocused()) {
        double tlinear = 1.0 * ((svcGetSystemTick() / 19200) % SUI_BUTTON_FOCUSED_BORDER_PERIOD) / SUI_BUTTON_FOCUSED_BORDER_PERIOD;
        double theta = 2 * M_PI * tlinear - M_PI_2;
        double tcycle = sin(theta) * 0.5 + 0.5;

        uint32_t border_color = ui_->interpolate(SUI_BUTTON_FOCUSED_BORDER_KEY1, SUI_BUTTON_FOCUSED_BORDER_KEY2, tcycle);

        for (int i = -SUI_BUTTON_FOCUSED_BORDER_WIDTH / 2; i <= SUI_BUTTON_FOCUSED_BORDER_WIDTH / 2; i++)
        {
            ui_->drawRectangleClipped(bounds_.x - i,
                                      bounds_.y - i,
                                      bounds_.w + 2 * i,
                                      bounds_.h + 2 * i,
                                      clip(),
                                      border_color);
        }
    }
}

char *SUIButton::text() { 
    return text_; 
}

void SUIButton::setText(char *text) { 
    text_ = text; 
}

bool SUIButton::isFocused() { 
    return focused_; 
}

void SUIButton::setFocused(bool val) { 
    focused_ = val; 
}