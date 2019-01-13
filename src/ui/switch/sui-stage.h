#pragma once

#include "sui-container.h"

class SUIStage : public SUIContainer {
public:
    SUIStage(SUI *ui, std::string name);
    ~SUIStage();

    void update(SUIInput *input) override;
    void render() override;

    SUIElement *focusedElement();
    void setFocusedElement(SUIElement *element);

    SUIFocusResult updateFocus(SUIInput *input, SUIElement *previous = nullptr) override;

private:
    SUIElement *focused_element_;
};