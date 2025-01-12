// Copyright (c) 2017 Jan Delgado <jdelgado[at]gmx.net>
// https://github.com/jandelgado/jled
//
// Standard HAL using Arduinos Core API analogWrite, analogWriteResolution
// and millis functions.
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

// some MCU like e.g. ESP8266, zero and mkr models support PWM resolutions
// higher then 8 bits, but the analogWriteResolution() function is only defined
// for these platforms. By using `__attribute((weak))`  we can check for the
// existence at runtime and adjust the resolution accordingly.
extern "C" __attribute__((weak)) void analogWriteResolution(int bits);

namespace jled {

class ArduinoHal {
 public:
    using PinType = uint8_t;

    explicit ArduinoHal(PinType pin) noexcept : pin_(pin) {}

    void analogWrite(uint16_t val) const {
        // some platforms, e.g. STM need lazy initialization
        if (!setup_) {
            ::pinMode(pin_, OUTPUT);
            setup_ = true;
        }
        val >>= shiftResBits();
        ::analogWrite(pin_, val);
    }

    static bool WriteResolution(uint8_t res_bits) {
        if (::analogWriteResolution != 0) {
            ::analogWriteResolution(res_bits);
            shiftResBits() = (16 - res_bits);
            return true;
        }
        return false;
    }

 private:
    // returns number of bits to shift to the right to scale a 16 bit value
    // to output to the given configured PWM resoltion.
    // Some arduinos support up to 12 bits (Due), 10 bits (Zero) in
    // addition to the default of 8 bits.
    static inline uint8_t &shiftResBits() {
#if defined(ESP8266)
#if defined(HAS_ESP8266_VERSION_NUMERIC)
        // in version 2 and earlier, the ESP8266 core analogWrite() operated
        // with 10 bits by default. this changed in version 3 for compatibility
        // reasons to 8 bits.
        static uint8_t shift_right_ = (esp8266::coreVersionMajor() < 3) ? 6 : 8;
#else
        static uint8_t shift_right_ = 6;
#endif
#else  // not ESP8266
        // default of Arduino framework is 8 bit resolution
        static uint8_t shift_right_ = 8;
#endif  // ESP8266
        return shift_right_;
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
