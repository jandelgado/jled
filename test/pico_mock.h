// Minimal Pico SDK mock for testing JLed hardware accessing functions
// Copyright 2024-2025 Jan Delgado jdelgado@gmx.net
#ifndef TEST_PICO_MOCK_H_
#define TEST_PICO_MOCK_H_

#include <stdint.h>

struct PicoSliceState {
    uint16_t wrap        = 0;
    uint8_t  div_int     = 0;
    uint8_t  div_frac    = 0;
    bool     enabled     = false;
    uint16_t chan_level[2] = {};
};

struct PicoState {
    uint32_t       clock_hz         = 125000000u;
    uint64_t       boot_time_us     = 0;
    PicoSliceState slices[8]        = {};
    uint32_t       gpio_function[30] = {};

    void setClockHz(uint32_t hz)    { clock_hz = hz; }
    void setBootTimeUs(uint64_t us) { boot_time_us = us; }

    uint32_t getGpioFunction(uint32_t gpio) const   { return gpio_function[gpio]; }
    uint16_t getPwmWrap(uint32_t slice) const        { return slices[slice].wrap; }
    uint8_t  getPwmDivInt(uint32_t slice) const      { return slices[slice].div_int; }
    uint8_t  getPwmDivFrac(uint32_t slice) const     { return slices[slice].div_frac; }
    bool     getPwmEnabled(uint32_t slice) const     { return slices[slice].enabled; }
    uint16_t getPwmChanLevel(uint32_t slice, uint32_t chan) const {
        return slices[slice].chan_level[chan];
    }
};

// Register the active mock instance. Resets it to default values.
// Pass nullptr in fixture destructor to clear the pointer.
void picoMockSetInstance(PicoState* state);

#endif  // TEST_PICO_MOCK_H_
