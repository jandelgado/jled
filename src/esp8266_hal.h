// Copyright (c) 2017 Jan Delgado <jdelgado[at]gmx.net>
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
#ifndef SRC_ESP8266_HAL_H_
#define SRC_ESP8266_HAL_H_

#include <Arduino.h>
#include "brightness.h"

namespace jled {

class Esp8266Hal {
 public:
    using PinType = uint8_t;
    // ESP8266 uses 10-bit PWM, but we expose as uint16_t since there's no uint10_t
    using NativeBrightnessType = uint16_t;
    static constexpr uint8_t kNativeBits = 10;

    explicit Esp8266Hal(PinType pin) noexcept : pin_(pin) {
        ::pinMode(pin_, OUTPUT);
    }

    template<typename BrightnessType>
    void analogWrite(BrightnessType val) const {
        // ESP8266 uses 10bit PWM range, scale value to 10-bit
        ::analogWrite(pin_, scaleToNative<BrightnessType>(val));
    }

 private:
    // Scale brightness value to native 10-bit resolution
    template<typename BrightnessType>
    static uint16_t scaleToNative(BrightnessType val) {
        // Use sizeof for compile-time optimization (optimizes same as if constexpr)
        if (sizeof(BrightnessType) == 1) {
            // 8-bit to 10-bit: 0 -> 0, 255 -> 1023
            // Preserves min/max relationships
            return (val == 0) ? 0 : (static_cast<uint16_t>(val) << 2) + 3;
        } else {
            // 16-bit to 10-bit: downscale by 6 bits
            return static_cast<uint16_t>(val >> 6);
        }
    }

    PinType pin_;
};

class Esp8266Clock {
 public:
    static uint32_t millis() {
        return ::millis();
    }
};

}  // namespace jled
#endif  // SRC_ESP8266_HAL_H_
