// Minimal esp32 mock for testing JLed hardware accessing functions
// Copyright 2017 Jan Delgado jdelgado@gmx.net
//
#include "esp32.h"  // NOLINT

// constexpr auto ESP32_PINS = 32;
// constexpr auto LEDC_CHANNELS = 16;

struct Esp32State {
    uint32_t ledc_state[LEDC_CHANNELS];
    struct LedcSetupState ledc_setup[LEDC_CHANNELS];
    uint8_t ledc_pin_attachments[ESP32_PINS];
} Esp32State_;

void esp32MockInit() {
    // TODO(jd) introduce UNDEFINED state to mock instead of initalizing with 0
    bzero(&Esp32State_, sizeof(Esp32State_));
}

double ledcSetup(uint8_t chan, double freq, uint8_t bit_num) {
    Esp32State_.ledc_setup[chan] = {freq, bit_num};
    return 0;  // not used.
}

struct LedcSetupState arduinoMockGetLedcSetup(uint8_t chan) {
    return Esp32State_.ledc_setup[chan];
}

void ledcAttachPin(uint8_t pin, uint8_t chan) {
    Esp32State_.ledc_pin_attachments[pin] = chan;
}

uint8_t arduinoMockGetLedcAttachPin(uint8_t pin) {
    return Esp32State_.ledc_pin_attachments[pin];
}

void ledcWrite(uint8_t chan, uint32_t duty) {
    Esp32State_.ledc_state[chan] = duty;
}

uint32_t arduinoMockGetLedcState(uint8_t chan) {
    return Esp32State_.ledc_state[chan];
}
