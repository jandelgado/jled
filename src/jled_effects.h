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

#include <inttypes.h>  // types, e.g. uint8_t
#include <stddef.h>    // size_t

#include "brightness.h"  // brightness type traits and utilities

// Brightness evaluators: stateless, copyable functors that compute a
// brightness value for a given point in time. Driven by TJLed (jled_base.h).

namespace jled {

// Legacy 8-bit constants for backwards compatibility
static constexpr uint8_t kFullBrightness = 255;
static constexpr uint8_t kZeroBrightness = 0;

// Compile-time log2 (C++14 compatible single-expression constexpr)
constexpr uint8_t log2_floor(size_t n) {
    return n <= 1 ? 0 : 1 + log2_floor(n >> 1);
}

// Generic LUT-based linear interpolation.
//
// Maps t in [0, period) to [lut[0], lut[N-1]] using a pre-computed lookup
// table and piecewise linear interpolation between adjacent entries.
// T and N are deduced from the lut array argument.
//
// kNormShift is derived from sizeof(T): 8 bits for uint8_t (range [0,256)),
// 0 for uint16_t (range [0,65536)). kSegShift encodes the width of each LUT
// segment as a power-of-two, computed from N and kNormShift at compile time.
//
// Requires: N >= 2 and (N-1) is a power of two (checked by static_assert).
template<typename T, size_t N>
T lut_lerp(uint32_t t, uint16_t period, const T (&lut)[N]) {
    static_assert(N >= 2 && ((N - 1) & (N - 2)) == 0, "lut_lerp: N-1 must be a power of 2");
    constexpr uint8_t kNormShift = 16 - sizeof(T) * 8;
    constexpr uint8_t kSegShift = (16 - kNormShift) - log2_floor(N - 1);
    if (t + 1 >= period) return lut[N - 1];
    const uint16_t tnorm =
        static_cast<uint16_t>((t << (16 - kNormShift)) / static_cast<uint16_t>(period));
    const uint16_t i = tnorm >> kSegShift;
    const auto y0 = lut[i];
    const auto y1 = lut[i + 1];
    const uint16_t x0 = i << kSegShift;
    const uint16_t dx = tnorm - x0;
    // For 8-bit LUTs, dx and (y1-y0) are both <= 255, so their product always
    // fits in 16 bits and can be computed natively. For 16-bit LUTs, dx can
    // be up to ~2^kSegShift and (y1-y0) up to ~2^16, so their product can
    // exceed 65535 and must be widened to 32 bits. On platforms where int is
    // 32 bits the 8-bit case happens to work out either way via integer
    // promotion, but on AVR (e.g. ATmega328P) int/unsigned int is only 16
    // bits, so the unconditional 32-bit widening would incur an expensive
    // 32-bit multiply/shift even when not needed.
    if (sizeof(T) == 1) {
        return static_cast<T>((dx * (y1 - y0) >> kSegShift) + y0);
    } else {
        return static_cast<T>((static_cast<uint32_t>(dx) * (y1 - y0) >> kSegShift) + y0);
    }
}

// Template helper functions - implemented below after evaluator definitions
template<typename Brightness>
Brightness fadeon_func(uint32_t t, uint16_t period);

template<typename Brightness>
Brightness candle_func(uint32_t t, uint8_t speed, uint8_t jitter);

template<typename Brightness>
Brightness scale(Brightness val, Brightness factor);

template<typename Brightness>
Brightness lerp(Brightness val, Brightness a, Brightness b);

// Legacy 8-bit function names (inline wrappers for backwards compatibility)
inline uint8_t scale8(uint8_t val, uint8_t f) {
    return scale<uint8_t>(val, f);
}
inline uint8_t lerp8by8(uint8_t val, uint8_t a, uint8_t b) {
    return lerp<uint8_t>(val, a, b);
}

// a function f(t,period,param) that calculates the LEDs brightness for a given
// point in time and the given period. param is an optionally user provided
// parameter. t will always be in range [0..period-1].
// f(period-1,period,param) will be called last to calculate the final state of
// the LED.
template<typename Brightness>
class BrightnessEvaluator {
 public:
    virtual uint16_t Period() const = 0;
    virtual Brightness Eval(uint32_t t) const = 0;
};

template<typename Brightness>
struct ConstantBrightnessEvaluator {
    Brightness val_;
    uint16_t duration_;

    uint16_t Period() const { return duration_; }
    Brightness Eval(uint32_t) const { return val_; }
};

// BlinkBrightnessEvaluator does n on-off cycles in the specified period
template<typename Brightness>
struct BlinkBrightnessEvaluator {
    uint16_t duration_on_;
    uint16_t sub_period_;
    uint8_t n_ = 1;

    BlinkBrightnessEvaluator(uint16_t duration_on, uint16_t duration_off, uint8_t n)
        : duration_on_(duration_on), sub_period_(duration_on + duration_off), n_(n) {}

