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

#include <inttypes.h>

// HSV<T>: input type for hsv_to_rgb() (value_rgb.h), so a color can be
// specified as hue/saturation/value and converted to the RGBColor<T> that
// TJLedRGB actually stores/animates. Never itself a TJLed<Value>'s Value
// type, so it carries no ValueTraits specialization.
namespace jled {

template<typename T>
struct HSV {
    T h, s, v;  // all in [0, ValueTraits<T>::kMaxValue()]; h spans the full
                // color wheel over 0..kMaxValue(), as in FastLED's CHSV.

    constexpr HSV WithH(T nh) const { return {nh, s, v}; }
    constexpr HSV WithV(T nv) const { return {h, s, nv}; }
};

template<typename T>
constexpr bool operator==(HSV<T> a, HSV<T> b) {
    return a.h == b.h && a.s == b.s && a.v == b.v;
}
template<typename T>
constexpr bool operator!=(HSV<T> a, HSV<T> b) {
    return !(a == b);
}

}  // namespace jled
