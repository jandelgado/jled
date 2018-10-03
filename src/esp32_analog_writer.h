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
#ifndef SRC_ESP32_ANALOG_WRITER_H_
#define SRC_ESP32_ANALOG_WRITER_H_

#include <Arduino.h>

class Esp32AnalogWriter /*: public AnalogWriter */ {
    static constexpr auto kLedcTimer8Bit = 8;

 public:
    static constexpr auto kLedcMaxChan = 16;
    static constexpr auto kAutoSelectChan = -1;

    // construct an ESP32 analog write object connected to the given pin.
    // chan specifies the EPS32 ledc channel to use. If set to kAutoSelectChan,
    // the next available channel will be used, otherwise the specified one.
    // freq defines the ledc base frequency to be used (default: 5000 Hz).
    explicit Esp32AnalogWriter(uint8_t pin, int chan = kAutoSelectChan,
                               uint16_t freq = 5000) noexcept {
        // ESP32 framework lacks analogWrite() support, but behaviour can
        // be achievedd using LEDC channels.
        // https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/peripherals/ledc.html
        chan_ = (chan == kAutoSelectChan)
                    ? (Esp32AnalogWriter::nextChan_++) % kLedcMaxChan
                    : chan;
        ledcSetup(chan_, freq, kLedcTimer8Bit);
        ledcAttachPin(pin, chan_);
    }
    void analogWrite(uint8_t val) { ledcWrite(chan_, val); }
    uint8_t chan() const { return chan_; }

 private:
    static uint8_t nextChan_;
    uint8_t chan_;
};

#endif  // SRC_ESP32_ANALOG_WRITER_H_
