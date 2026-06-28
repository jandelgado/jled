// Minimal Arduino mock for testing JLed hardware accessing functions
// Copyright 2017 Jan Delgado jdelgado@gmx.net
//
#include "Arduino.h"  // NOLINT
#include <cassert>    // NOLINT

static ArduinoState* gState_ = nullptr;

void arduinoMockSetInstance(ArduinoState* s) {
    gState_ = s;
    if (s) {
        *s = ArduinoState{};
    }
}

void pinMode(uint8_t pin, uint8_t mode) {
    assert(gState_);
    gState_->pin_modes[pin] = mode;
}

void analogWrite(uint8_t pin, int value) {
    assert(gState_);
    gState_->pin_state[pin] = value;
}

uint32_t millis(void) {
    assert(gState_);
    return gState_->millis;
}

extern "C" void analogWriteResolution(int bits) {
    assert(gState_);
    gState_->analog_write_resolution = bits;
}
