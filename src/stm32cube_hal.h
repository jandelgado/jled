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

#include <stdint.h>

#include "brightness.h"  // BrightnessTraits
#include "jled_std.h"

// Symbol provenance: this header references STM32Cube types, macros and
// functions (TIM_HandleTypeDef, HAL_TIM_PWM_Start, HAL_TIM_PWM_ConfigChannel,
// __HAL_TIM_SET_COMPARE/GET_COMPARE, TIM_OC_InitTypeDef, TIM_OCMODE_PWM1,
// TIM_OCPOLARITY_HIGH/_LOW, TIM_OCFAST_DISABLE, HAL_GetTick) without including
// any vendor header. The family HAL header (stm32XXxx_hal.h) must already be
// visible: in CubeMX projects via main.h, in STM32duino via Arduino.h, both
// pulled in before <jled.h>. On the host, test/stm32cube_hal_mock.h stands in.

namespace jled {

// A timer channel: the resource Stm32CubeHal needs, analogous to a "pin".
// htim must already be initialized (MX_TIMx_Init() / HardwareTimer) with the
// desired PWM frequency and resolution (Init.Period) before use.
struct Stm32PwmChannel {
    TIM_HandleTypeDef* htim;
    uint32_t channel;  // e.g. TIM_CHANNEL_1
};

// HAL for the STM32Cube framework, driving a pre-configured timer PWM channel.
// Eager init: the constructor caches Init.Period and starts PWM. Construct
// Stm32Cube-backed JLed objects inside/below main(), after MX_TIMx_Init(),
// never as file-scope globals (whose constructors run before main()).
class Stm32CubeHal {
 public:
    using PinType = Stm32PwmChannel;

    explicit Stm32CubeHal(PinType pin) noexcept
        : htim_(pin.htim), channel_(pin.channel), period_(pin.htim->Init.Period) {
        HAL_TIM_PWM_Start(htim_, channel_);
    }

    template <typename Brightness>
    void analogWrite(Brightness val) const {
        constexpr uint32_t kFull = BrightnessTraits<Brightness>::kFullBrightness;
        // 64-bit intermediate: val (<=65535) * period_ (<=2^32-1) can exceed 2^32.
        uint32_t duty =
            static_cast<uint32_t>((static_cast<uint64_t>(val) * period_) / kFull);
        // CCR > ARR forces the output permanently active in PWM mode 1, the only
        // way to reach true 100% duty (CCR == ARR is briefly inactive for one tick).
        if (val == kFull) duty = period_ + 1;
        __HAL_TIM_SET_COMPARE(htim_, channel_, duty);
    }

    // Native hardware invert (SetLowActive() below) owns inversion entirely;
    // invert is ignored here, the same pattern as Esp32Hal/PicoHal.
    template <typename Brightness>
    void analogWrite(Brightness val, bool /*invert*/) const {
        analogWrite(val);
    }

    // Set output-compare polarity via the official HAL_TIM_PWM_ConfigChannel().
    // Reads back the current compare value first so flipping polarity does not
    // reset an in-flight duty cycle (same read-back pattern as Esp32Hal).
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
