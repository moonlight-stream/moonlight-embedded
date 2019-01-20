#pragma once

#include "sui-element.h"

class SUIText : public SUIElement {
public:
    SUIText(std::string name);
    ~SUIText();

    void render() override;

    std::string &text();
    bool &centered();
    TTF_Font *&font();
    uint32_t &color();

    inline bool isFocusable() override { 
        return false;
    }

private:
    std::string text_;
    TTF_Font *font_;
    bool centered_;
    uint32_t color_;
};
