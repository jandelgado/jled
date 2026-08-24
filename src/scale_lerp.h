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

// Generic shared interpolation math: scale, lerp, lerp_ordered

namespace jled {

template<typename T> struct ValueTraits;    // fwd decl.

// Scale a value by a factor. Properties:
//   scale(0, f) == 0 for all f
//   scale(x, max) == x for all x (where max is the maximum value for the type)
// This algorithm avoids division, but is not 100% accurate, but "good enough".
// It is the same algorithmn used in FastLED.
template<typename Level>
Level scale(Level val, Level factor) {
    // Use sizeof to determine type at compile time (optimizes to same code as if constexpr)
    if (sizeof(Level) == 1) {
        return (static_cast<uint16_t>(val) * static_cast<uint16_t>(1 + factor)) >> 8;
    } else {
        return (static_cast<uint32_t>(val) * static_cast<uint32_t>(1 + factor)) >> 16;
    }
}

// Linear interpolation: map val from [0,max] to [a,b]. assumes b >= a:
// examples: lerp<uint8_t>(0, a, b) = a, lerp<uint8_t>(255, a, b) = b
template<typename Level>
Level lerp(Level val, Level a, Level b) {
    constexpr auto kMax = ValueTraits<Level>::kMaxValue();
    // Optimize for most common case: full range
    if (a == 0 && b == kMax) return val;
    const Level delta = b - a;
    return a + scale<Level>(val, delta);
}

// Linear interpolation, but correct regardless of whether b >= a or b < a
// Needed wherever an interpolation's endpoints aren't guaranteed ascending, e.g. Breathe's
// generic from/to builder, which, unlike FadeOn/FadeOff, never reorders
// its arguments.
template<typename Level>
Level lerp_ordered(Level alpha, Level a, Level b) {
    return b >= a ? lerp<Level>(alpha, a, b)
                  : static_cast<Level>(a - scale<Level>(alpha, static_cast<Level>(a - b)));
}

inline uint8_t scale8(uint8_t a, uint8_t b) {
    return scale<uint8_t>(a, b);
}

// Like scale(), but a nonzero val scaled by a nonzero factor never rounds
// down to 0 (FastLED's scale8_video idea). Plain scale() multiplies each
// channel independently, so a small channel (e.g. green=69 in orange-red)
// stays at exactly 0 for the first few steps of a fade while a large
// channel (red=255) is already visible, flashing the wrong hue. Clamping
// the result up to 1 keeps every nonzero channel lit from the first step.
template<typename Level>
Level scale_video(Level val, Level factor) {
    const Level scaled = scale<Level>(val, factor);
    return (val != 0 && factor != 0 && scaled == 0) ? static_cast<Level>(1) : scaled;
}

// scale_video-based counterparts of lerp()/lerp_ordered(), for interpolating
// multi-channel colors where per-channel flooring to 0 causes a hue shift.
template<typename Level>
Level lerp_video(Level val, Level a, Level b) {
    constexpr auto kMax = ValueTraits<Level>::kMaxValue();
    if (a == 0 && b == kMax) return val;
    const Level delta = b - a;
    return a + scale_video<Level>(val, delta);
}

template<typename Level>
Level lerp_ordered_video(Level alpha, Level a, Level b) {
    return b >= a
               ? lerp_video<Level>(alpha, a, b)
               : static_cast<Level>(a - scale_video<Level>(alpha, static_cast<Level>(a - b)));
}

}  // namespace jled
