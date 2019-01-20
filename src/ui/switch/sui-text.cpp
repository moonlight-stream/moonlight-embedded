#include "sui-text.h"
#include "sui-graphics.h"
#include "sui.h"

SUIText::SUIText(std::string name)
    : SUIElement(name),
      text_(),
      font_(),
      centered_(false),
      color_(0)
{

}

SUIText::~SUIText() {

}

void SUIText::render() {
    graphics()->drawText(
        font_, 
        text_, 
        {0, 0, bounds().w, bounds().h}, 
        color_, 
        centered_
    );
}