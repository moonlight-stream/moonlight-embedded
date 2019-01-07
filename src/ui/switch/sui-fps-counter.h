#pragma once

#include "sui-element.h"

#define MAX_TICK_SAMPLES 100
#define TICKS_PER_SECOND 19200000

class SUIFpsCounter : public SUIElement {
public:
    SUIFpsCounter();
    ~SUIFpsCounter();

    void update(SUIInput *) override;
    void render() override;

    uint64_t frame();
    uint64_t framesPerSecond();
    uint64_t ticksPerFrame();

private:
    uint64_t frame_;
    uint64_t frames_per_second_;
    uint64_t ticks_per_frame_;

    uint64_t last_time_;
    uint64_t tick_index_;
    uint64_t tick_sum_;
    uint64_t tick_list_[MAX_TICK_SAMPLES];
};
