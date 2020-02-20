// Minimal Arduino mock for testing JLed hardware accessing functions
// Copyright 2017 Jan Delgado jdelgado@gmx.net
//
#include "Arduino.h"  // NOLINT
#include <time.h>     // NOLINT
#include <cstring>    // NOLINT


struct ArduinoState {
    time_t millis;  // current time

    int pin_state[ARDUINO_PINS];
    uint8_t pin_modes[ARDUINO_PINS];
} ArduinoState_;

void arduinoMockInit() {
    // TODO(jd) introduce UNDEFINED state to mock instead of initalizing with 0
    bzero(&ArduinoState_, sizeof(ArduinoState_));
}

void pinMode(uint8_t pin, uint8_t mode) { ArduinoState_.pin_modes[pin] = mode; }

uint8_t arduinoMockGetPinMode(uint8_t pin) {
    return ArduinoState_.pin_modes[pin];
}

void analogWrite(uint8_t pin, int value) {
    ArduinoState_.pin_state[pin] = value;
}

int arduinoMockGetPinState(uint8_t pin) { return ArduinoState_.pin_state[pin]; }

uint32_t millis(void) { return ArduinoState_.millis; }

void arduinoMockSetMillis(uint32_t value) { ArduinoState_.millis = value; }

