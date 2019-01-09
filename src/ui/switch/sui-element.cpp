#include "sui-element.h"
#include "sui-graphics.h"
#include "sui.h"

SUIElement::SUIElement() : SUIElement(nullptr) { }
SUIElement::SUIElement(SUI *ui) 
    : ui_(ui),
      parent_(nullptr),
      children_(),
      bounds_(),
      fixed_position_(false),
      focused_(false)
{
    if (ui) {
        bounds_.x = 0;
        bounds_.y = 0;
        bounds_.w = ui->width;
        bounds_.h = ui->height;
    }

    graphics_ = new SUIGraphics(this);
}

SUIElement::~SUIElement() {
    delete graphics_;
}

void SUIElement::update(SUIInput *input) {
    // Update every child element
    for (auto child : children_) {
        child->update(input);
    }

    // Update the focused element based on the input
    updateFocus(input);
}

void SUIElement::render() {
    // Update every child element
    for (auto child : children_) {
        child->render();
    }
}

void SUIElement::addChild(SUIElement *element) {
    children_.push_back(element);
    element->ui_ = ui_;
    element->parent_ = this;

    if (!element->bounds_.x && !element->bounds_.y && !element->bounds_.w && !element->bounds_.h) {
        element->bounds_.w = bounds_.w;
        element->bounds_.h = bounds_.h;
    }
}

void SUIElement::removeChild(SUIElement *element) {
    element->ui_ = nullptr;
    element->parent_ = nullptr;
    children_.erase(std::remove(children_.begin(), children_.end(), element), children_.end());
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

bool SUIElement::isFocusable() {
    return false;
}

bool SUIElement::isFocused() {
    return focused_;
}

void SUIElement::setFocused(bool val) {
    focused_ = val;    
}

bool SUIElement::isFixedPosition() { 
    return fixed_position_; 
}

void SUIElement::setFixedPosition(bool val) { 
    fixed_position_ = val; 
}

void SUIElement::scrollToChild(SUIElement *element) {
    SUIRect in = graphics()->clipBounds(element->bounds(), this->bounds());
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
        for (auto e : children_) {
            if (!e->isFixedPosition()) {
                e->bounds()->x += dx;
                e->bounds()->y += dy;
            }
        }
    }
}

void SUIElement::updateFocus(SUIInput *input) {
    SUIElement *focus_previous = nullptr,
               *focus_current = nullptr,
               *focus_next = nullptr;

    std::vector<SUIElement *> focusables;
    std::copy_if(children_.begin(), children_.end(), std::back_inserter(focusables), [](SUIElement *e) { return e->isFocusable(); });


    for (int i = 0; i < focusables.size(); i++) {  
        SUIElement *child = focusables[i];

        if (child->isFocused()) {
            focus_previous = focusables[(i + 1 + focusables.size()) % focusables.size()];
            focus_current = child;
            focus_next = focusables[(i - 1 + focusables.size()) % focusables.size()];
            break;
        }
    }

    if (focus_current) {
        if (focus_previous && focus_current && focus_next) {
            if (input->buttons.down & KEY_UP || input->buttons.down & KEY_DUP) {
                focus_current->setFocused(false);
                focus_previous->setFocused(true);
            }
            
            if (input->buttons.down & KEY_DOWN || input->buttons.down & KEY_DDOWN) {
                focus_current->setFocused(false);
                focus_next->setFocused(true);
            }
        }
    }
    else if (focusables.size()) {
        focusables[0]->setFocused(true);
    }

    // if (focusIndex != -1) {
    //     // Regular button layouts
    //     if ((buttonSet->direction == Vertical && input->buttons.down & KEY_DOWN) ||
    //         (buttonSet->direction == FlowVertical && input->buttons.down & KEY_DOWN) ||
    //         (buttonSet->direction == Horizontal && input->buttons.down & KEY_RIGHT) ||
    //         (buttonSet->direction == FlowHorizontal && input->buttons.down & KEY_RIGHT))
    //     {
    //         nextFocusIndex = next_index(buttonSet, focusIndex, +1);
    //         buttonSet->buttons[focusIndex]->focused = false;
    //     }

    //     if ((buttonSet->direction == Vertical && input->buttons.down & KEY_UP) ||
    //         (buttonSet->direction == FlowVertical && input->buttons.down & KEY_UP) ||
    //         (buttonSet->direction == Horizontal && input->buttons.down & KEY_LEFT) ||
    //         (buttonSet->direction == FlowHorizontal && input->buttons.down & KEY_LEFT))
    //     {
    //         nextFocusIndex = next_index(buttonSet, focusIndex, -1);
    //         buttonSet->buttons[focusIndex]->focused = false;
    //     }

    //     // Flow button layouts
    //     if ((buttonSet->direction == FlowVertical && input->buttons.down & KEY_RIGHT) ||
    //         (buttonSet->direction == FlowHorizontal && input->buttons.down & KEY_DOWN))
    //     {
    //         nextFocusIndex = next_index(buttonSet, focusIndex, +buttonSet->flowSize);
    //         buttonSet->buttons[focusIndex]->focused = false;
    //     }

    //     if ((buttonSet->direction == FlowVertical && input->buttons.down & KEY_LEFT) ||
    //         (buttonSet->direction == FlowHorizontal && input->buttons.down & KEY_UP))
    //     {
    //         nextFocusIndex = next_index(buttonSet, focusIndex, -buttonSet->flowSize);
    //         buttonSet->buttons[focusIndex]->focused = false;
    //     }
    // }

    // buttonSet->buttons[nextFocusIndex]->focused = true;

    // if (focused)
    // {
    //     *focused = buttonSet->buttons[nextFocusIndex];
    // }

    // if (input->buttons.down & KEY_A && focusIndex >= 0)
    // {
    //     return buttonSet->buttons[focusIndex];
    // }

    // // Determine if any of the buttons is touched
    // if (input->touch.touched)
    // {
    //     for (int i = 0; i < buttonSet->count; i++)
    //     {
    //         SUIButton *button = buttonSet->buttons[i];

    //         if (sui_rect_contains_point(&button->e.bounds, input->touch.position.px, input->touch.position.py))
    //         {
    //             buttonSet->buttons[nextFocusIndex]->focused = false;
    //             button->focused = true;

    //             if (focused)
    //             {
    //                 *focused = button;
    //             }

    //             return button;
    //         }
    //     }
    // }
}

std::vector<SUIElement *>& SUIElement::children() {
    return children_;
}

SUIElement *SUIElement::parent() {
    return parent_;
}

SUIGraphics *SUIElement::graphics() {
    return graphics_;
}

SUI *SUIElement::ui() {
    return ui_;
}

SUIRect *SUIElement::clip() {
    if (parent_) {
        return parent_->bounds();
    }
    else {
        return &bounds_;
    }
}