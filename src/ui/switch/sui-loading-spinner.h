#pragma once

#include "sui-element.h"
#include "sui-fps-counter.h"

class SUILoadingSpinner : public SUIElement {
public:
    SUILoadingSpinner(std::string name);
    ~SUILoadingSpinner();

    void update(SUIInput *) override;
    void render() override;

    uint32_t &color();

    inline bool isFocusable() override { 
        return false;
    }

private:
    SUIFpsCounter *counter_;
    uint32_t color_;
};
