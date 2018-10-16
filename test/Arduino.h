// Arduino mock for unit testing JLed.
// Copyright 2017 Jan Delgado jdelgado@gmx.net
#ifndef TEST_ARDUINO_H_
#define TEST_ARDUINO_H_

#include <math.h>
#include <stdint.h>

constexpr auto ARDUINO_PINS = 32;
constexpr auto LEDC_CHANNELS = 16;

void arduinoMockInit();

void pinMode(uint8_t pin, uint8_t mode);
uint8_t arduinoMockGetPinMode(uint8_t pin);

void analogWrite(uint8_t pint, int value);
int arduinoMockGetPinState(uint8_t pin);

uint32_t millis(void);
void arduinoMockSetMillis(uint32_t value);

//#define min(a, b) ((a) < (b) ? (a) : (b))
//#define max(a, b) ((a) > (b) ? (a) : (b))

#define PI 3.1415926535897932384626433832795
#define OUTPUT 0x1

// ESP32 sepcific functions, see
// packages/framework-arduinoespressif32/cores/esp32/esp32-hal-ledc.h
double ledcSetup(uint8_t chan, double freq, uint8_t bit_num);

// this struct records calls to ledcSetup
struct LedcSetupState {
    double freq;
    uint8_t bit_num;
};

struct LedcSetupState arduinoMockGetLedcSetup(uint8_t chan);

void ledcAttachPin(uint8_t pin, uint8_t chan);
uint8_t arduinoMockGetLedcAttachPin(uint8_t pin);

void ledcWrite(uint8_t chan, uint32_t duty);
uint32_t arduinoMockGetLedcState(uint8_t chan);


#endif  // TEST_ARDUINO_H_
