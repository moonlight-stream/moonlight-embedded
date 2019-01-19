#include "sui-fps-counter.h"
#include "sui-graphics.h"
#include "sui.h"

SUIFpsCounter::SUIFpsCounter(std::string name) 
    : SUIElement(name),
      tick_list_{}
{
    frame_ = 0;
    frames_per_second_ = 0;
    ticks_per_frame_ = 0;

    last_time_ = 0;
    tick_index_ = 0;
    tick_sum_ = 0;

    first_tick_ = svcGetSystemTick();
}

SUIFpsCounter::~SUIFpsCounter() {
    
}

void SUIFpsCounter::update(SUIInput *input) {
    uint64_t now = svcGetSystemTick();

    if (last_time_) {
        uint64_t delta = now - last_time_;

        tick_sum_ -= tick_list_[tick_index_];
        tick_sum_ += delta;
        tick_list_[tick_index_] = delta;

        if (++tick_index_ == MAX_TICK_SAMPLES)
            tick_index_ = 0;

        ticks_per_frame_ = tick_sum_ / MAX_TICK_SAMPLES;
        frames_per_second_ = TICKS_PER_SECOND / ticks_per_frame_;
    }

    last_time_ = now;
    frame_++;
}

void SUIFpsCounter::render() {
    uint32_t fps_color = RGBA8(180, 0, 0, 255);
    char fps_text[20];
    int fps_text_width, fps_text_height;
    snprintf(fps_text, 20, "FPS: %ld", frames_per_second_);
    graphics()->measureText(ui_->font_small, fps_text, &fps_text_width, &fps_text_height);

    graphics()->drawBox(8, 8, 10 + fps_text_width, fps_text_height + 2, RGBA8(255, 255, 255, 200));
    graphics()->drawText(ui_->font_small, fps_text, 18, 10, fps_color, false, -1);

    short progressDigit = frame_ % fps_text_height;
    graphics()->drawBox(10, 10, 2, progressDigit, fps_color);
}

uint64_t SUIFpsCounter::frame() {
    return frame_;
}

uint64_t SUIFpsCounter::framesPerSecond() {
    return frames_per_second_;
}

uint64_t SUIFpsCounter::ticksPerFrame() {
    return ticks_per_frame_;
}

float SUIFpsCounter::secondsSinceFirstTick() {
    return (float)(svcGetSystemTick() - first_tick_) / TICKS_PER_SECOND;
}
