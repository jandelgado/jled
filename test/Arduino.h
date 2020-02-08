// Arduino mock for unit testing JLed.
// Copyright 2017 Jan Delgado jdelgado@gmx.net
#ifndef TEST_ARDUINO_H_
#define TEST_ARDUINO_H_

#include <math.h>
#include <stdint.h>
#include <cstring>
#include "esp32.h"  // NOLINT

constexpr auto ARDUINO_PINS = 32;

void arduinoMockInit();

void pinMode(uint8_t pin, uint8_t mode);
uint8_t arduinoMockGetPinMode(uint8_t pin);

void analogWrite(uint8_t pint, int value);
int arduinoMockGetPinState(uint8_t pin);

uint32_t millis(void);
void arduinoMockSetMillis(uint32_t value);

#define PI 3.1415926535897932384626433832795
#define OUTPUT 0x1


#endif  // TEST_ARDUINO_H_
