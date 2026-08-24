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

#include "value_scalar.h"  // ValueTraits and its scalar specializations
#include "progmem.h"       // JLED_PROGMEM / FlashReader: flash-resident storage on AVR

// Brightness evaluators: stateless, copyable functors that compute a
// brightness value for a given point in time. Driven by TJLed (jled_base.h).

namespace jled {

// Type of effects period (period_t Period()) and time argument to Eval(period_t t).
// Always ins [0, period)
using period_t = uint16_t;

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
T lut_lerp(period_t t, period_t period, const T (&lut)[N]) {
    static_assert(N >= 2 && ((N - 1) & (N - 2)) == 0, "lut_lerp: N-1 must be a power of 2");
    constexpr uint8_t kNormShift = 16 - sizeof(T) * 8;
    constexpr uint8_t kSegShift = (16 - kNormShift) - log2_floor(N - 1);
    if (t + 1 >= period) return FlashReader<T>::Read(&lut[N - 1]);
    // t << (16-kNormShift) can need up to ~24 bits (t is up to 16 bits wide),
    // so re-widen explicitly here for correctness; this is unrelated to (and
    // not shrunk by) t's 16-bit parameter type.
    const uint16_t tnorm = static_cast<uint16_t>((static_cast<uint32_t>(t) << (16 - kNormShift)) /
                                                 static_cast<uint16_t>(period));
    const uint16_t i = tnorm >> kSegShift;
    const auto y0 = FlashReader<T>::Read(&lut[i]);
    const auto y1 = FlashReader<T>::Read(&lut[i + 1]);
    const uint16_t x0 = i << kSegShift;
    const uint16_t dx = tnorm - x0;


    // avoid unnecessary uint32_t promotion:
    // For 8-bit LUTs, dx and (y1-y0) are both <= 255, so their product always
    // fits in 16 bits and can be computed natively. For 16-bit LUTs, dx can
    // be up to ~2^kSegShift and (y1-y0) up to ~2^16, so their product can
    // exceed 65535 and must be widened to 32 bits.
    if (sizeof(T) == 1) {
        return static_cast<T>((dx * (y1 - y0) >> kSegShift) + y0);
    } else {
        return static_cast<T>((static_cast<uint32_t>(dx) * (y1 - y0) >> kSegShift) + y0);
    }
}

// Template helper functions - implemented below after evaluator definitions
template<typename Value>
Value fadeon_func(period_t t, period_t period);

template<typename Value>
Value candle_func(period_t t, uint8_t speed, uint8_t jitter);

// Simple 32-bit integer hash (avalanche mix). Exposed so callers outside
// jled_effects.cpp (e.g. TJLed::Candle()) can derive a well-spread
// pseudo-random value from a small/related seed, such as an instance
// address, without picking up the address's original small deltas.
uint32_t hash32(uint32_t x);

// a function f(t,period,param) that calculates the LEDs brightness for a given
// point in time and the given period. param is an optionally user provided
// parameter. t will always be in range [0..period-1].
// f(period-1,period,param) will be called last to calculate the final state of
// the LED.
template<typename Value>
class BrightnessEvaluator {
 public:
    virtual uint16_t Period() const = 0;
    virtual Value Eval(period_t t) const = 0;
};

template<typename Value>
struct ConstantBrightnessEvaluator {
    Value val_;
    uint16_t duration_;

    uint16_t Period() const { return duration_; }
    Value Eval(period_t) const { return val_; }
};

// BlinkBrightnessEvaluator does n on-off cycles in the specified period
template<typename Value>
struct BlinkBrightnessEvaluator {
    uint16_t duration_on_;
    uint16_t sub_period_;
    uint8_t n_ = 1;
    Value color_on_;
    Value color_off_;

    BlinkBrightnessEvaluator(uint16_t duration_on, uint16_t duration_off, uint8_t n,
                             Value color_on = ValueTraits<Value>::kOnColor(),
                             Value color_off = ValueTraits<Value>::kOffColor())
        : duration_on_(duration_on),
          sub_period_(duration_on + duration_off),
          n_(n),
          color_on_(color_on),
          color_off_(color_off) {}

