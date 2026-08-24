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

#include "scale_bit_depth.h"

// Some platforms support PWM resolutions higher than 8 bits (e.g. SAMD/Due
// up to 12-bit, RP2040 up to 16-bit). ESP8266 Core v1/v2 used 10-bit natively
// but v3 reverted to 8-bit. analogWriteResolution() is called once, when
// kResBits_ != 8, via a weak symbol check so it is safe on platforms that
// don't provide it.
extern "C" __attribute__((weak)) void analogWriteResolution(int bits);

namespace jled {

namespace detail {
// Holds the "already set up" flag needed by ArduinoHal's lazy init path.
// Empty for kLazyInit_ == false, so it costs no storage (empty base
// optimization) when ArduinoHal is used as a private base with it.
template<bool kLazyInit_>
struct ArduinoHalInitState {};
template<>
struct ArduinoHalInitState<true> {
    mutable bool setup_ = false;
};
}  // namespace detail

// ArduinoHal controls a single PWM pin.
//
// kResBits_: native PWM resolution in bits (default 8).
//   Use a higher value (e.g. 10 or 12) on platforms that support it, such as
//   ESP8266 v1/v2 (10-bit) or SAMD/Due (up to 12-bit).  The platform SDK's
//   analogWriteResolution() is called automatically when kResBits_ != 8.
//   Platform selection is handled in jled.h.
// kLazyInit_: defer pinMode()/analogWriteResolution() from the constructor to
//   the first analogWrite() call (default false, i.e. init in the
//   constructor, at zero extra storage/runtime cost). STM32duino requires
//   this to be true, since it forbids touching hardware from a global
//   constructor - JLed objects are typically declared globally, and their
//   HAL's constructor runs as part of C++ global static initialization,
//   before the STM32 core's clock/GPIO init has run. See
//   https://wiki.stm32duino.com/index.php?title=API#Important_information_about_global_constructor_methods
template<uint8_t kResBits_ = 8, bool kLazyInit_ = false>
class ArduinoHal : private detail::ArduinoHalInitState<kLazyInit_> {
 public:
    using PinType = uint8_t;

    explicit ArduinoHal(PinType pin) noexcept : pin_(pin) { InitNow(Tag<kLazyInit_>{}); }

    template<typename Level>
    void analogWrite(Level val) const {
        EnsureSetup(Tag<kLazyInit_>{});
        ::analogWrite(pin_, jled::scale_bit_depth<kResBits_>(val));
    }

 private:
    template<bool>
    struct Tag {};

    void Setup() const {
        ::pinMode(pin_, OUTPUT);
        if (kResBits_ != 8 && ::analogWriteResolution != nullptr) {
            ::analogWriteResolution(kResBits_);
        }
    }

    void InitNow(Tag<false>) { Setup(); }
    void InitNow(Tag<true>) {}

    void EnsureSetup(Tag<false>) const {}
    void EnsureSetup(Tag<true>) const {
        if (!this->setup_) {
            Setup();
            this->setup_ = true;
        }
    }

    PinType pin_;
};

class ArduinoClock {
 public:
    static uint32_t millis() { return ::millis(); }
};

}  // namespace jled
