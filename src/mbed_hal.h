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
#include "brightness.h"

namespace jled {

class MbedHal {
 public:
    using PinType = ::PinName;
    using NativeBrightness = uint8_t;  // Mbed uses float internally, but we expose 8-bit API
    static constexpr uint8_t kNativeBits = 8;

    explicit MbedHal(PinType pin) noexcept : pin_(pin) {}

    MbedHal(const MbedHal& rhs) { pin_ = rhs.pin_; }

    ~MbedHal() {
        delete pwmout_;
        pwmout_ = nullptr;
    }

    template<typename Brightness>
    void analogWrite(Brightness val) const {
        if (!pwmout_) {
            pwmout_ = new PwmOut(pin_);
        }
        // Mbed uses float in range [0.0, 1.0]
        constexpr auto kMax = BrightnessTraits<Brightness>::kFullBrightness;
        pwmout_->write(static_cast<float>(val) / static_cast<float>(kMax));
    }

    MbedHal& operator=(const MbedHal& rhs) {
        delete pwmout_;
        pwmout_ = nullptr;
        pin_ = rhs.pin_;
        return *this;
    }

 private:
    PinType pin_;
    mutable PwmOut* pwmout_ = nullptr;
};

class MbedClock {
 public:
    static uint32_t millis() {
        return Kernel::Clock::now().time_since_epoch().count();
    }
};

}  // namespace jled
#endif  // __MBED__
#endif  // SRC_MBED_HAL_H_
