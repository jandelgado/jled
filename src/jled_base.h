// Copyright (c) 2017-2021 Jan Delgado <jdelgado[at]gmx.net>
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

// JLed - non-blocking LED abstraction library.
//
// Example Arduino sketch:
//   auto led = JLed(LED_BUILTIN).Blink(500, 500).Repeat(10).DelayBefore(1000);
//
//   void setup() {}
//
//   void loop() {
//     led.Update();
//   }

namespace jled {

// Legacy 8-bit constants for backwards compatibility
static constexpr uint8_t kFullBrightness = 255;
static constexpr uint8_t kZeroBrightness = 0;

// Random number generation (shared across all brightness types)
uint8_t rand8();
void rand_seed(uint32_t s);

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
template <typename T, size_t N>
T lut_lerp(uint32_t t, uint16_t period, const T (&lut)[N]) {
    static_assert(N >= 2 && ((N - 1) & (N - 2)) == 0,
                  "lut_lerp: N-1 must be a power of 2");
    constexpr uint8_t kNormShift = 16 - sizeof(T) * 8;
    constexpr uint8_t kSegShift = (16 - kNormShift) - log2_floor(N - 1);
    if (t + 1 >= period) return lut[N - 1];
    const uint16_t tnorm = static_cast<uint16_t>(
        (t << (16 - kNormShift)) / static_cast<uint16_t>(period));
    const uint16_t i = tnorm >> kSegShift;
    const auto y0 = lut[i];
    const auto y1 = lut[i + 1];
    const uint16_t x0 = i << kSegShift;
    return static_cast<T>((((tnorm - x0) * (y1 - y0)) >> kSegShift) + y0);
}

// Template helper functions - implemented below after evaluator definitions
template<typename Brightness>
Brightness fadeon_func(uint32_t t, uint16_t period);

template<typename Brightness>
Brightness scale(Brightness val, Brightness factor);

template<typename Brightness>
Brightness lerp(Brightness val, Brightness a, Brightness b);

// Legacy 8-bit function names (inline wrappers for backwards compatibility)
inline uint8_t scale8(uint8_t val, uint8_t f) { return scale<uint8_t>(val, f); }
inline uint8_t lerp8by8(uint8_t val, uint8_t a, uint8_t b) { return lerp<uint8_t>(val, a, b); }

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

// BlinkBrightnessEvaluator does one on-off cycle in the specified period
template<typename Brightness>
struct BlinkBrightnessEvaluator {
    uint16_t duration_on_, duration_off_;

    uint16_t Period() const { return duration_on_ + duration_off_; }
    Brightness Eval(uint32_t t) const {
        return (t < duration_on_) ? BrightnessTraits<Brightness>::kFullBrightness
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
    uint16_t Period() const {
        return duration_fade_on_ + duration_on_ + duration_fade_off_;
    }
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

    uint16_t DurationFadeOn() const { return duration_fade_on_; }
    uint16_t DurationFadeOff() const { return duration_fade_off_; }
    uint16_t DurationOn() const { return duration_on_; }
    Brightness From() const { return from_; }
    Brightness To() const { return to_; }
};

template<typename Brightness>
struct CandleBrightnessEvaluator {
    uint8_t speed_;
    uint8_t jitter_;
    uint16_t period_;
    mutable Brightness last_;
    mutable uint32_t last_t_ = 0;

    CandleBrightnessEvaluator() = delete;

    // speed - speed of effect (0..15). 0 fastest. Each increment by 1
    //         halfes the speed.
    // jitter - amount of jittering to apply. 0 - no jitter, 15 - candle,
    //                                        64 - fire, 255 - storm
    CandleBrightnessEvaluator(uint8_t speed, uint8_t jitter, uint16_t period)
        : speed_(speed), jitter_(jitter), period_(period),
          last_(scale<Brightness>(BrightnessTraits<Brightness>::kFullBrightness,
                                      static_cast<Brightness>(5))) {}

