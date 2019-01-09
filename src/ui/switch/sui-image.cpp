#include "sui-image.h"
#include "sui-graphics.h"
#include "sui.h"

SUIImage::SUIImage(SDL_Texture *image)
    : SUIElement(),
      image_(image)
{

}

SUIImage::~SUIImage() {

}

void SUIImage::update(SUIInput *) {

}

void SUIImage::render() {
    graphics()->drawTexture(image_, 0, 0, bounds().w, bounds().h);
}