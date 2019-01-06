#pragma once

#include "sui.h"

#include <vector>

class SUIScene;
class SUIElement;
class SUIButton;

/**
 * SUI Scene
 */

#define SUI_SCROLL_SPEED 15

class SUIScene {
public: 
    SUIScene(SUI *sui);
    ~SUIScene();

    void update(SUIInput *input);
    void render();

    void addElement(SUIElement *element);

    void scroll(SUIElement *element);

    SUIRect *bounds();
    void setBounds(SUIRect *bounds);

protected: 
    friend class SUIElement;

    SUI *ui_;
    SUIRect bounds_;
    std::vector<SUIElement *> elements_;
};

/**
 * SUI Element
 */

class SUIElement {
public:
    SUIElement(SUIScene *scene);
    virtual ~SUIElement();

    virtual void update(SUIInput *input);
    virtual void render();

    SUIRect *clip();
    SUIRect *bounds();
    void setBounds(SUIRect *bounds);
    bool isFixedPosition();
    void setFixedPosition(bool val);  

protected:
    friend class SUIScene;

    SUIScene *scene_;
    SUI *ui_;
    SUIRect bounds_;
    bool fixed_position_;
};

/**
 * SUI Button
 */

#define SUI_BUTTON_TEXT_COLOR             RGBA8(0x2d, 0x2d, 0x2d, 0xff)
#define SUI_BUTTON_FOCUSED_TEXT_COLOR     RGBA8(0x49, 0x28, 0xf0, 0xff)
#define SUI_BUTTON_FOCUSED_BACKGROUND     RGBA8(0xfd, 0xfd, 0xfd, 0xff)
#define SUI_BUTTON_FOCUSED_BORDER_KEY1    RGBA8(0x00, 0xc0, 0xc0, 0xff)
#define SUI_BUTTON_FOCUSED_BORDER_KEY2    RGBA8(0x00, 0xff, 0xdd, 0xff)
#define SUI_BUTTON_FOCUSED_BORDER_WIDTH   5
#define SUI_BUTTON_FOCUSED_BORDER_PERIOD  1250
#define SUI_BUTTON_MENU_FOCUSED_LINE_WIDTH  4
#define SUI_BUTTON_MENU_FOCUSED_PADDING     9
#define SUI_BUTTON_DEFAULT_WIDTH          376
#define SUI_BUTTON_DEFAULT_HEIGHT         76

class SUIButton : public SUIElement {
public:
    SUIButton(SUIScene *scene);
    SUIButton(SUIScene *scene, char *text);
    virtual ~SUIButton();
    
    void update(SUIInput *input) override;
    void render() override;

    char *text();
    void setText(char *text);

    bool isFocused();
    void setFocused(bool val);

protected:
    char *text_;
    bool focused_;
};
