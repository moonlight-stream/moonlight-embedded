#pragma once

#include "sui-common.h"

class SUI;
class SUIContainer;
class SUIGraphics;
class SUIStage;

#define SUI_SCROLL_SPEED 15
#define SUI_BUTTON_SET_DEFAULT_CAPACITY 10

class SUIElement {
public:
    SUIElement(std::string name);

    virtual ~SUIElement();

    virtual void update(SUIInput *input);
    virtual void render();

    SUI *ui();
    std::string& name();
    SUIGraphics *graphics();
    SUIContainer *parent();
    SUIStage *stage();
    
    virtual bool isContainer();
    virtual bool isFocusable();
    bool isFocused();
    virtual SUIElement *acceptFocus();

    void triggerListener(SUIEvent event);
    void addListener(SUIEvent event, SUIEventListener listener);

    SUIRect &bounds();
    void setBounds(const SUIRect &bounds);
    
    SUIRect globalClip();
    SUIRect globalBounds();

protected:
    friend class SUIStage;
    friend class SUIContainer;
    friend class SUIGraphics;
    
    std::string name_;
    SUI *ui_;
    SUIGraphics *graphics_;
    SUIContainer *parent_;
    SUIStage *stage_;

    SUIRect bounds_;

    std::multimap<SUIEvent, SUIEventListener> listeners_;

    // Focus set capabilities
    bool focusable_;
};