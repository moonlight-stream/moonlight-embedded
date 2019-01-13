#include "sui-container.h"
#include "sui.h"

SUIContainer::SUIContainer(std::string name)
    : SUIElement(name),
      children_(),
      last_focus_(nullptr)
{
    
}

SUIContainer::~SUIContainer() {

}

bool SUIContainer::isContainer() {
    return true;
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

SUIFocusResult SUIContainer::updateFocus(SUIInput *input, SUIElement *previous) {
    last_focus_ = previous;

    auto previous_find = std::find(children_.begin(), children_.end(), previous);
    int previous_index = std::distance(children_.begin(), previous_find);

    // printf("\tIndex of previous in container: %d\n", previous_index);

    SUIElement *next_candidate;
    int next_index = previous_index;

    while (next_index >= 0 && next_index < children_.size()) {
        if (input->buttons.down & KEY_UP) {
            next_index = next_index - 1;
        }
        else if (input->buttons.down & KEY_DOWN) {
            next_index = next_index + 1;
        }

        // printf("\tNext index to check: %d\n", next_index);
        if (next_index >= 0 && next_index < children_.size()) {
            next_candidate = children_[next_index];

            if (next_candidate) {
                // printf("\tChecking acceptFocus() of %s\n", next_candidate->name_.c_str());
                next_candidate = next_candidate->acceptFocus();
            }

            if (next_candidate) {
                // printf("\tGot non-null acceptFocus() with name %s\n", next_candidate->name_.c_str());
                break;
            }
        }
    }

    if (next_candidate) {
        stage()->setFocusedElement(next_candidate);
        return SUIFocusRetain;
    }
    else {
        return SUIFocusRelease;
    }
}

SUIElement *SUIContainer::acceptFocus() {
    SUIElement *next = nullptr; 

    if (last_focus_) {
        if (last_focus_->parent() == this) {
            next = last_focus_->acceptFocus();
        }
        else {
            last_focus_ = nullptr;
        }
    }

    if (!next) {
        for (auto child : children_) {
            next = child->acceptFocus();
            if (next) {
                break;
            }
        }
    }

    return next;
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
            // if (!e->isFixedPosition()) {
                e->bounds().x += dx;
                e->bounds().y += dy;
            // }
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
    element->stage_ = stage_;

    if (!element->bounds_.x && !element->bounds_.y && !element->bounds_.w && !element->bounds_.h) {
        element->bounds_.w = bounds_.w;
        element->bounds_.h = bounds_.h;
    }
}

void SUIContainer::removeChild(SUIElement *element) {
    element->ui_ = nullptr;
    element->parent_ = nullptr;
    element->stage_ = nullptr;
    children_.erase(std::remove(children_.begin(), children_.end(), element), children_.end());
}