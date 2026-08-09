// STM32Cube HAL for JLed
// Copyright (c) 2017-2020 Jan Delgado <jdelgado[at]gmx.net>
// https://github.com/jandelgado/jled
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
#pragma once

#include <stdint.h>  // NOLINT

#include "brightness.h"  // BrightnessTraits

// This header references STM32Cube types, macros and
// functions (TIM_HandleTypeDef, HAL_TIM_PWM_Start, HAL_TIM_PWM_ConfigChannel,
// __HAL_TIM_SET_COMPARE/GET_COMPARE, TIM_OC_InitTypeDef, TIM_OCMODE_PWM1,
// TIM_OCPOLARITY_HIGH/_LOW, TIM_OCFAST_DISABLE, HAL_GetTick) without including
// any vendor header (because the header depends on the actual MCU that is used and
// we want to keep things simple here). The family HAL header (stm32XXxx_hal.h) must
// therefore be included before <jled.h>: in CubeMX projects it arrives via main.h, in
// STM32duino via Arduino.h. On the host, test/stm32cube_hal_mock.h stands in.

namespace jled {

// A timer channel: the resource Stm32CubeHal needs, analogous to a "pin".
// htim must already be initialized (MX_TIMx_Init() / HardwareTimer) with the
// desired PWM frequency and resolution (Init.Period) before use.
struct Stm32PwmChannel {
    TIM_HandleTypeDef* htim;
    uint32_t channel;  // e.g. TIM_CHANNEL_1
};

// HAL for the STM32Cube framework, driving a pre-configured timer PWM channel.
// STM32 timer PWM (mode 1, up-counting) in a nutshell:
//   - ARR (Auto Reload Register, == Init.Period) sets frequency and resolution
//     (ARR+1 steps) together: f_pwm = f_timer / ((prescaler+1) * (ARR+1)).
//   - CNT counts 0,1,...,ARR then reloads to 0. One period is ARR+1 ticks.
//   - CCR (Capture/Compare Register) is the duty threshold. It is compared against CNT
//     every tick to decide whether the output pin is high or low.
//
//   The output is active while CNT < CCR and inactive if CNT >= CCR.
//   duty fraction is CCR / (ARR+1). In the HAL, analogWrite()
//   derives CCR from the brightness value. SetLowActive() flips the pin in hardware.
//
//  Example:
//   ARR = 7 (period 8 ticks), CCR = 3:
//     CNT:  0 1 2 3 4 5 6 7   (wraps to 0)
//     pin:  H H H L L L L L
//           |active-|---inactive---|   (active while CNT < CCR)
//
class Stm32CubeHal {
 public:
    using PinType = Stm32PwmChannel;

    // Eager init: the constructor caches Init.Period and starts PWM. Construct
    // Stm32Cube-backed JLed objects inside/below main(), after MX_TIMx_Init(),
    // never as file-scope globals (whose constructors run before main()).
    explicit Stm32CubeHal(PinType pin) noexcept
        : htim_(pin.htim), channel_(pin.channel), period_(pin.htim->Init.Period) {
        HAL_TIM_PWM_Start(htim_, channel_);
    }

    template<typename Brightness>
    void analogWrite(Brightness val) const {
        constexpr uint32_t kFull = BrightnessTraits<Brightness>::kFullBrightness;
        // Fast path: when the user's period equals JLed's full-brightness value,
        // period_ / kFull == 1 so we can skip the 64-bit multiply/divide. Otherwise scale
        // with a 64-bit intermediate: val (<=65535) * period_ (<=2^32-1) can exceed 2^32.
        uint32_t duty = (period_ == kFull)
                            ? static_cast<uint32_t>(val)
                            : static_cast<uint32_t>((static_cast<uint64_t>(val) * period_) / kFull);
        // fix full brightness: CCR == ARR is one tick short of 100% (CNT == ARR
        // stays inactive), so push CCR past ARR. Matters most at small periods;
        // symmetric across polarity since inversion is done in hardware. A
        // full-width ARR has no room for +1 (it would wrap to 0 and blank the
        // LED), so fall back to CCR == ARR there.
        if (val == kFull) {
            duty = (period_ == 0xFFFFU || period_ == 0xFFFFFFFFU) ? period_ : period_ + 1;
        }
        __HAL_TIM_SET_COMPARE(htim_, channel_, duty);
    }

    // Native hardware invert (SetLowActive() below) owns inversion entirely;
    // invert is ignored here, the same pattern as Esp32Hal/PicoHal.
    template<typename Brightness>
    void analogWrite(Brightness val, bool /*invert*/) const {
        analogWrite(val);
    }

    // Set output-compare polarity via the official HAL_TIM_PWM_ConfigChannel().
    // Reads back the current compare value first so flipping polarity does not
    // reset an in-flight duty cycle. On advanced timers (TIM1/TIM8) this also
    // resets the channel's complementary output/idle config, so use standard
    // non-complementary channels.
    void SetLowActive(bool f) const {
        TIM_OC_InitTypeDef sConfigOC = {};
        sConfigOC.OCMode = TIM_OCMODE_PWM1;
        sConfigOC.Pulse = __HAL_TIM_GET_COMPARE(htim_, channel_);
        sConfigOC.OCPolarity = f ? TIM_OCPOLARITY_LOW : TIM_OCPOLARITY_HIGH;
        sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
        HAL_TIM_PWM_ConfigChannel(htim_, &sConfigOC, channel_);
    }

    // Compiler-generated copy ctor/assignment duplicate htim_/channel_/period_
    // only; they touch no hardware, so TJLed's copy/assign never re-start PWM.

 private:
    TIM_HandleTypeDef* htim_;
    uint32_t channel_;
    uint32_t period_;
};

class Stm32CubeClock {
 public:
    static uint32_t millis() { return HAL_GetTick(); }
};

}  // namespace jled
