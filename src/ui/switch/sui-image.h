#pragma once

#include "sui-element.h"

class SUIImage : public SUIElement {
public:
    SUIImage(std::string name, SDL_Texture *image);
    ~SUIImage();

    void update(SUIInput *) override;
    void render() override;

    inline bool isFocusable() override { 
        return false;
    }

private:
    SDL_Texture *image_;
};
