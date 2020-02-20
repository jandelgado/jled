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

namespace jled {

class Esp8266Hal {
 public:
    using PinType = uint8_t;

    explicit Esp8266Hal(PinType pin) noexcept : pin_(pin) {
        ::pinMode(pin_, OUTPUT);
    }
    void analogWrite(uint8_t val) const {
        // ESP8266 uses 10bit PWM range per default, scale value up
        ::analogWrite(pin_, Esp8266Hal::ScaleTo10Bit(val));
    }
    uint32_t millis() const { return ::millis(); }

 protected:
    // scale an 8bit value to 10bit: 0 -> 0, ..., 255 -> 1023,
    // preserving min/max relationships in both ranges.
    static uint16_t ScaleTo10Bit(uint8_t x) {
        return (x == 0) ? 0 : (x << 2) + 3;
    }

 private:
    PinType pin_;
};
}  // namespace jled
#endif  // SRC_ESP8266_HAL_H_
