// Copyright (c) 2017-2022 Jan Delgado <jdelgado[at]gmx.net>
// https://github.com/jandelgado/jled
//
// HAL for the ESP32 compatible with Arduino and ESP-IDF framework. Uses
// ESP-IDF SDK under the hood.
//
// Documentation:
// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/ledc.html
// https://documentation.espressif.com/esp32_technical_reference_manual_en.pdf#ledpwm
//
// Inspiration from:
// https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/esp32-hal-ledc.c
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

#include <driver/ledc.h>
#include <esp_timer.h>
#include <stdint.h>
#include "brightness.h"

namespace jled {

class Esp32ChanMapper {
    static constexpr auto kFreeChan = 0xff;

 public:
    using PinType = uint8_t;

    static constexpr int kLedcMaxChan = LEDC_CHANNEL_MAX;

    Esp32ChanMapper() {
        for (auto i = 0; i < kLedcMaxChan; i++) chanMap_[i] = 0xff;
    }

    ledc_channel_t chanForPin(PinType pin) {
        // find existing channel for given pin
        for (auto i = 0; i < kLedcMaxChan; i++) {
            if (chanMap_[i] == pin) return (ledc_channel_t)i;
        }
        // find and return first free slot
        for (auto i = 0; i < kLedcMaxChan; i++) {
            if (chanMap_[i] == kFreeChan) {
                chanMap_[i] = pin;
                return (ledc_channel_t)i;
            }
        }
        // no more free slots, start over
        const auto i = nextChan_;
        chanMap_[i] = pin;
        nextChan_ = (nextChan_ + 1) % kLedcMaxChan;
        return (ledc_channel_t)i;
    }

 private:
    PinType nextChan_ = 0;
    PinType chanMap_[kLedcMaxChan];
};

// All Esp32Hal template instantiations must share the same Esp32ChanMapper instance,
// so channels for different resolutions end up in the one and only channel mapper.
class Esp32HalBase {
protected:
    static Esp32ChanMapper& chanMapper() {
        static Esp32ChanMapper instance;
        return instance;
    }
};

template<uint8_t kResBits_ = 8, ledc_timer_t kTimer_ = LEDC_TIMER_0>
class Esp32Hal : public Esp32HalBase {
    static constexpr auto kLedcSpeedMode = LEDC_LOW_SPEED_MODE;

 public:
    using PinType = Esp32ChanMapper::PinType;
    using NativeBrightness = uint16_t;
    static constexpr uint8_t kNativeBits = kResBits_;
	static constexpr uint16_t kMaxBrightness = (1 << kResBits_) - 1;

    static constexpr auto kAutoSelectChan = -1;

    // Construct an ESP32 HAL object for the given GPIO pin.
    // pin   GPIO pin to connect to.
    // chan  LEDC channel to use. kAutoSelectChan (default) picks the next free
    //       channel automatically; pass an explicit value to override.
    // freq  LEDC base frequency in Hz (default: 5000).
    // The PWM resolution (kResBits_) and timer (kTimer_) are template parameters.
    Esp32Hal(PinType pin, int chan = kAutoSelectChan, uint16_t freq = 5000) noexcept {
        chan_ = (chan == kAutoSelectChan)
                    ? Esp32Hal::chanMapper().chanForPin(pin)
                    : (ledc_channel_t)chan;

        ledc_timer_config_t ledc_timer{};
        ledc_timer.speed_mode = kLedcSpeedMode;
        ledc_timer.duty_resolution = static_cast<ledc_timer_bit_t>(kNativeBits);
        ledc_timer.timer_num = kTimer_;
        ledc_timer.freq_hz = freq;
#if ESP_IDF_VERSION_MAJOR > 3
        ledc_timer.clk_cfg = LEDC_AUTO_CLK;
#endif
        ledc_timer_config(&ledc_timer);

        ledc_channel_config_t ledc_channel{};
        ledc_channel.gpio_num = pin;
        ledc_channel.speed_mode = kLedcSpeedMode;
        ledc_channel.channel = chan_;
        ledc_channel.intr_type = LEDC_INTR_DISABLE;
        ledc_channel.timer_sel = kTimer_;
        ledc_channel.duty = 0;
        ledc_channel.hpoint = 0;
#if ESP_IDF_VERSION_MAJOR > 4
        ledc_channel.flags.output_invert = 0;
#endif
        ledc_channel_config(&ledc_channel);
    }

    template<typename Brightness>
    void analogWrite(Brightness val) const {
        // Scale brightness to actual resolution
        const uint16_t duty = jled::scaleToNative<kNativeBits>(val);

		// from: https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/ledc.html
        // The range of the duty cycle values passed to functions depends on selected
        // duty_resolution and should be from 0 to (2 ** duty_resolution). For example, if the
        // selected duty resolution is 10, then the duty cycle values can range from 0 to 1024. This
        // provides the resolution of ~ 0.1%.

        // Fixing if all bits in resolution is set = LEDC FULL ON. This is important
		// for low active LEDs since these would not be 100% without this fix.
        const uint32_t full_duty = (duty == kMaxBrightness) ? kMaxBrightness + 1 : duty;

        ledc_set_duty(kLedcSpeedMode, chan_, full_duty);
        ledc_update_duty(kLedcSpeedMode, chan_);
    }

    PinType chan() const { return chan_; }

 private:
    ledc_channel_t chan_;
};

class Esp32Clock {
 public:
    static uint32_t millis() {
        return static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);
    }
};

}  // namespace jled
