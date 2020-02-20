// Copyright (c) 2017-2020 Jan Delgado <jdelgado[at]gmx.net>
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
#ifndef SRC_MBED_HAL_H_
#define SRC_MBED_HAL_H_

#ifdef __MBED__

#include <mbed.h>

namespace jled {

class MbedHal {
 public:
    using PinType = ::PinName;

    explicit MbedHal(PinType pin) noexcept : pin_(pin) {}

    MbedHal(const MbedHal& rhs) { pin_ = rhs.pin_; }

    ~MbedHal() {
        delete pwmout_;
        pwmout_ = nullptr;
    }

    void analogWrite(uint8_t val) const {
        if (!pwmout_) {
            pwmout_ = new PwmOut(pin_);
        }
        pwmout_->write(val / 255.);
    }

    MbedHal& operator=(const MbedHal& rhs) {
        delete pwmout_;
        pwmout_ = nullptr;
        pin_ = rhs.pin_;
        return *this;
    }

    uint32_t millis() const {
        // TODO(JD)
        // us_ticker_read() returns an unsigned 32 bit value with the micro
        // seconds elapsed since booting the mcu. This value wraps over after
        // 4294 seconds, or approx. 71 minutes.
        return us_ticker_read() / 1000;
    }

 private:
    PinType pin_;
    mutable PwmOut* pwmout_ = nullptr;
};
}  // namespace jled
#endif  // __MBED__
#endif  // SRC_MBED_HAL_H_
