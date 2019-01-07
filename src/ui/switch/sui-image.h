#pragma once

#include "sui-element.h"

class SUIImage : public SUIElement {
public:
    SUIImage(SDL_Texture *image);
    ~SUIImage();

    void update(SUIInput *) override;
    void render() override;

private:
    SDL_Texture *image_;
};
