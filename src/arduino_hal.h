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
#pragma once
#include <Arduino.h>
#include "jled_std.h"
#include "brightness.h"

// Some platforms support PWM resolutions higher than 8 bits (e.g. SAMD/Due
// up to 12-bit, RP2040 up to 16-bit). ESP8266 Core v1/v2 used 10-bit natively
// but v3 reverted to 8-bit. analogWriteResolution() is called on first use
// when kResBits_ != 8, via a weak symbol check so it is safe on platforms that
// don't provide it.
extern "C" __attribute__((weak)) void analogWriteResolution(int bits);

namespace jled {;

// ArduinoHal controls a single PWM pin.
//
// kResBits_: native PWM resolution in bits (default 8).
//   Use a higher value (e.g. 10 or 12) on platforms that support it, such as
//   ESP8266 v1/v2 (10-bit) or SAMD/Due (up to 12-bit).  The platform SDK's
//   analogWriteResolution() is called automatically on first use when
//   kResBits_ != 8.  Platform selection is handled in jled.h.
template <uint8_t kResBits_ = 8>
class ArduinoHal {
 public:
    using PinType = uint8_t;
    using NativeBrightness =
        typename Conditional<(kResBits_ > 8), uint16_t, uint8_t>::type;
    static constexpr uint8_t kNativeBits = kResBits_;
    static constexpr NativeBrightness kMaxBrightness = (1u << kResBits_) - 1;

    explicit ArduinoHal(PinType pin) noexcept : pin_(pin) {}

    template <typename Brightness>
    void analogWrite(Brightness val) const {
        // some platforms, e.g. STM need lazy initialization
        if (!setup_) {
//            ::pinMode(pin_, OUTPUT);
            // configure higher PWM resolution once on first use, if supported
            if (kResBits_ != 8 && ::analogWriteResolution != nullptr) {
                ::analogWriteResolution(kResBits_);
            }
            setup_ = true;
        }
        ::analogWrite(pin_, jled::scaleToNative<kResBits_>(val));
    }

 private:
    mutable bool setup_ = false;
    PinType pin_;
};

class ArduinoClock {
 public:
    static uint32_t millis() {
        return ::millis();
    }
};

}  // namespace jled