    uint16_t Period() const { return period_; }
    Brightness Eval(uint32_t t) const {
        // idea from
        // https://cpldcpu.wordpress.com/2013/12/08/hacking-a-candleflicker-led/
        // TODO(jd) finetune values
        // Table values are 8-bit, need to be scaled to Brightness
        static constexpr uint8_t kCandleTable[] = {
            5, 10, 20, 30, 50, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 255};
        if ((t >> speed_) == last_t_) return last_;
        last_t_ = (t >> speed_);
        const auto rnd = rand8() & 255;

        // Scale 8-bit table values to Brightness
        constexpr auto kFullBright = BrightnessTraits<Brightness>::kFullBrightness;
        if (rnd >= jitter_) {
            last_ = kFullBright;
        } else {
            // Scale table value from 8-bit to Brightness
            const uint8_t table_val = 50 + kCandleTable[rnd & 0xf];
            // Use sizeof to determine type at compile time
            // (optimizes to same code as if constexpr)
            if (sizeof(Brightness) == 1) {
                last_ = table_val;
            } else {
                // For 16-bit: scale 8-bit value to 16-bit
                last_ = static_cast<Brightness>(
                    (static_cast<uint32_t>(table_val) * 65535) / 255);
            }
        }
        return last_;
    }
};

// Identifies which brightness evaluator is active in EvalStorage.
enum class EvalType : uint8_t { NONE = 0, CONSTANT, BLINK, BREATHE, CANDLE, USER };

// Type-safe discriminated union holding the active brightness evaluator.
// Dispatches Period() and Eval() via switch — no virtual functions for
// built-in effects. The USER arm calls through the user's virtual pointer.
template<typename Brightness>
struct EvalStorage {
    EvalType type = EvalType::NONE;

    union Data {
        ConstantBrightnessEvaluator<Brightness> constant;
        BlinkBrightnessEvaluator<Brightness>    blink;
        BreatheBrightnessEvaluator<Brightness>  breathe;
        CandleBrightnessEvaluator<Brightness>   candle;
        BrightnessEvaluator<Brightness>*        user;
        Data() {}
        ~Data() {}
    } data;

    bool IsSet() const { return type != EvalType::NONE; }

    uint16_t Period() const {
        switch (type) {
            case EvalType::CONSTANT: return data.constant.Period();
            case EvalType::BLINK:    return data.blink.Period();
            case EvalType::BREATHE:  return data.breathe.Period();
            case EvalType::CANDLE:   return data.candle.Period();
            case EvalType::USER:     return data.user->Period();
            default:                 return 0;
        }
    }

    Brightness Eval(uint32_t t) const {
        switch (type) {
            case EvalType::CONSTANT: return data.constant.Eval(t);
            case EvalType::BLINK:    return data.blink.Eval(t);
            case EvalType::BREATHE:  return data.breathe.Eval(t);
            case EvalType::CANDLE:   return data.candle.Eval(t);
            case EvalType::USER:     return data.user->Eval(t);
            default:                 return BrightnessTraits<Brightness>::kZeroBrightness;
        }
    }
};

template <typename Hal, typename Clock, typename Brightness, typename Derived>
class TJLed {
 protected:
    // Active brightness evaluator (discriminated union).
    EvalStorage<Brightness> eval_storage_;
    // Hardware abstraction giving access to the MCU
    Hal hal_;

    // Evaluate effect(t) — assumes eval_storage_.IsSet().
    Brightness Eval(uint32_t t) const { return eval_storage_.Eval(t); }

    // Write val out to the "hardware", inverting signal when active-low is set.
    void Write(Brightness val) {
        constexpr auto kFullBright = BrightnessTraits<Brightness>::kFullBrightness;
        hal_.template analogWrite<Brightness>(IsLowActive() ? kFullBright - val : val);
    }

 public:
    TJLed() = delete;
    explicit TJLed(const Hal& hal)
        : hal_{hal},
          state_{ST_INIT},
          bLowActive_{false},
          minBrightness_{BrightnessTraits<Brightness>::kZeroBrightness},
          maxBrightness_{BrightnessTraits<Brightness>::kFullBrightness} {}

    explicit TJLed(typename Hal::PinType pin)
        : hal_{pin},
          state_{ST_INIT},
          bLowActive_{false},
          minBrightness_{BrightnessTraits<Brightness>::kZeroBrightness},
          maxBrightness_{BrightnessTraits<Brightness>::kFullBrightness} {}

    TJLed(const TJLed& rLed) : hal_{rLed.hal_} { *this = rLed; }

    Derived& operator=(const TJLed<Hal, Clock, Brightness, Derived>& rLed) {
        state_ = rLed.state_;
        bLowActive_ = rLed.bLowActive_;
        minBrightness_ = rLed.minBrightness_;
        maxBrightness_ = rLed.maxBrightness_;
        num_repetitions_ = rLed.num_repetitions_;
        last_update_time_ = rLed.last_update_time_;
        delay_before_ = rLed.delay_before_;
        delay_after_ = rLed.delay_after_;
        time_start_ = rLed.time_start_;
        hal_ = rLed.hal_;
        eval_storage_ = rLed.eval_storage_;
        return static_cast<Derived&>(*this);
    }

