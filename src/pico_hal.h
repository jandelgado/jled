// Copyright (c) 2021-2026 Jan Delgado <jdelgado[at]gmx.net>
// https://github.com/jandelgado/jled
//
// This HAL uses the Raspi Pico SDK: https://github.com/raspberrypi/pico-sdk
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//
// PWM Configuration for RP2040/RP2350
// ===================================
//
// The RP2040 has 8 PWM slices (numbered 0-7), and each slice has two channels (A and B). That gives
// you up to 16 PWM outputs total. Each GPIO pin on the chip is mapped to a specific slice and
// channel. The mapping follows a fixed pattern: GPIO 0 -> Slice 0 Channel A,
// GPIO 1 -> Slice 0 Channel B,
// GPIO 2 -> Slice 1 Channel A, and so on, wrapping around after GPIO 15.
//
// NOTE: Both channels of a slice share the same wrap/clkdiv settings, so two PicoHal instances on
// pins that map to the same slice (e.g. GPIO 10 and 11 both use slice 5) must use the same
// kResBits_ template parameter, otherwise they will silently overwrite each other's configuration.
//
// With a fixed clock divider of 1 (int=1, frac=0), the only parameter
// that determines both resolution and frequency is the wrap value:
//
//   wrap  = 2^bits - 1
//   f_pwm = f_sys / (wrap + 1)  =  f_sys / 2^bits
//
// At f_sys = 125 MHz (RP2040) and 150 MHz (RP2350) this gives approximate frequencies:
//
// | Bits | Wrap     | RP2040 (Hz)  | RP2350 (Hz)  |
// |------|----------|--------------|--------------|
// |   8  |      255 |      488,281 |      585,938 |
// |  10  |     1023 |      122,070 |      146,484 |
// |  12  |     4095 |       30,518 |       36,621 |
// |  16  |    65535 |        1,907 |        2,289 |
//
// Even at 16-bit resolution the PWM frequency remains above 1 kHz, which produces smooth
// visible output on LEDs.
//
#pragma once

#include "hardware/pwm.h"
#include "pico/time.h"
#include "brightness.h"
#include "jled_std.h"

namespace jled {

template <uint8_t kResBits_ = 8>
class PicoHal {
 public:
    using PinType = uint8_t;
    using NativeBrightness =
        typename Conditional<(kResBits_ > 8), uint16_t, uint8_t>::type;
    static constexpr uint8_t kNativeBits = kResBits_;
    static constexpr NativeBrightness kMaxBrightness = (1u << kResBits_) - 1;

 private:
    // divider=1.0, so wrap = 2^kResBits_-1 gives exactly kResBits_ resolution
    static constexpr uint16_t kWrap = kMaxBrightness;

 public:
    explicit PicoHal(PinType pin) noexcept {
        slice_num_ = pwm_gpio_to_slice_num(pin);
        channel_ = pwm_gpio_to_channel(pin);
        gpio_set_function(pin, GPIO_FUNC_PWM);

        pwm_set_wrap(slice_num_, kWrap);
        pwm_set_clkdiv_int_frac(slice_num_, 1, 0);
        pwm_set_enabled(slice_num_, true);
    }

    template<typename Brightness>
    void analogWrite(Brightness val) const {
        const uint16_t duty = jled::scaleToNative<kResBits_>(val);
        pwm_set_chan_level(slice_num_, channel_, duty);
    }

 private:
    uint8_t slice_num_, channel_;
};

class PicoClock {
 public:
    static uint32_t millis() { return to_ms_since_boot(get_absolute_time()); }
};

}  // namespace jled
