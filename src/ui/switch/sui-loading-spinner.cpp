#include "sui-loading-spinner.h"
#include "sui-graphics.h"
#include "sui.h"

SUILoadingSpinner::SUILoadingSpinner(std::string name)
    : SUIElement(name),
      color_(RGBA8(0, 0, 0, 255))
{
    counter_ = new SUIFpsCounter(name + ":counter");
    bounds_.w = 60;
    bounds_.h = 60;
}

SUILoadingSpinner::~SUILoadingSpinner() {
    delete counter_;
}

void SUILoadingSpinner::update(SUIInput *input) {
    SUIElement::update(input);
}

void SUILoadingSpinner::render() {
    // Compute the center of the circle and its radius
    int cx = bounds_.w / 2;
    int cy = bounds_.h / 2;
    int cr = std::min(bounds_.w, bounds_.h) / 2;

    // Compute the "active" spoke of the spinner
    float dummy;
    float fractional = modff(counter_->secondsSinceFirstTick(), &dummy);
    int active_spoke = static_cast<int>(fractional * 12); 

    // Draw 12 spokes
    for (int spoke = 0; spoke < 12; spoke++) {
        // Get the angle of this spoke
        float angle = spoke * 2 * M_PI / 12;

        // Compute the distance on the unit circle
        float dx_unit = cosf(angle),
              dy_unit = sinf(angle);

        // Find the close and far positions on the spoke
        float dx_close = dx_unit * 0.5 * cr;
        float dx_far   = dx_unit * cr;
        float dy_close = dy_unit * 0.5 * cr;
        float dy_far   = dy_unit * cr;

        // Fade the current spoke based on its distance to the active spoke
        int dspoke = (active_spoke - spoke + 12) % 12;
        uint8_t alpha = static_cast<uint8_t>(255 * (12 - dspoke) / 12.0);

        // Draw the line
        graphics()->drawLine(
            cx + dx_close, 
            cy + dy_close, 
            cx + dx_far,
            cy + dy_far,
            (color_ & 0x00FFFFFF) | (alpha << 24)
        );
    }
}

uint32_t &SUILoadingSpinner::color() {
    return color_;
}