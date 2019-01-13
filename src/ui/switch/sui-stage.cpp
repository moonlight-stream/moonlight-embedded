#include "sui-stage.h"
#include "sui.h"

SUIStage::SUIStage(SUI *ui, std::string name) 
    : SUIContainer(name),
      focused_element_(nullptr)
{
    if (ui) {
        ui_ = ui;
        stage_ = this;
        bounds_.x = 0;
        bounds_.y = 0;
        bounds_.w = ui->width;
        bounds_.h = ui->height;
    }
}

SUIStage::~SUIStage() {

}

SUIElement *SUIStage::focusedElement() {
    return focused_element_;
}

void SUIStage::setFocusedElement(SUIElement *element) {
    focused_element_ = element;
}

void SUIStage::update(SUIInput *input) {
    SUIContainer::update(input);
    
    if (input->buttons.down & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)) {
        updateFocus(input);
    }
}

void SUIStage::render() {
    SUIContainer::render();
}

SUIFocusResult SUIStage::updateFocus(SUIInput *input, SUIElement *) {
    // printf("[SUIStage] updateFocus():\n");

    // printf("\tfocused element is '%s'\n", focused_element_ ? focused_element_->name().c_str() : "<null>");
    if (focused_element_ == nullptr) {
        // printf("\tCalling acceptFocus()\n");
        SUIElement *element = acceptFocus();

        if (element) {
            // printf("\tFocusing on: %s\n", element->name_.c_str());
        }
        else {
            // printf("\tNo element to focus on\n");
        }

        setFocusedElement(element);
        // printf("\n");
        return SUIFocusRetain;
    }

    SUIContainer *focused_container = focused_element_->parent();
    SUIElement *last_container = focused_element_;

    while (focused_container && focused_container != this) {
        // printf("\tCurrent focused_container is %s\n", focused_container->name_.c_str());
        // printf("\tCurrent last_container is %s\n", last_container->name_.c_str());

        SUIFocusResult result = focused_container->updateFocus(input, last_container);
        // printf("\tResult is %s\n", result == SUIFocusRetain ? "SUIFocusRetain" : "SUIFocusRelease");

        if (result == SUIFocusRetain) {
            // printf("\tFocused is %s\n", focused_element_ ? focused_element_->name_.c_str() : "<null>");
            break;
        }
        else {
            last_container = focused_container;
            focused_container = focused_container->parent();
        }
    }

    // printf("\n");
    return SUIFocusRetain;
}