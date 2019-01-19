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

    Application *application();
    SUI *ui();
    inline SUIStage *stage() { return stage_; }
    inline SUIContainer *content() { return content_; }
    inline SUIContainer *overlay() { return overlay_; }

protected:
    friend class Application;

    Application *application_;

    SUIStage *stage_;
    SUIContainer *content_;
    SUIContainer *overlay_;
    SUIFpsCounter *counter_;

    std::string header_text_;
    std::vector<SUIToolbarActionItem> toolbar_items_;
};
