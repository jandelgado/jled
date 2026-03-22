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
#pragma once

#ifdef __MBED__

#include <mbed.h>
#include "jled_std.h"
#include "brightness.h"

namespace jled {

template<uint8_t kResBits_ = 8>
class MbedHal {
 public:
    using PinType = ::PinName;
    using NativeBrightness =
        typename Conditional<(kResBits_ > 8), uint16_t, uint8_t>::type;
    static constexpr uint8_t kNativeBits = kResBits_;
    static constexpr NativeBrightness kMaxBrightness = (1u << kResBits_) - 1;

    explicit MbedHal(PinType pin) noexcept : pin_(pin) {}

    MbedHal(const MbedHal& rhs) : pin_(rhs.pin_) {}

    ~MbedHal() {
        delete pwmout_;
        pwmout_ = nullptr;
    }

    template<typename Brightness>
    void analogWrite(Brightness val) const {
        if (!pwmout_) {
            pwmout_ = new PwmOut(pin_);
        }
        const uint16_t duty = jled::scaleToNative<kNativeBits>(val);
        // Mbed PwmOut::write() takes a float in [0.0, 1.0]
        pwmout_->write(static_cast<float>(duty) / static_cast<float>(kMaxBrightness));
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
