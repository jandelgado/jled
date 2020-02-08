// ESP32 mock for unit testing JLed.
// Copyright 2017 Jan Delgado jdelgado@gmx.net
#ifndef TEST_ESP32_H_
#define TEST_ESP32_H_

#include <stdint.h>
#include <cstring>

constexpr auto LEDC_CHANNELS = 16;
constexpr auto ESP32_PINS = 32;

void esp32MockInit();

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

#endif  // TEST_ESP32_H_