    uint16_t Period() const { return sub_period_ * n_; }
    Value Eval(period_t t) const {
        // For the common single-cycle case (n_ == 1) t < sub_period_, so the
        // modulo is a no-op and skipped, avoiding the costly division/modulo
        // routine on MCUs without a hardware divider (e.g. AVR).
        const period_t slot_start_t = (n_ == 1) ? t : t % sub_period_;

        return (slot_start_t < duration_on_) ? color_on_ : color_off_;
    }
};

// The breathe func is composed by fade-on, on and fade-off phases. For fading
// we approximate the following function:
//   y(x) = exp(sin((t-period/4.) * 2. * PI / period)) - 0.36787944) *  108.)
// idea see:
//   http://sean.voisen.org/blog/2011/10/breathing-led-with-arduino/
// But we do it with integers only.
template<typename Value>
struct BreatheBrightnessEvaluator {
    using level_t = typename ValueTraits<Value>::level_t;

    uint16_t duration_fade_on_;
    uint16_t duration_on_;
    uint16_t duration_fade_off_;
    Value from_;
    Value to_;

    uint16_t Period() const { return duration_fade_on_ + duration_on_ + duration_fade_off_; }

    Value Eval(period_t t) const {
        const level_t alpha = Alpha(t, duration_fade_on_, duration_on_, duration_fade_off_);
        return ValueTraits<Value>::Blend(alpha, from_, to_);
    }

 private:
    static level_t Alpha(period_t t, uint16_t duration_fade_on, uint16_t duration_on,
                          uint16_t duration_fade_off) {
        // fade-on in the beginning
        if (t < duration_fade_on) return fadeon_func<level_t>(t, duration_fade_on);
        // return maximum brightness in the plateau phase
        if (t < duration_fade_on + duration_on) return ValueTraits<level_t>::kMaxValue();
        // fade out at the end
        const uint16_t period = duration_fade_on + duration_on + duration_fade_off;
        return fadeon_func<level_t>(period - t, duration_fade_off);
    }
};

template<typename Value>
struct CandleBrightnessEvaluator {
    using level_t = typename ValueTraits<Value>::level_t;

    uint8_t speed_;
    uint8_t jitter_;
    uint16_t period_;
    uint16_t offset_;
    Value color_on_;
    Value color_off_;

    CandleBrightnessEvaluator() = delete;

    // speed     - speed of effect (0..15). 0 fastest. Each increment by 1 halves the speed.
    // jitter    - amount of jittering to apply. 0 - no jitter, 15 - candle,
    //                                           64 - fire, 255 - storm
    // period    - period of the effect
    // offset    - time offset in ms added before speed scaling; use different values
    //             per LED for independent flicker.
    // color_on  - color the candle flickers up to at full intensity.
    // color_off - color the candle dims down to at zero intensity.
    CandleBrightnessEvaluator(uint8_t speed, uint8_t jitter, uint16_t period, uint16_t offset = 0,
                              Value color_on = ValueTraits<Value>::kOnColor(),
                              Value color_off = ValueTraits<Value>::kOffColor())
        : speed_(speed),
          jitter_(jitter),
          period_(period),
          offset_(offset),
          color_on_(color_on),
          color_off_(color_off) {}

    uint16_t Period() const { return period_; }

    Value Eval(period_t t) const {
        // offset_ shifts t to give independently configured LEDs distinct
        // flicker phases. The sum wraps at 65536, which only reshuffles the
        // hash input candle_func uses, which is ok.
        return ValueTraits<Value>::Blend(
            candle_func<level_t>(t + offset_, speed_, jitter_), color_off_, color_on_);
    }
};

// Identifies which brightness evaluator is active in EvalStorage.
enum class EvalType : uint8_t { NONE = 0, CONSTANT, BLINK, BREATHE, CANDLE, USER };

// Type-safe discriminated union holding the active brightness evaluator.
// Dispatches Period() and Eval() via switch, no virtual functions for
// built-in effects. The USER arm calls through the user's virtual pointer.
template<typename Value>
struct EvalStorage {
    EvalType type = EvalType::NONE;

    union Data {
        ConstantBrightnessEvaluator<Value> constant;
        BlinkBrightnessEvaluator<Value> blink;
        BreatheBrightnessEvaluator<Value> breathe;
        CandleBrightnessEvaluator<Value> candle;
        BrightnessEvaluator<Value>* user;
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

    Value Eval(period_t t) const {
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
                return ValueTraits<Value>::kOffColor();
        }
    }
};

// ===== Template Helper Function Implementations =====

// Fade-on function: approximates exp(sin(x)) curve for smooth LED fading
// 8-bit specialization uses pre-computed table from jled_effects.cpp
template<>
uint8_t fadeon_func<uint8_t>(period_t t, period_t period);

// 16-bit specialization: interpolate from 8-bit table and scale to 16-bit
// This is implemented in jled_effects.cpp
template<>
uint16_t fadeon_func<uint16_t>(period_t t, period_t period);

};  // namespace jled