    Hal& GetHal() { return hal_; }

    // Set physical LED polarity to be low active. This inverts every
    // signal physically output to a pin.
    Derived& LowActive() {
        bLowActive_ = true;
        return static_cast<Derived&>(*this);
    }

    bool IsLowActive() const { return bLowActive_; }

    // turn LED on
    Derived& On(uint16_t duration = 1) {
        return Set(BrightnessTraits<Brightness>::kFullBrightness, duration);
    }

    // turn LED off
    Derived& Off(uint16_t duration = 1) {
        return Set(BrightnessTraits<Brightness>::kZeroBrightness, duration);
    }

    // Sets LED to given brightness. As for every effect, a duration can be
    // specified. Update() will return false after the duration elapsed.
    Derived& Set(Brightness brightness, uint16_t duration = 1) {
        eval_storage_.type = EvalType::CONSTANT;
        eval_storage_.data.constant = {brightness, duration};
        return Reset();
    }

    // Fade LED on
    Derived& FadeOn(uint16_t duration,
                    Brightness from = BrightnessTraits<Brightness>::kZeroBrightness,
                    Brightness to = BrightnessTraits<Brightness>::kFullBrightness) {
        eval_storage_.type = EvalType::BREATHE;
        eval_storage_.data.breathe = {duration, 0, 0, from, to};
        return Reset();
    }

    // Fade LED off - actually is just inverted version of FadeOn()
    Derived& FadeOff(uint16_t duration,
                     Brightness from = BrightnessTraits<Brightness>::kFullBrightness,
                     Brightness to = BrightnessTraits<Brightness>::kZeroBrightness) {
        eval_storage_.type = EvalType::BREATHE;
        eval_storage_.data.breathe = {0, 0, duration, to, from};
        return Reset();
    }

    // Fade from "from" to "to" with period "duration". Sets up the breathe
    // effect with the proper parameters and sets Min/Max brightness to reflect
    // levels specified by "from" and "to".
    Derived& Fade(Brightness from, Brightness to, uint16_t duration) {
        if (from < to) {
            return FadeOn(duration, from, to);
        } else {
            return FadeOff(duration, from, to);
        }
    }

    // Set effect to Breathe, with the given period time in ms.
    Derived& Breathe(uint16_t period) { return Breathe(period / 2, 0, period / 2); }

    // Set effect to Breathe, with the given fade on-, on- and fade off-
    // duration values.
    Derived& Breathe(uint16_t duration_fade_on, uint16_t duration_on,
                     uint16_t duration_fade_off) {
        eval_storage_.type = EvalType::BREATHE;
        eval_storage_.data.breathe = {duration_fade_on, duration_on, duration_fade_off,
            BrightnessTraits<Brightness>::kZeroBrightness,
            BrightnessTraits<Brightness>::kFullBrightness};
        return Reset();
    }

    // Set effect to Blink, with the given on- and off- duration values.
    Derived& Blink(uint16_t duration_on, uint16_t duration_off) {
        eval_storage_.type = EvalType::BLINK;
        eval_storage_.data.blink = {duration_on, duration_off};
        return Reset();
    }

    // Set effect to Candle light simulation
    Derived& Candle(uint8_t speed = 6, uint8_t jitter = 15,
                    uint16_t period = 0xffff) {
        eval_storage_.type = EvalType::CANDLE;
        eval_storage_.data.candle = CandleBrightnessEvaluator<Brightness>(speed, jitter, period);
        return Reset();
    }

    // Use a user provided brightness evaluator.
    Derived& UserFunc(BrightnessEvaluator<Brightness>* user_eval) {
        eval_storage_.type = EvalType::USER;
        eval_storage_.data.user = user_eval;
        return Reset();
    }

    // set number of repetitions for effect.
    Derived& Repeat(uint16_t num_repetitions) {
        num_repetitions_ = num_repetitions;
        return static_cast<Derived&>(*this);
    }

    // repeat Forever
    Derived& Forever() { return Repeat(kRepeatForever); }
    bool IsForever() const { return num_repetitions_ == kRepeatForever; }

    // Set amount of time to initially wait before effect starts. Time is
    // relative to first call of Update() method and specified in ms.
    Derived& DelayBefore(uint16_t delay_before) {
        delay_before_ = delay_before;
        return static_cast<Derived&>(*this);
    }

