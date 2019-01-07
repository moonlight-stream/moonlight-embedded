#pragma once

#include "sui-common.h"

class SUI;
class SUIGraphics;

#define SUI_SCROLL_SPEED 15
#define SUI_BUTTON_SET_DEFAULT_CAPACITY 10

class SUIElement {
public:
    SUIElement();
    SUIElement(SUI *ui);
    virtual ~SUIElement();

    virtual void update(SUIInput *input);
    virtual void render();

    SUIRect *bounds();
    void setBounds(SUIRect *bounds);
    
    virtual bool isFocusable();
    bool isFocused();
    void setFocused(bool val);

    bool isFixedPosition();
    void setFixedPosition(bool val);  

    void addChild(SUIElement *element);
    void removeChild(SUIElement *element);
    void scrollToChild(SUIElement *element);
    std::vector<SUIElement *> &children();
    SUIElement *parent();

    SUIGraphics *graphics();
    SUI *ui();
    SUIRect *clip();

protected:
    friend class SUIGraphics;
    
    SUI *ui_;
    SUIGraphics *graphics_;
    SUIElement *parent_;
    std::vector<SUIElement *> children_;

    SUIRect bounds_;
    bool fixed_position_;

    // Focus set capabilities
    bool focusable_;
    bool focused_;

private:
    void updateFocus(SUIInput *input);
};