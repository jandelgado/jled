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
#ifndef SRC_ARDUINO_HAL_H_
#define SRC_ARDUINO_HAL_H_

#include <Arduino.h>
#include "brightness.h"

namespace jled {

class ArduinoHal {
 public:
    using PinType = uint8_t;
    using NativeBrightnessType = uint8_t;
    static constexpr uint8_t kNativeBits = 8;

    explicit ArduinoHal(PinType pin) noexcept : pin_(pin) {}

    // Template analogWrite to accept any brightness type
    template<typename BrightnessType>
    void analogWrite(BrightnessType val) const {
        // some platforms, e.g. STM need lazy initialization
        if (!setup_) {
            ::pinMode(pin_, OUTPUT);
            setup_ = true;
        }
        // Scale brightness value to native HAL resolution (8-bit)
        ::analogWrite(pin_, scaleToNative<BrightnessType>(val));
    }

 private:
    // Scale brightness value to native 8-bit resolution
    template<typename BrightnessType>
    static uint8_t scaleToNative(BrightnessType val) {
        // Use sizeof for compile-time optimization (optimizes same as if constexpr)
        if (sizeof(BrightnessType) == 1) {
            // 8-bit to 8-bit: no scaling needed (zero-cost abstraction)
            return val;
        } else {
            // 16-bit to 8-bit: downscale
            return static_cast<uint8_t>(val >> 8);
        }
    }

    mutable bool setup_ = false;
    PinType pin_;
};

class ArduinoClock {
 public:
    static uint32_t millis() { return ::millis(); }
};

}  // namespace jled
#endif  // SRC_ARDUINO_HAL_H_