    // Set amount of time to wait in ms after each iteration.
    Derived& DelayAfter(uint16_t delay_after) {
        delay_after_ = delay_after;
        return static_cast<Derived&>(*this);
    }

    // Stop current effect and turn LED immeadiately off. Further calls to
    // Update() will have no effect.
    enum eStopMode { TO_MIN_BRIGHTNESS = 0, FULL_OFF, KEEP_CURRENT };
    Derived& Stop(eStopMode mode = eStopMode::TO_MIN_BRIGHTNESS) {
        if (mode != eStopMode::KEEP_CURRENT) {
            Write(mode == eStopMode::FULL_OFF
                      ? BrightnessTraits<Brightness>::kZeroBrightness
                      : minBrightness_);
        }
        state_ = ST_STOPPED;
        return static_cast<Derived&>(*this);
    }

    bool IsRunning() const { return state_ != ST_STOPPED; }

    // Reset to inital state
    Derived& Reset() {
        time_start_ = 0;
        last_update_time_ = 0;
        state_ = ST_INIT;
        return static_cast<Derived&>(*this);
    }

    // Sets the minimum brightness level.
    Derived& MinBrightness(Brightness level) {
        minBrightness_ = level;
        return static_cast<Derived&>(*this);
    }

    // Returns current minimum brightness level.
    Brightness MinBrightness() const { return minBrightness_; }

    // Sets the maximum brightness level.
    Derived& MaxBrightness(Brightness level) {
        maxBrightness_ = level;
        return static_cast<Derived&>(*this);
    }

    // Returns current maximum brightness level.
    Brightness MaxBrightness() const { return maxBrightness_; }

    // update brightness of LED using the given brightness evaluator and the
    // current time. If the optional pLast pointer is set, then the actual
    // brightness value (if an update happened), will be returned through
    // the pointer. The value returned will be the calculated value after
    // min- and max-brightness scaling was applied, which is the value that
    // is written to the output.
    //
    //  (brightness)                       ________________
    // on 255 |                         ¸-'
    //        |                      ¸-'
    //        |                   ¸-'
    // off 0  |________________¸-'
    //        |<-delay before->|<--period-->|<-delay after-> (time)
    //                         | func(t)    |
    //                         |<- num_repetitions times  ->
    bool Update(int16_t* pLast = nullptr) {
        return Update(Clock::millis(), pLast);
    }

    bool Update(uint32_t t, int16_t* pLast = nullptr) {
        if (state_ == ST_STOPPED || !eval_storage_.IsSet()) return false;

        if (state_ == ST_INIT) {
            time_start_ = t + delay_before_;
            state_ = ST_RUNNING;
        } else {
            // no need to process updates twice during one time tick.
            if (!timeChangedSinceLastUpdate(t)) return true;
        }

        trackLastUpdateTime(t);

        if (static_cast<int32_t>(t - time_start_) < 0) return true;

        auto writeCur = [this](uint32_t t, int16_t* p) {
            const auto val = lerp<Brightness>(Eval(t), minBrightness_, maxBrightness_);
            if (p) {
                *p = static_cast<int16_t>(val);
            }
            Write(val);
        };

        const auto period = eval_storage_.Period();
        const auto cycle_period = period + delay_after_;

        if (!IsForever()) {
            const auto time_end = time_start_ +
                                  cycle_period * num_repetitions_ - 1;

            if (static_cast<int32_t>(t - time_end) >= 0) {
                // make sure final value of t = (period-1) is set
                state_ = ST_STOPPED;
                writeCur(period - 1, pLast);
                return false;
            }
        }

        const uint32_t elapsed = t - time_start_;
        const uint32_t t_cycle = elapsed % cycle_period;

        if (t_cycle < period) {
            state_ = ST_RUNNING;
            writeCur(t_cycle, pLast);
        } else {
            if (state_ == ST_RUNNING) {
                // when in delay after phase, just call Write()
                // once at the beginning.
                state_ = ST_IN_DELAY_AFTER_PHASE;
                writeCur(period - 1, pLast);
            }
        }
        return true;
    }

 protected:
    // test if time stored in last_update_time_ differs from provided timestamp.
    bool inline timeChangedSinceLastUpdate(uint32_t now) {
        return (now & 255) != last_update_time_;
    }

    void trackLastUpdateTime(uint32_t t) { last_update_time_ = (t & 255); }

 public:
    // Number of bits used to control brightness with Min/MaxBrightness().
    static constexpr uint8_t kBitsBrightness = BrightnessTraits<Brightness>::kBits;
    static constexpr Brightness kBrightnessStep = 1;

