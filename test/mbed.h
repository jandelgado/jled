// Minimal mbed mock for testing JLed hardware accessing functions
// Copyright 2020 Jan Delgado jdelgado@gmx.net
//

#ifndef TEST_MBED_H_
#define TEST_MBED_H_

#include <stdint.h>

using PinName = uint8_t;

constexpr auto kUninitializedPin = 255;
constexpr auto kUninitialized = -1.;

uint32_t us_ticker_read();

class PwmOut {
    PinName pin_ = kUninitializedPin;

 public:
    explicit PwmOut(PinName pin) : pin_(pin) {}
    void write(float val);
};

void mbedMockInit();
void mbedMockSetUsTicks(uint32_t ticks);
float mbedMockGetPinState(uint8_t pin);

#endif  // TEST_MBED_H_
