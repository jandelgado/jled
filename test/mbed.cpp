// Minimal mbed mock for testing JLed hardware accessing functions
// Copyright 2020 Jan Delgado jdelgado@gmx.net
//
#include "mbed.h"  // NOLINT
#include <cassert>

static MbedState* gState_ = nullptr;

void mbedMockSetInstance(MbedState* s) {
    gState_ = s;
    if (s != nullptr) {
        *s = MbedState{};
        for (auto i = 0; i < MBED_PINS; i++) {
            s->pin_state[i] = kUninitialized;
        }
    }
}

uint32_t us_ticker_read() {
    assert(gState_);
    return gState_->us_ticks;
}

void PwmOut::write(float val) {
    assert(gState_);
    gState_->pin_state[pin_] = val;
}

PwmOut::~PwmOut() {
    assert(gState_);
    gState_->dtor_called[pin_]++;
}

Kernel::Clock::time_point Kernel::Clock::now() {
    assert(gState_);
    return Kernel::Clock::time_point(
        std::chrono::microseconds(gState_->us_ticks));
}