    uint16_t Period() const { return sub_period_ * n_; }
    Brightness Eval(uint32_t t) const {
        // Eval is only ever called with t < Period(), and the contract requires
        // Period() to fit in uint16_t, so t fits in uint16_t too. For the common
        // single-cycle case (n_ == 1) t < sub_period_, so the modulo is a no-op
        // and skipped. Narrowing to 16-bit avoids the costly 32-bit
        // division/modulo routine on MCUs without a hardware divider (e.g. AVR).
        const uint16_t slot_start_t =
            (n_ == 1) ? static_cast<uint16_t>(t) : static_cast<uint16_t>(t) % sub_period_;

        return (slot_start_t < duration_on_) ? BrightnessTraits<Brightness>::kFullBrightness
                                             : BrightnessTraits<Brightness>::kZeroBrightness;
    }
};

// The breathe func is composed by fade-on, on and fade-off phases. For fading
// we approximate the following function:
//   y(x) = exp(sin((t-period/4.) * 2. * PI / period)) - 0.36787944) *  108.)
// idea see:
//   http://sean.voisen.org/blog/2011/10/breathing-led-with-arduino/
// But we do it with integers only.
template<typename Brightness>
struct BreatheBrightnessEvaluator {
    uint16_t duration_fade_on_;
    uint16_t duration_on_;
    uint16_t duration_fade_off_;
    Brightness from_;
    Brightness to_;
    uint16_t Period() const { return duration_fade_on_ + duration_on_ + duration_fade_off_; }
    Brightness Eval(uint32_t t) const {
        Brightness val = BrightnessTraits<Brightness>::kZeroBrightness;
        if (t < duration_fade_on_)
            val = fadeon_func<Brightness>(t, duration_fade_on_);
        else if (t < duration_fade_on_ + duration_on_)
            val = BrightnessTraits<Brightness>::kFullBrightness;
        else
            val = fadeon_func<Brightness>(Period() - t, duration_fade_off_);
        return lerp<Brightness>(val, from_, to_);
    }
};

template<typename Brightness>
struct CandleBrightnessEvaluator {
    uint8_t speed_;
    uint8_t jitter_;
    uint16_t period_;
    uint16_t offset_;

    CandleBrightnessEvaluator() = delete;

    // speed  - speed of effect (0..15). 0 fastest. Each increment by 1 halves the speed.
    // jitter - amount of jittering to apply. 0 - no jitter, 15 - candle,
    //                                        64 - fire, 255 - storm
    // period - period of the effect
    // offset - time offset in ms added before speed scaling; use different values
    //          per LED for independent flicker.
    CandleBrightnessEvaluator(uint8_t speed, uint8_t jitter, uint16_t period, uint16_t offset = 0)
        : speed_(speed), jitter_(jitter), period_(period), offset_(offset) {}

    uint16_t Period() const { return period_; }

    Brightness Eval(uint32_t t) const {
        return candle_func<Brightness>(t + offset_, speed_, jitter_);
    }
};

// Identifies which brightness evaluator is active in EvalStorage.
enum class EvalType : uint8_t { NONE = 0, CONSTANT, BLINK, BREATHE, CANDLE, USER };

// Type-safe discriminated union holding the active brightness evaluator.
// Dispatches Period() and Eval() via switch, no virtual functions for
// built-in effects. The USER arm calls through the user's virtual pointer.
template<typename Brightness>
struct EvalStorage {
    EvalType type = EvalType::NONE;

    union Data {
        ConstantBrightnessEvaluator<Brightness> constant;
        BlinkBrightnessEvaluator<Brightness> blink;
        BreatheBrightnessEvaluator<Brightness> breathe;
        CandleBrightnessEvaluator<Brightness> candle;
        BrightnessEvaluator<Brightness>* user;
        Data() {}
        ~Data() {}
    } data;

    bool IsSet() const { return type != EvalType::NONE; }

    uint16_t Period() const {
        switch (type) {
            case EvalType::CONSTANT:
                return data.constant.Period();
            case EvalType::BLINK:
                return data.blink.Period();
            case EvalType::BREATHE:
                return data.breathe.Period();
            case EvalType::CANDLE:
                return data.candle.Period();
            case EvalType::USER:
                return data.user->Period();
            default:
                return 0;
        }
    }

    Brightness Eval(uint32_t t) const {
        switch (type) {
            case EvalType::CONSTANT:
                return data.constant.Eval(t);
            case EvalType::BLINK:
                return data.blink.Eval(t);
            case EvalType::BREATHE:
                return data.breathe.Eval(t);
            case EvalType::CANDLE:
                return data.candle.Eval(t);
            case EvalType::USER:
                return data.user->Eval(t);
            default:
                return BrightnessTraits<Brightness>::kZeroBrightness;
        }
    }
};

// ===== Template Helper Function Implementations =====

// Scale a value by a factor. Properties:
//   scale(0, f) == 0 for all f
//   scale(x, max) == x for all x (where max is the maximum value for the type)
// This algorithm avoids division, but is not 100% accurate, but "good enough".
// It is the same algorithmn used in FastLED.
template<typename Brightness>
Brightness scale(Brightness val, Brightness factor) {
    // Use sizeof to determine type at compile time (optimizes to same code as if constexpr)
    if (sizeof(Brightness) == 1) {
        return (static_cast<uint16_t>(val) * static_cast<uint16_t>(1 + factor)) >> 8;
    } else {
        return (static_cast<uint32_t>(val) * static_cast<uint32_t>(1 + factor)) >> 16;
    }
}

// Linear interpolation: map val from [0,max] to [a,b]
template<typename Brightness>
Brightness lerp(Brightness val, Brightness a, Brightness b) {
    constexpr auto kMax = BrightnessTraits<Brightness>::kFullBrightness;
    // Optimize for most common case: full range
    if (a == 0 && b == kMax) return val;
    const Brightness delta = b - a;
    return a + scale<Brightness>(val, delta);
}

// Fade-on function: approximates exp(sin(x)) curve for smooth LED fading
// 8-bit specialization uses pre-computed table from jled_effects.cpp
template<>
uint8_t fadeon_func<uint8_t>(uint32_t t, uint16_t period);

// 16-bit specialization: interpolate from 8-bit table and scale to 16-bit
// This is implemented in jled_effects.cpp
template<>
uint16_t fadeon_func<uint16_t>(uint32_t t, uint16_t period);

};  // namespace jled
