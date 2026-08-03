// Copyright (c) 2017-2026 Jan Delgado <jdelgado[at]gmx.net>
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

#include "brightness.h"  // BrightnessTraits

namespace jled {

// Adapts a HAL with a single-argument analogWrite(Color) to the two-argument
// analogWrite(Color, bool invert) contract required by TJLed::WriteRaw(),
// applying inversion in software. Stateless: computes the inverted value
// inline on every call instead of caching the invert flag, so it adds no
// data member of its own on top of Hal. Public inheritance keeps Hal's own
// public members (e.g. Esp32Hal::chan(), HalMock::Value()/Pin()) reachable
// through TJLed::GetHal(); using Hal::Hal reuses Hal's constructors.
template<typename Hal>
class InvertableHal : public Hal {
 public:
    using Hal::Hal;

    template<typename Color>
    void analogWrite(Color val, bool invert) const {
        // Unlike a HAL with native invert (e.g. Esp32Hal), which discards
        // invert here because SetLowActive() already applied it in hardware,
        // this is where invert actually gets applied: val is flipped in
        // software before being forwarded to Hal's plain analogWrite(val).
        Hal::template analogWrite<Color>(invert ? BrightnessTraits<Color>::kFullBrightness - val
                                                : val);
    }

    // Hal has no hardware polarity register; invert is already applied above
    // on every analogWrite(), so there is nothing to pre-arm here.
    void SetLowActive(bool) const {}
};

}  // namespace jled
