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
#include <esp_idf_version.h>
#include <esp_timer.h>
#include <stdint.h>

#include "brightness.h"
#include "jled_std.h"

// The LEDC driver's own output_invert flag (driver/ledc.h) is only available
// starting with ESP-IDF v4.4 (added in
// https://github.com/espressif/esp-idf/commit/48c848a1, absent from the v4.3
// maintenance branch). Older SDKs have no documented hardware invert for
// LEDC: on those, Esp32Hal below doesn't define analogWrite(val, invert) or
// SetLowActive() at all, and jled.h wraps Esp32Hal in InvertableHal instead,
// the same software-invert fallback used for Arduino/ESP8266/mbed.
#define JLED_ESP32_HAS_LEDC_OUTPUT_INVERT \
    (ESP_IDF_VERSION_MAJOR > 4 || (ESP_IDF_VERSION_MAJOR == 4 && ESP_IDF_VERSION_MINOR >= 4))

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

    PinType pinForChan(ledc_channel_t chan) const { return chanMap_[chan]; }
    void registerPin(ledc_channel_t chan, PinType pin) { chanMap_[chan] = pin; }

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

    static constexpr auto kAutoSelectChan = -1;

    // Construct an ESP32 HAL object for the given GPIO pin.
    // pin   GPIO pin to connect to.
    // chan  LEDC channel to use. kAutoSelectChan (default) picks the next free
    //       channel automatically; pass an explicit value to override.
    // freq  LEDC base frequency in Hz (default: 5000).
    // The PWM resolution (kResBits_) and timer (kTimer_) are template parameters.
    explicit Esp32Hal(PinType pin, int chan = kAutoSelectChan, uint16_t freq = 5000) noexcept {
        if (chan == kAutoSelectChan) {
            // chanForPin() already registers the pin for the channel it returns.
            chan_ = Esp32Hal::chanMapper().chanForPin(pin);
        } else {
            chan_ = (ledc_channel_t)chan;
            Esp32Hal::chanMapper().registerPin(chan_, pin);
        }

        ledc_timer_config_t ledc_timer{};
        ledc_timer.speed_mode = kLedcSpeedMode;
        ledc_timer.duty_resolution = static_cast<ledc_timer_bit_t>(kResBits_);
        ledc_timer.timer_num = kTimer_;
        ledc_timer.freq_hz = freq;
#if ESP_IDF_VERSION_MAJOR > 3
        ledc_timer.clk_cfg = LEDC_AUTO_CLK;
#endif
        ledc_timer_config(&ledc_timer);

        auto ledc_channel = makeChannelConfig(pin, 0);
        ledc_channel_config(&ledc_channel);
    }

    template<typename Brightness>
    void analogWrite(Brightness val) const {
        // Scale brightness to actual resolution
        const uint16_t duty = jled::scaleToNative<kResBits_>(val);

        // from:
        // https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/ledc.html
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

#if JLED_ESP32_HAS_LEDC_OUTPUT_INVERT
    template<typename Brightness>
    void analogWrite(Brightness val, bool /*invert*/) const {
        // Inversion is fully owned by SetLowActive()'s output_invert flag
        // below; this HAL inverts in hardware, so it never needs to inspect
        // invert on a per-call basis the way a software-fallback HAL would.
        analogWrite(val);
    }

    // Inverts the channel's output in hardware via the LEDC driver's own
    // output_invert flag. ledc_channel_config() always writes a full
    // channel config, so this reads back the current duty/hpoint first;
    // otherwise the call would reset an in-flight duty to 0 instead of
    // just flipping polarity. If the native LEDC flag misbehaves at some
    // duty value on your chip, wrap this HAL in InvertableHal
    // (src/invertable_hal.h) for a software fallback instead.
    void SetLowActive(bool f) const {
        const auto pin = chanMapper().pinForChan(chan_);
        auto ledc_channel = makeChannelConfig(pin, ledc_get_duty(kLedcSpeedMode, chan_));
        ledc_channel.hpoint = ledc_get_hpoint(kLedcSpeedMode, chan_);
        ledc_channel.flags.output_invert = f;
        ledc_channel_config(&ledc_channel);
    }
#endif  // JLED_ESP32_HAS_LEDC_OUTPUT_INVERT

    PinType chan() const { return chan_; }

 private:
    static constexpr uint16_t kMaxBrightness = (1u << kResBits_) - 1;

    // Builds the shared part of a ledc_channel_config_t. output_invert is
    // deliberately left untouched here: that struct field only exists from
    // ESP-IDF v4.4 on (see JLED_ESP32_HAS_LEDC_OUTPUT_INVERT), while this
    // helper is also called unconditionally from the constructor.
    ledc_channel_config_t makeChannelConfig(PinType pin, uint32_t duty) const {
        ledc_channel_config_t ledc_channel{};
        ledc_channel.gpio_num = pin;
        ledc_channel.speed_mode = kLedcSpeedMode;
        ledc_channel.channel = chan_;
        ledc_channel.intr_type = LEDC_INTR_DISABLE;
        ledc_channel.timer_sel = kTimer_;
        ledc_channel.duty = duty;
        ledc_channel.hpoint = 0;
        return ledc_channel;
    }

    ledc_channel_t chan_;
};

class Esp32Clock {
 public:
    static uint32_t millis() { return static_cast<uint32_t>(esp_timer_get_time() / 1000ULL); }
};

}  // namespace jled
