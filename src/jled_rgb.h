// Copyright (c) 2026 Jan Delgado <jdelgado[at]gmx.net>
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

#include "value_rgb.h"  // NOLINT
#include "jled_base.h"  // NOLINT
#include "rgb_hal.h"    // NOLINT

namespace jled {

// TJLedRGB: base Template for JLedRGB classes for 3-pin RGB Leds.
// TJLedRGB uses the RGBHal<Hal>, which uses 3 individual HALs to address the r,g and b
// pins of a RGB LED.
template<typename Hal, typename Clock, typename Value, typename Derived>
class TJLedRGB : public TJLed<RGBHal<Hal>, Clock, Value, Derived> {
    using Base = TJLed<RGBHal<Hal>, Clock, Value, Derived>;

 public:
    TJLedRGB(typename Hal::PinType r, typename Hal::PinType g, typename Hal::PinType b)
        : Base(RGBHal<Hal>(r, g, b)) {}
};

}  // namespace jled
