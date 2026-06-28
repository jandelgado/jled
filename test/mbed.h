// Minimal mbed mock for testing JLed hardware accessing functions
// Copyright 2020 Jan Delgado jdelgado@gmx.net
//

#ifndef TEST_MBED_H_
#define TEST_MBED_H_

#include <stdint.h>
#include <chrono>  // NOLINT

using PinName = uint8_t;

constexpr auto kUninitializedPin = 255;
constexpr auto kUninitialized = -1.;
constexpr auto MBED_PINS = 32;

struct MbedState {
    uint32_t us_ticks               = 0;
    float    pin_state[MBED_PINS];          // initialized to kUninitialized by mbedMockSetInstance
    uint16_t dtor_called[MBED_PINS] = {};

    void     setUsTicks(uint32_t t)           { us_ticks = t; }
    float    getPinState(uint8_t p)   const   { return pin_state[p]; }
    uint16_t getDtorCalled(uint8_t p) const   { return dtor_called[p]; }
};

void mbedMockSetInstance(MbedState* state);

uint32_t us_ticker_read();

class PwmOut {
    PinName pin_ = kUninitializedPin;

 public:
    explicit PwmOut(PinName pin) : pin_(pin) {}
    ~PwmOut();
    void write(float val);
};

namespace Kernel {
struct Clock {
    using time_point = std::chrono::time_point<std::chrono::system_clock>;
    static time_point now();
};
};  // namespace Kernel

#endif  // TEST_MBED_H_
