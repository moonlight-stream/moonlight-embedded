#pragma once

#include "sui-element.h"

enum SUIFocusResult {
    SUIFocusRetain,
    SUIFocusRelease
};

class SUIContainer : public SUIElement {
public:
    SUIContainer(std::string name);
    ~SUIContainer();

    bool isFocusable() override;

    void update(SUIInput *input) override;
    void render() override;

    void addChild(SUIElement *element);
    void removeChild(SUIElement *element);
    std::vector<SUIElement *> &children();

protected: 
    virtual SUIFocusResult updateFocus(SUIInput *input);
    void scrollToChild(SUIElement *element);

    std::vector<SUIElement *> children_;
};