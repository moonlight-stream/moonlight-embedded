#include "sui-stage.h"
#include "sui.h"

SUIStage::SUIStage(SUI *ui, std::string name) 
    : SUIContainer(name)
{
    if (ui) {
        ui_ = ui;
        bounds_.x = 0;
        bounds_.y = 0;
        bounds_.w = ui->width;
        bounds_.h = ui->height;
    }
}

SUIStage::~SUIStage() {

}

void SUIStage::update(SUIInput *input) {
    SUIContainer::update(input);
}

void SUIStage::render() {
    SUIContainer::render();
}