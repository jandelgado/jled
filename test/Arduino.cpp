// Minimal Arduino mock for testing JLed
// Copyright 2017 Jan Delgado jdelgado@gmx.net
//
#include "Arduino.h"  // NOLINT
#include <time.h>     // NOLINT
#include <cstring>    // NOLINT


struct ArduinoState {
    time_t millis;  // current time

    int pin_state[ARDUINO_PINS];
    uint8_t pin_modes[ARDUINO_PINS];

    // records ESP32 specific calls to ledc* functions.
    uint32_t ledc_state[LEDC_CHANNELS];
    struct LedcSetupState ledc_setup[LEDC_CHANNELS];
    uint8_t ledc_pin_attachments[ARDUINO_PINS];
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

// EPS32 specific
double ledcSetup(uint8_t chan, double freq, uint8_t bit_num) {
    ArduinoState_.ledc_setup[chan] = {freq, bit_num};
    return 0;  // not used.
}

struct LedcSetupState arduinoMockGetLedcSetup(uint8_t chan) {
    return ArduinoState_.ledc_setup[chan];
}

// EPS32 specific
void ledcAttachPin(uint8_t pin, uint8_t chan) {
    ArduinoState_.ledc_pin_attachments[pin] = chan;
}

uint8_t arduinoMockGetLedcAttachPin(uint8_t pin) {
    return ArduinoState_.ledc_pin_attachments[pin];
}

// EPS32 specific
void ledcWrite(uint8_t chan, uint32_t duty) {
    ArduinoState_.ledc_state[chan] = duty;
}

uint32_t arduinoMockGetLedcState(uint8_t chan) {
    return ArduinoState_.ledc_state[chan];
}

