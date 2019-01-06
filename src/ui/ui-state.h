#pragma once

#include "ui-fps-counter.h"
#include "switch/sui.h"
#include "switch/sui-scene.h"

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

protected:
    friend class Application;

    SUI *ui();
    Application *application_;
    UiFpsCounter *counter_;
};
