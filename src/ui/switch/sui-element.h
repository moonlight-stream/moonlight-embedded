#pragma once

#include "sui-common.h"

class SUI;
class SUIGraphics;

#define SUI_SCROLL_SPEED 15
#define SUI_BUTTON_SET_DEFAULT_CAPACITY 10

class SUIElement {
public:
    SUIElement(std::string name);

    virtual ~SUIElement();

    virtual void update(SUIInput *input);
    virtual void render();

    SUI *ui();
    SUIGraphics *graphics();
    SUIElement *parent();
    
    virtual bool isFocusable();
    bool isFocused();
    void setFocused(bool val);


    SUIRect &bounds();
    void setBounds(const SUIRect &bounds);
    
    SUIRect globalClip();
    SUIRect globalBounds();

    bool isFixedPosition();
    void setFixedPosition(bool val);  

protected:
    friend class SUIContainer;
    friend class SUIGraphics;
    
    std::string name_;
    SUI *ui_;
    SUIGraphics *graphics_;
    SUIElement *parent_;

    SUIRect bounds_;
    bool fixed_position_;

    // Focus set capabilities
    bool focusable_;
    bool focused_;
};