 private:
    static constexpr uint8_t ST_STOPPED = 0;
    static constexpr uint8_t ST_INIT = 1;
    static constexpr uint8_t ST_RUNNING = 2;
    static constexpr uint8_t ST_IN_DELAY_AFTER_PHASE = 3;

    uint8_t state_ : 2;
    uint8_t bLowActive_ : 1;
    Brightness minBrightness_;
    Brightness maxBrightness_;

    static constexpr uint16_t kRepeatForever = 65535;
    uint16_t num_repetitions_ = 1;

    // We store the timestamp the effect was last updated to avoid multiple
    // updates when called during the same time tick.  Only the lower 8 bits of
    // the timestamp are used (which saves us 3 bytes of memory per JLed
    // instance), resulting in limited accuracy, which may lead to false
    // negatives if Update() is not called for a longer time (i.e. > 255ms),
    // which should not be a problem at all.
    uint8_t last_update_time_ = 0;
    uint32_t time_start_ = 0;

    uint16_t delay_before_ = 0;  // delay before the first effect starts
    uint16_t delay_after_ = 0;   // delay after each repetition
};

template <typename T>
T* ptr(T& obj) {  // NOLINT
    return &obj;
}
template <typename T>
T* ptr(T* obj) {
    return obj;
}

// a group of JLed objects which can be controlled simultanously, in parallel
// or sequentially.
template <typename L, typename Clock, typename B>
class TJLedSequence {
 protected:
    // update all leds parallel. Returns true while any of the JLeds is
    // active, else false
    bool UpdateParallel() {
        auto result = false;
        uint32_t t = Clock::millis();
        for (auto i = 0u; i < n_; i++) {
            result |= ptr(leds_[i])->Update(t);
        }
        return result;
    }

    // update all leds sequentially. Returns true while any of the JLeds is
    // active, else false
    bool UpdateSequentially() {
        if (!ptr(leds_[cur_])->Update()) {
            return ++cur_ < n_;
        }
        return true;
    }

    void ResetLeds() {
        for (auto i = 0u; i < n_; i++) {
            ptr(leds_[i])->Reset();
        }
    }

 public:
    enum eMode { SEQUENCE, PARALLEL };
    TJLedSequence() = delete;

    template <size_t N>
    TJLedSequence(eMode mode, L (&leds)[N]) : TJLedSequence(mode, leds, N) {}

    TJLedSequence(eMode mode, L* leds, size_t n)
        : mode_{mode}, leds_{leds}, cur_{0}, n_{n} {}

    bool Update() {
        if (!is_running_ || n_ < 1) {
            return false;
        }

        const auto led_running = (mode_ == eMode::PARALLEL)
                                     ? UpdateParallel()
                                     : UpdateSequentially();
        if (led_running) {
            return true;
        }

        // start next iteration of sequence
        cur_ = 0;
        ResetLeds();

        is_running_ = ++iteration_ < num_repetitions_ ||
                      num_repetitions_ == kRepeatForever;

        return is_running_;
    }

    void Reset() {
        ResetLeds();
        cur_ = 0;
        iteration_ = 0;
        is_running_ = true;
    }

    void Stop() {
        is_running_ = false;
        for (auto i = 0u; i < n_; i++) {
            ptr(leds_[i])->Stop();
        }
    }

    // set number of repetitions for the sequence
    B& Repeat(uint16_t num_repetitions) {
        num_repetitions_ = num_repetitions;
        return static_cast<B&>(*this);
    }

    // repeat Forever
    B& Forever() { return Repeat(kRepeatForever); }
    bool IsForever() const { return num_repetitions_ == kRepeatForever; }

 private:
    eMode mode_;
    L* leds_;
    size_t cur_;
    size_t n_;
    static constexpr uint16_t kRepeatForever = 65535;
    uint16_t num_repetitions_ = 1;
    uint16_t iteration_ = 0;
    bool is_running_ = true;
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
        return (static_cast<uint16_t>(val) * static_cast<uint16_t>(1+factor)) >> 8;
    } else {
        return (static_cast<uint32_t>(val) * static_cast<uint32_t>(1+factor)) >> 16;
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
// 8-bit specialization uses pre-computed table from jled_base.cpp
template<>
uint8_t fadeon_func<uint8_t>(uint32_t t, uint16_t period);

// 16-bit specialization: interpolate from 8-bit table and scale to 16-bit
// This is implemented in jled_base.cpp
template<>
uint16_t fadeon_func<uint16_t>(uint32_t t, uint16_t period);

};  // namespace jled
