#include "sui-element.h"
#include "sui-stage.h"
#include "sui-graphics.h"
#include "sui.h"

SUIElement::SUIElement(std::string name) 
    : name_(name),
      parent_(nullptr),
      bounds_(),
      visible_(true)
{
    graphics_ = new SUIGraphics(this);
}

SUIElement::~SUIElement() {
    delete graphics_;
}

void SUIElement::update(SUIInput *input) {
    
}

void SUIElement::render() {

}

bool SUIElement::isContainer() {
    return false;
}

bool SUIElement::isFocusable() {
    return true;
}

bool SUIElement::isFocused() {
    return stage_->focusedElement() == this;
}

SUIElement *SUIElement::acceptFocus() {
    return isFocusable() ? this : nullptr;
}

bool SUIElement::isVisible() {
    return visible_;
}

void SUIElement::setVisible(bool value) {
    visible_ = value;
}

void SUIElement::triggerListener(SUIEvent event) {
    auto event_range = listeners_.equal_range(event);
    for (auto it = event_range.first; it != event_range.second; it++) {
        it->second(this, event);
    }
}

void SUIElement::addListener(SUIEvent event, SUIEventListener listener) {
    listeners_.insert(std::make_pair(event, listener));
}

std::string& SUIElement::name() {
    return name_;
}

SUIStage *SUIElement::stage() {
    return stage_;
}

SUIContainer *SUIElement::parent() {
    return parent_;
}

SUIGraphics *SUIElement::graphics() {
    return graphics_;
}

SUI *SUIElement::ui() {
    return ui_;
}

SUIRect &SUIElement::bounds() { 
    return bounds_; 
}

void SUIElement::setBounds(const SUIRect &bounds) { 
    bounds_.x = bounds.x;
    bounds_.y = bounds.y;
    bounds_.w = bounds.w;
    bounds_.h = bounds.h;
}

SUIRect SUIElement::globalClip() {
    if (parent_) {
        return parent_->globalBounds();
    }
    else {
        return ui_->bounds;
    }
}

SUIRect SUIElement::globalBounds() {
    if (parent_) {
        SUIRect b = parent_->globalBounds();
        b.x += bounds_.x;
        b.y += bounds_.y;
        b.w = bounds_.w;
        b.h = bounds_.h;
        return b;
    }
    else {
        return bounds_;
    }
}