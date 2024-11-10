// Copyright (c) 2017-2022 Jan Delgado <jdelgado[at]gmx.net>
// https://github.com/jandelgado/jled
//
// HAL for the ESP32 compatible with Arduino and ESP-IDF framework. Uses
// ESP-IDF SDK under the hood.
//
// Documentation:
// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/ledc.html
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
#ifndef SRC_ESP32_HAL_H_
#define SRC_ESP32_HAL_H_

#include <driver/ledc.h>
#include <esp_timer.h>
#include <stdint.h>

namespace jled {

class Esp32ChanMapper {
    static constexpr auto kFreeChan = 0xff;

 public:
    using PinType = uint8_t;

    static constexpr auto kLedcMaxChan = 16;

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

class Esp32Hal {
    static constexpr auto kLedcTimerResolution = LEDC_TIMER_8_BIT;
    static constexpr auto kLedcSpeedMode = LEDC_LOW_SPEED_MODE;

 public:
    using PinType = Esp32ChanMapper::PinType;

    static constexpr auto kAutoSelectChan = -1;

    // construct an ESP32 analog write object connected
    // pin    gpio pin to connect to
    // chan   specifies the EPS32 ledc channel to use. If set to
    // kAutoSelectChan,
    //        the next available channel will be used, otherwise the specified
    //        one.
    // freq   defines the ledc base frequency to be used (default: 5000 Hz).
    // timer is the ledc timer to use (default: LEDC_TIMER_0). When different
    //       frequencies are used, also different timers must be used.
    Esp32Hal(PinType pin, int chan = kAutoSelectChan, uint16_t freq = 5000,
             ledc_timer_t timer = LEDC_TIMER_0) noexcept {
        chan_ = (chan == kAutoSelectChan)
                    ? Esp32Hal::chanMapper_.chanForPin(pin)
                    : (ledc_channel_t)chan;

        ledc_timer_config_t ledc_timer{};
        ledc_timer.speed_mode = kLedcSpeedMode;
        ledc_timer.duty_resolution = kLedcTimerResolution;
        ledc_timer.timer_num = timer;
        ledc_timer.freq_hz = freq;
#if ESP_IDF_VERSION_MAJOR > 3
        ledc_timer.clk_cfg = LEDC_AUTO_CLK;
#endif
        ledc_timer_config(&ledc_timer);

        ledc_channel_t channel = (ledc_channel_t)(chan_ % LEDC_CHANNEL_MAX);
        ledc_channel_config_t ledc_channel{};
        ledc_channel.gpio_num = pin;
        ledc_channel.speed_mode = kLedcSpeedMode;
        ledc_channel.channel = channel;
        ledc_channel.intr_type = LEDC_INTR_DISABLE;
        ledc_channel.timer_sel = timer;
        ledc_channel.duty = 0;
        ledc_channel.hpoint = 0;
#if ESP_IDF_VERSION_MAJOR > 4
        ledc_channel.flags.output_invert = 0;
#endif
        ledc_channel_config(&ledc_channel);
    }

    void analogWrite(uint8_t duty) const {
        // Fixing if all bits in resolution is set = LEDC FULL ON
        const uint32_t _duty = (duty == (1 << kLedcTimerResolution) - 1)
                             ? 1 << kLedcTimerResolution
                             : duty;

        ledc_set_duty(kLedcSpeedMode, chan_, _duty);
        ledc_update_duty(kLedcSpeedMode, chan_);
    }

    uint32_t millis() const {
        return static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);
    }

    PinType chan() const { return chan_; }

 private:
    static Esp32ChanMapper chanMapper_;
    ledc_channel_t chan_;
};
}  // namespace jled
#endif  // SRC_ESP32_HAL_H_
