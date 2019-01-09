#pragma once

#include "switch/sui.h"

#include <switch.h>
#include <memory>

class Application;

enum UiStateResult {
    UiStateResultNormal,
    UiStateResultExit
};

class UiState {
public: 
    UiState(Application *application);
    virtual ~UiState();

    virtual void enter(UiState *parent);
    virtual void exit();

    virtual UiStateResult update(SUIInput *input);
    virtual void render();

    inline SUIElement *stage() { return stage_; }
    inline SUIElement *content() { return content_; }
    inline SUIElement *overlay() { return overlay_; }

protected:
    friend class Application;

    SUI *ui();
    Application *application_;

    SUIElement *stage_;
    SUIElement *content_;
    SUIElement *overlay_;
    SUIFpsCounter *counter_;

    std::string header_text_;
    std::vector<SUIToolbarActionItem> toolbar_items_;
};
