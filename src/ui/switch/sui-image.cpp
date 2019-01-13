#include "sui-image.h"
#include "sui-graphics.h"
#include "sui.h"

SUIImage::SUIImage(std::string name, SDL_Texture *image)
    : SUIElement(name),
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