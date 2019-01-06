#pragma once

#include "switch/sui.h"

#include <switch.h>

#define MAX_TICK_SAMPLES 100
#define TICKS_PER_SECOND 19200000

class UiFpsCounter {
public:
    UiFpsCounter(SUI *ui);

    void update();
    void render();

    uint64_t frame();
    uint64_t framesPerSecond();
    uint64_t ticksPerFrame();

private:
    SUI *ui_;

    uint64_t frame_;
    uint64_t frames_per_second_;
    uint64_t ticks_per_frame_;

    uint64_t last_time_;
    uint64_t tick_index_;
    uint64_t tick_sum_;
    uint64_t tick_list_[MAX_TICK_SAMPLES];
};
