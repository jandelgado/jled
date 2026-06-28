// Arduino mock for unit testing JLed.
// Copyright 2017 Jan Delgado jdelgado@gmx.net
#ifndef TEST_ARDUINO_H_
#define TEST_ARDUINO_H_

#include <math.h>
#include <stdint.h>
#include <cstring>

constexpr auto ARDUINO_PINS = 32;

struct ArduinoState {
    uint32_t millis = 0;
    int pin_state[ARDUINO_PINS] = {};
    uint8_t pin_modes[ARDUINO_PINS] = {};
    int analog_write_resolution = 0;

    void setMillis(uint32_t v) { millis = v; }
    uint32_t getMillis() const { return millis; }

    int getPinState(uint8_t pin) const { return pin_state[pin]; }
    uint8_t getPinMode(uint8_t pin) const { return pin_modes[pin]; }
    int getAnalogWriteResolution() const { return analog_write_resolution; }
};

void arduinoMockSetInstance(ArduinoState* s);

void pinMode(uint8_t pin, uint8_t mode);
void analogWrite(uint8_t pin, int value);
extern "C" void analogWriteResolution(int bits);
uint32_t millis(void);

#define OUTPUT 0x1

#endif  // TEST_ARDUINO_H_
