// Minimal Pico SDK mock for testing JLed hardware accessing functions
// Copyright 2024-2025 Jan Delgado jdelgado@gmx.net
#include "pico_mock.h"          // NOLINT
#include <cassert>
#include "pico-sdk/hardware/clocks.h"  // NOLINT
#include "pico-sdk/hardware/pwm.h"     // NOLINT
#include "pico-sdk/pico/time.h"        // NOLINT

static PicoState* gState_ = nullptr;

void picoMockSetInstance(PicoState* state) {
    gState_ = state;
    if (state) {
        *state = PicoState{};
    }
}

// --- Pico SDK mock implementations ---

uint32_t clock_get_hz(clock_index_t /*clk*/) {
    assert(gState_); return gState_->clock_hz;
}

uint pwm_gpio_to_slice_num(uint gpio) { return gpio / 2; }
uint pwm_gpio_to_channel(uint gpio)   { return gpio % 2; }

void gpio_set_function(uint gpio, gpio_function_t fn) {
    assert(gState_); gState_->gpio_function[gpio] = static_cast<uint32_t>(fn);
}

void pwm_set_wrap(uint slice_num, uint16_t wrap) {
    assert(gState_); gState_->slices[slice_num].wrap = wrap;
}

void pwm_set_clkdiv_int_frac(uint slice_num, uint8_t integer, uint8_t fract) {
    assert(gState_);
    gState_->slices[slice_num].div_int  = integer;
    gState_->slices[slice_num].div_frac = fract;
}

void pwm_set_enabled(uint slice_num, bool enabled) {
    assert(gState_); gState_->slices[slice_num].enabled = enabled;
}

void pwm_set_chan_level(uint slice_num, uint chan, uint16_t level) {
    assert(gState_); gState_->slices[slice_num].chan_level[chan] = level;
}

absolute_time_t get_absolute_time() { assert(gState_); return gState_->boot_time_us; }

uint32_t to_ms_since_boot(absolute_time_t t) {
    return static_cast<uint32_t>(t / 1000u);
}
