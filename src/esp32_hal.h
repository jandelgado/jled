// Copyright (c) 2017-2020 Jan Delgado <jdelgado[at]gmx.net>
// https://github.com/jandelgado/jled
// HAL for the ESP32
// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/ledc.html
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

#include <Arduino.h>

namespace jled {

class Esp32ChanMapper {
    static constexpr auto kFreeChan = 0xff;

 public:
    using PinType = uint8_t;

    static constexpr auto kLedcMaxChan = 16;

    Esp32ChanMapper() {
        for (auto i = 0; i < kLedcMaxChan; i++) chanMap_[i] = 0xff;
    }
    PinType chanForPin(PinType pin) {
        // find existing channel for given pin
        for (auto i = 0; i < kLedcMaxChan; i++) {
            if (chanMap_[i] == pin) return i;
        }
        // find and return first free slot
        for (auto i = 0; i < kLedcMaxChan; i++) {
            if (chanMap_[i] == kFreeChan) {
                chanMap_[i] = pin;
                return i;
            }
        }
        // no more free slots, start over
        auto i = nextChan_;
        chanMap_[i] = pin;
        nextChan_ = (nextChan_ + 1) % kLedcMaxChan;
        return i;
    }

 private:
    PinType nextChan_ = 0;
    PinType chanMap_[kLedcMaxChan];
};

class Esp32Hal {
    static constexpr auto kLedcTimer8Bit = 8;

 public:
    using PinType = Esp32ChanMapper::PinType;

    static constexpr auto kAutoSelectChan = -1;

    // construct an ESP32 analog write object connected to the given pin.
    // chan specifies the EPS32 ledc channel to use. If set to kAutoSelectChan,
    // the next available channel will be used, otherwise the specified one.
    // freq defines the ledc base frequency to be used (default: 5000 Hz).
    Esp32Hal(PinType pin, int chan = kAutoSelectChan,
             uint16_t freq = 5000) noexcept {
        // ESP32 framework lacks analogWrite() support, but behaviour can
        // be achievedd using LEDC channels.
        // https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/peripherals/ledc.html
        chan_ = (chan == kAutoSelectChan)
                    ? Esp32Hal::chanMapper_.chanForPin(pin)
                    : chan;
        ::ledcSetup(chan_, freq, kLedcTimer8Bit);
        ::ledcAttachPin(pin, chan_);
    }
    void analogWrite(uint8_t val) const {
        ::ledcWrite(chan_, (val == 255)? 256 : val);
    }
    uint32_t millis() const { return ::millis(); }

    PinType chan() const { return chan_; }

 private:
    static Esp32ChanMapper chanMapper_;
    PinType chan_;
};
}  // namespace jled
#endif  // SRC_ESP32_HAL_H_
