#include "sui-container.h"
#include "sui.h"

SUIContainer::SUIContainer(std::string name)
    : SUIElement(name)
{
    
}

SUIContainer::~SUIContainer() {

}

bool SUIContainer::isFocusable() {
    return false;
}

void SUIContainer::update(SUIInput *input) {
    for (auto child : children_) {
        child->update(input);
    }
}

void SUIContainer::render() {
    for (auto child : children_) {
        child->render();
    }
}

SUIFocusResult SUIContainer::updateFocus(SUIInput *input) {
    // SUIElement *focus_previous = nullptr,
    //            *focus_current = nullptr,
    //            *focus_next = nullptr;

    // std::vector<SUIContainer *> containers;
    // std::copy_if(children_.begin(), children_.end(), std::back_inserter(containers), [](SUIElement *e) { 
    //     return e->isFocusable(); 
    // });

    // for (int i = 0; i < focusables.size(); i++) {  
    //     SUIElement *child = focusables[i];

    //     if (child->isFocused()) {
    //         focus_previous = focusables[(i - 1 + focusables.size()) % focusables.size()];
    //         focus_current = child;
    //         focus_next = focusables[(i + 1 + focusables.size()) % focusables.size()];
    //         break;
    //     }
    // }

    // if (focus_current) {
    //     if (focus_current->updateFocus(input) == SUIFocusRetain) {
    //         return SUIFocusRetain;
    //     }

    //     if (focus_previous && focus_current && focus_next) {
    //         if (input->buttons.down & KEY_UP) {
    //             focus_current->setFocused(false);
    //             focus_previous->setFocused(true);
    //         }
            
    //         if (input->buttons.down & KEY_DOWN) {
    //             focus_current->setFocused(false);
    //             focus_next->setFocused(true);
    //         }
    //     }
    // }
    // else if (focusables.size()) {
    //     focusables[0]->setFocused(true);
    // }

    return SUIFocusRelease;
}

void SUIContainer::scrollToChild(SUIElement *element) {
    SUIRect in = graphics()->clipBounds(element->bounds(), this->bounds());
    int dx = 0, dy = 0;

    if (in.x != element->bounds().x || in.y != element->bounds().y) {
        // Element is clipped off the top or the left of the scene's clip
        int targetx = (int)fmaxf(element->bounds().x, this->bounds().x);
        int targety = (int)fmaxf(element->bounds().y, this->bounds().y);

        dx = (int)fminf(SUI_SCROLL_SPEED, targetx - element->bounds().x);
        dy = (int)fminf(SUI_SCROLL_SPEED, targety - element->bounds().y);
    }
    else if (in.w != element->bounds().w || in.h != element->bounds().h)
    {
        // Element is clipped off the bottom or right of the scene's clip
        int targetx = (int)fminf(element->bounds().x, this->bounds().x + (this->bounds().w - element->bounds().w));
        int targety = (int)fminf(element->bounds().y, this->bounds().y + (this->bounds().h - element->bounds().h));

        dx = -1 * (int)fminf(SUI_SCROLL_SPEED, element->bounds().x - targetx);
        dy = -1 * (int)fminf(SUI_SCROLL_SPEED, element->bounds().y - targety);
    }

    if (dx || dy) {
        // Update the location of every element in the scene by the delta
        for (auto e : children_) {
            if (!e->isFixedPosition()) {
                e->bounds().x += dx;
                e->bounds().y += dy;
            }
        }
    }
}

std::vector<SUIElement *>& SUIContainer::children() {
    return children_;
}

void SUIContainer::addChild(SUIElement *element) {
    children_.push_back(element);
    element->ui_ = ui_;
    element->parent_ = this;

    if (!element->bounds_.x && !element->bounds_.y && !element->bounds_.w && !element->bounds_.h) {
        element->bounds_.w = bounds_.w;
        element->bounds_.h = bounds_.h;
    }
}

void SUIContainer::removeChild(SUIElement *element) {
    element->ui_ = nullptr;
    element->parent_ = nullptr;
    children_.erase(std::remove(children_.begin(), children_.end(), element), children_.end());
}