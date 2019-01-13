#pragma once

#include "sui-element.h"

class SUIContainer : public SUIElement {
public:
    SUIContainer(std::string name);
    ~SUIContainer();

    bool isContainer() override;
    bool isFocusable() override;

    void update(SUIInput *input) override;
    void render() override;

    void addChild(SUIElement *element);
    void removeChild(SUIElement *element);
    std::vector<SUIElement *> &children();
 
    virtual SUIFocusResult updateFocus(SUIInput *input, SUIElement *previous = nullptr);
    SUIElement *acceptFocus() override;
    void scrollToChild(SUIElement *element);

protected:
    std::vector<SUIElement *> children_;
    SUIElement *last_focus_;
};