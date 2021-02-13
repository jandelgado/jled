// Copyright (c) 2021 Jan Delgado <jdelgado[at]gmx.net>
// https://github.com/jandelgado/jled
// HAL for the Raspi Pico
// This HAL uses the Raspi Pico SDK: https://github.com/raspberrypi/pico-sdk
//
// Adapted from https://pastebin.com/uVMigyFN (Scott Beasley) and
// https://github.com/raspberrypi/micropython/blob/pico/ports/rp2/machine_pwm.c
//   (Copyright (c) 2020 Damien P. George)
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
#ifndef SRC_PICO_HAL_H_
#define SRC_PICO_HAL_H_

namespace jled {

#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "pico/time.h"

class PicoHal {
    static constexpr auto TOP_MAX = 65534;
    static constexpr auto DUTY_100_PCT = 65535;
    static constexpr auto DEFAULT_FREQ_HZ = 5000;

    static int set_pwm_freq(uint slice, int freq, uint32_t *div,
                            uint32_t *top_) {
        // Set the frequency, making "top_" as large as possible for maximum
        // resolution.
        *div = (uint32_t)(16 * clock_get_hz(clk_sys) / (uint32_t)freq);
        *top_ = 1;
        for (;;) {
            // Try a few small prime factors to get close to the desired
            // frequency.
            if (*div >= 16 * 5 && *div % 5 == 0 && *top_ * 5 <= TOP_MAX) {
                *div /= 5;
                *top_ *= 5;
            } else if (*div >= 16 * 3 && *div % 3 == 0 &&
                       *top_ * 3 <= TOP_MAX) {
                *div /= 3;
                *top_ *= 3;
            } else if (*div >= 16 * 2 && *top_ * 2 <= TOP_MAX) {
                *div /= 2;
                *top_ *= 2;
            } else {
                break;
            }
        }

        if (*div < 16) {
            *div = 0;
            *top_ = 0;
            return -1;  // freq too large
        } else if (*div >= 256 * 16) {
            *div = 0;
            *top_ = 0;
            return -2;  // freq too small
        }

        return 0;
    }

    static void set_pwm_duty(uint slice, uint channel, uint32_t top,
                             uint32_t duty) {
        const uint32_t cc = duty * (top + 1) / 65535;
        pwm_set_chan_level(slice, channel, cc);
        pwm_set_enabled(slice, true);
    }

 public:
    using PinType = uint8_t;

    explicit PicoHal(PinType pin) noexcept {
        slice_num_ = pwm_gpio_to_slice_num(pin);
        channel_ = pwm_gpio_to_channel(pin);
        gpio_set_function(pin, GPIO_FUNC_PWM);

        uint32_t div = 0;
        set_pwm_freq(slice_num_, DEFAULT_FREQ_HZ, &div, &top_);
        // TODO(jd) check return value?

        pwm_set_wrap(slice_num_, top_);
    }

    void analogWrite(uint8_t val) const {
        set_pwm_duty(slice_num_, channel_, top_,
                     (uint32_t)(DUTY_100_PCT / 255) * val);
    }

    uint32_t millis() const { return to_ms_since_boot(get_absolute_time()); }

 private:
    uint slice_num_, channel_;
    uint32_t top_ = 0;
};
}  // namespace jled
#endif  // SRC_PICO_HAL_H_
