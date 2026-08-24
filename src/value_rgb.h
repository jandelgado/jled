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

#include "value_hsv.h"      // HSV<T>, needed for the hsv_to_rgb conversion below
#include "value_scalar.h"  // ValueTraits<T> for scalar T, used by ValueTraits<RGBColor<T>> below
#include "scale_lerp.h"     // lerp<T>, lerp_ordered_video<T>

// RGBColor<T>: TJLed's actual Value type for RGB LEDs (see TJLedRGB, jled_rgb.h).
namespace jled {

template<typename T>
struct RGBColor {
    T r, g, b;
};

template<typename T>
constexpr bool operator==(RGBColor<T> a, RGBColor<T> b) {
    return a.r == b.r && a.g == b.g && a.b == b.b;
}
template<typename T>
constexpr bool operator!=(RGBColor<T> a, RGBColor<T> b) {
    return !(a == b);
}

// Converts an HSV color to RGB, e.g. to compute a rainbow-sweep.
RGBColor<uint8_t> hsv_to_rgb(HSV<uint8_t> c);

// 16-bit variant, used by JLedRGBHD. Hue and saturation resolve to 256 steps
// (the 8-bit rainbow mapping runs on their high bytes), value to 65536, which
// is what FadeOn/FadeOff/Breathe animate.
RGBColor<uint16_t> hsv_to_rgb(HSV<uint16_t> c);

template<typename T>
struct ValueTraits<RGBColor<T>> {
    using value_t = RGBColor<T>;
    using level_t = T;

    static constexpr value_t kOffColor() { return {0, 0, 0}; }
    static constexpr value_t kOnColor() {
        return {ValueTraits<T>::kMaxValue(),
                ValueTraits<T>::kMaxValue(),
                ValueTraits<T>::kMaxValue()};  // white
    }
    static constexpr uint8_t kBits = sizeof(T) * 8;

    // Scales every channel by the same factor so the peak channel lands at
    // lerp(peak,lo,hi), preserving the color's hue/ratios exactly - MinBrightness/
    // MaxBrightness touch only overall brightness, never the color itself. A
    // fully-off (black) input has no direction to scale toward, so it floors
    // to gray at lo instead of staying black.
    static value_t ApplyBounds(value_t v, level_t lo, level_t hi) {
        // Optimize for the common case
        if (lo == 0 && hi == ValueTraits<T>::kMaxValue()) return v;
        const T peak = v.r > v.g ? (v.r > v.b ? v.r : v.b) : (v.g > v.b ? v.g : v.b);
        if (peak == 0) return {lo, lo, lo};
        const T newPeak = lerp<T>(peak, lo, hi);
        // Use sizeof to determine type at compile time (mirrors scale() in scale_lerp.h)
        const auto scaleChannel = [peak, newPeak](T c) {
            if (sizeof(T) == 1) {
                return static_cast<T>(
                    (static_cast<uint16_t>(c) * static_cast<uint16_t>(newPeak)) / peak);
            }
            return static_cast<T>(
                (static_cast<uint32_t>(c) * static_cast<uint32_t>(newPeak)) / peak);
        };
        return {scaleChannel(v.r), scaleChannel(v.g), scaleChannel(v.b)};
    }

    // Per-channel interpolation: each endpoint is hit exactly, but a sweep
    // between two differently-hued endpoints dips through a desaturated
    // midpoint (e.g. red -> green via muddy brown) rather than through hue
    // space. Used by CandleBrightnessEvaluator<RGBColor<T>> (color_off/
    // color_on) and, via the primary BreatheBrightnessEvaluator<Value>
    // (jled_effects.h), by Breathe/FadeOn/FadeOff/Fade.
    //
    // Uses lerp_ordered_video (not lerp_ordered) so a channel with a small
    // delta doesn't sit at 0 while a channel with a large delta already
    // moved - see scale_video() in scale_lerp.h.
    static value_t Blend(level_t alpha, value_t from, value_t to) {
        return {lerp_ordered_video<T>(alpha, from.r, to.r),
                lerp_ordered_video<T>(alpha, from.g, to.g),
                lerp_ordered_video<T>(alpha, from.b, to.b)};
    }

    // Cheap proxy, not a perceptual luma: sums the channels. Only used by
    // Fade()'s FadeOn/FadeOff dispatch, which just needs a consistent
    // direction, not a perceptually accurate one.
    static bool IsBrighter(value_t a, value_t b) {
        const uint32_t sa = static_cast<uint32_t>(a.r) + a.g + a.b;
        const uint32_t sb = static_cast<uint32_t>(b.r) + b.g + b.b;
        return sa < sb;
    }

    // Per-channel invert, used by InvertableHal for HALs with no hardware
    // polarity register.
    static value_t Invert(value_t v) {
        constexpr auto kMax = ValueTraits<T>::kMaxValue();
        return {static_cast<T>(kMax - v.r), static_cast<T>(kMax - v.g),
                static_cast<T>(kMax - v.b)};
    }
};

}  // namespace jled
