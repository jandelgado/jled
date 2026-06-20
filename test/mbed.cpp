// Minimal mbed mock for testing JLed hardware accessing functions
// Copyright 2020 Jan Delgado jdelgado@gmx.net
//
#include "mbed.h"  // NOLINT

constexpr auto MBED_PINS = 32;

struct MbedState {
    uint32_t us_ticks;
    float pin_state[MBED_PINS];
    uint16_t dtor_called[MBED_PINS];
} MbedState_;

void mbedMockInit() {
    for (auto i = 0; i < MBED_PINS; i++) {
        MbedState_.pin_state[i] = kUninitialized;
        MbedState_.dtor_called[i] = 0;
    }
    MbedState_.us_ticks = 0;
}

void mbedMockWrite(PinName pin, float value) {
    MbedState_.pin_state[pin] = value;
}

float mbedMockGetPinState(uint8_t pin) { return MbedState_.pin_state[pin]; }

uint16_t mbedMockGetDtorCalled(uint8_t pin) {return MbedState_.dtor_called[pin];}

void mbedMockSetUsTicks(uint32_t ticks) { MbedState_.us_ticks = ticks; }

uint32_t us_ticker_read() { return MbedState_.us_ticks; }

void PwmOut::write(float val) { mbedMockWrite(pin_, val); }

PwmOut::~PwmOut() { MbedState_.dtor_called[pin_]++; }

Kernel::Clock::time_point Kernel::Clock::now() {
    return Kernel::Clock::time_point(
        std::chrono::microseconds(MbedState_.us_ticks));
}
