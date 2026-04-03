// Minimal Pico SDK mock for testing JLed hardware accessing functions
// Copyright 2024-2025 Jan Delgado jdelgado@gmx.net
#include "pico_mock.h"          // NOLINT
#include "pico-sdk/hardware/clocks.h"  // NOLINT
#include "pico-sdk/hardware/pwm.h"     // NOLINT
#include "pico-sdk/pico/time.h"        // NOLINT

static constexpr auto kNumSlices = 8;
static constexpr auto kNumGpios  = 30;

struct PicoSliceState {
    uint16_t wrap;
    uint8_t  div_int;
    uint8_t  div_frac;
    bool     enabled;
    uint16_t chan_level[2];
};

struct PicoState {
    uint32_t       clock_hz;
    uint64_t       boot_time_us;
    PicoSliceState slices[kNumSlices];
    uint32_t       gpio_function[kNumGpios];
} PicoState_;

void picoMockInit() {
    PicoState_ = PicoState{};
    PicoState_.clock_hz = 125000000u;
}

void picoMockSetClockHz(uint32_t hz)   { PicoState_.clock_hz = hz; }
void picoMockSetBootTimeUs(uint64_t us) { PicoState_.boot_time_us = us; }

uint32_t picoMockGetGpioFunction(uint32_t gpio) {
    return PicoState_.gpio_function[gpio];
}
uint16_t picoMockGetPwmWrap(uint32_t slice) {
    return PicoState_.slices[slice].wrap;
}
uint8_t picoMockGetPwmDivInt(uint32_t slice) {
    return PicoState_.slices[slice].div_int;
}
uint8_t picoMockGetPwmDivFrac(uint32_t slice) {
    return PicoState_.slices[slice].div_frac;
}
bool picoMockGetPwmEnabled(uint32_t slice) {
    return PicoState_.slices[slice].enabled;
}
uint16_t picoMockGetPwmChanLevel(uint32_t slice, uint32_t chan) {
    return PicoState_.slices[slice].chan_level[chan];
}

// --- Pico SDK mock implementations ---

uint32_t clock_get_hz(clock_index_t /*clk*/) { return PicoState_.clock_hz; }

uint pwm_gpio_to_slice_num(uint gpio) { return gpio / 2; }
uint pwm_gpio_to_channel(uint gpio)   { return gpio % 2; }

void gpio_set_function(uint gpio, gpio_function_t fn) {
    PicoState_.gpio_function[gpio] = static_cast<uint32_t>(fn);
}

void pwm_set_wrap(uint slice_num, uint16_t wrap) {
    PicoState_.slices[slice_num].wrap = wrap;
}

void pwm_set_clkdiv_int_frac(uint slice_num, uint8_t integer, uint8_t fract) {
    PicoState_.slices[slice_num].div_int  = integer;
    PicoState_.slices[slice_num].div_frac = fract;
}

void pwm_set_enabled(uint slice_num, bool enabled) {
    PicoState_.slices[slice_num].enabled = enabled;
}

void pwm_set_chan_level(uint slice_num, uint chan, uint16_t level) {
    PicoState_.slices[slice_num].chan_level[chan] = level;
}

absolute_time_t get_absolute_time() { return PicoState_.boot_time_us; }

uint32_t to_ms_since_boot(absolute_time_t t) {
    return static_cast<uint32_t>(t / 1000u);
}
