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

#include "value_rgb.h"  // RGBColor

namespace jled {

// RGBHal is the HAL to drive 3-leg RGB LED connected to 3 GPIO pins using 3
// individual HALs. All HAL related calls are delegated to the composed HALs for
// red, green and blue control of the RGB LED.
template<typename Hal>
class RGBHal {
    Hal r_, g_, b_;

 public:
    using PinType = typename Hal::PinType;

    RGBHal(PinType pinr, PinType ping, PinType pinb) : r_{pinr}, g_{ping}, b_{pinb} {}

    template<typename T>
    void analogWrite(RGBColor<T> val, bool invert) const {
        r_.analogWrite(val.r, invert);
        g_.analogWrite(val.g, invert);
        b_.analogWrite(val.b, invert);
    }

    void SetLowActive(bool f) const {
        r_.SetLowActive(f);
        g_.SetLowActive(f);
        b_.SetLowActive(f);
    }

    Hal& r() { return r_; }
    Hal& g() { return g_; }
    Hal& b() { return b_; }
    const Hal& r() const { return r_; }
    const Hal& g() const { return g_; }
    const Hal& b() const { return b_; }
};

}  // namespace jled
