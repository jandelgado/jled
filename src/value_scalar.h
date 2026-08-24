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

#include <inttypes.h>

#include "scale_lerp.h"  // lerp<T>, lerp_ordered<T>

namespace jled {

// Shared implementation for the two scalar ValueTraits specializations below
// ApplyBounds/Blend/IsBrighter are identical for uint8_t and
// uint16_t, differing only in kMaxValue()/kBits.
template<typename T>
struct ScalarValueTraitsBase {
    using value_t = T;
    using level_t = T;

    static constexpr value_t kOffColor() { return 0; }
    static constexpr value_t kOnColor() { return ValueTraits<T>::kMaxValue(); }
    static constexpr value_t kMinValue() { return 0; }

    static value_t ApplyBounds(value_t v, level_t lo, level_t hi) {
        return lerp<value_t>(v, lo, hi);
    }
    static value_t Blend(level_t alpha, value_t from, value_t to) {
        return lerp_ordered<value_t>(alpha, from, to);
    }
    static bool IsBrighter(value_t a, value_t b) { return a < b; }
    static value_t Invert(value_t v) { return ValueTraits<T>::kMaxValue() - v; }
};

template<>
struct ValueTraits<uint8_t> : ScalarValueTraitsBase<uint8_t> {
    static constexpr value_t kMaxValue() { return 255; }
    static constexpr uint8_t kBits = 8;
};

template<>
struct ValueTraits<uint16_t> : ScalarValueTraitsBase<uint16_t> {
    static constexpr value_t kMaxValue() { return 65535; }
    static constexpr uint8_t kBits = 16;
};

// Represents a brightness level as a percentage (0-100).
// Implicitly converts to uint8_t or uint16_t so it works transparently
// with both JLed (8-bit) and JLedHD (16-bit) without any code changes:
//
//   JLed   led   = JLed(13)  .MaxBrightness(75_pct).Breathe(500);
//   JLedHD ledHD = JLedHD(13).MaxBrightness(75_pct).Breathe(500);
//
class Percentage {
    uint8_t pct_;

 public:
    constexpr explicit Percentage(uint8_t pct) : pct_(pct) {}

    constexpr operator uint8_t() const {
        return static_cast<uint8_t>(static_cast<uint16_t>(pct_) *
                                    ValueTraits<uint8_t>::kMaxValue() / 100);
    }
    constexpr operator uint16_t() const {
        return static_cast<uint16_t>(static_cast<uint32_t>(pct_) *
                                     ValueTraits<uint16_t>::kMaxValue() / 100);
    }
};

constexpr Percentage operator""_pct(unsigned long long pct) {  // NOLINT(runtime/int)
    return Percentage(static_cast<uint8_t>(pct));
}

}  // namespace jled
