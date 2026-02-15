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
#ifndef SRC_JLED_BASE_H_
#define SRC_JLED_BASE_H_

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

// Template helper functions - implemented below after evaluator definitions
template<typename BrightnessType>
BrightnessType fadeon_func(uint32_t t, uint16_t period);

template<typename BrightnessType>
BrightnessType scale(BrightnessType val, BrightnessType factor);

template<typename BrightnessType>
BrightnessType lerp(BrightnessType val, BrightnessType a, BrightnessType b);

// Legacy 8-bit function names (inline wrappers for backwards compatibility)
inline uint8_t scale8(uint8_t val, uint8_t f) { return scale<uint8_t>(val, f); }
inline uint8_t lerp8by8(uint8_t val, uint8_t a, uint8_t b) { return lerp<uint8_t>(val, a, b); }

template <typename T>
static constexpr T __max(T a, T b) {
    return (a > b) ? a : b;
}

// a function f(t,period,param) that calculates the LEDs brightness for a given
// point in time and the given period. param is an optionally user provided
// parameter. t will always be in range [0..period-1].
// f(period-1,period,param) will be called last to calculate the final state of
// the LED.
template<typename BrightnessType>
class BrightnessEvaluator {
 public:
    virtual uint16_t Period() const = 0;
    virtual BrightnessType Eval(uint32_t t) const = 0;
};

template<typename BrightnessType>
class CloneableBrightnessEvaluator : public BrightnessEvaluator<BrightnessType> {
 public:
    virtual BrightnessEvaluator<BrightnessType>* clone(void* ptr) const = 0;
    static void* operator new(size_t, void* ptr) { return ptr; }
    static void operator delete(void*) {}
};

template<typename BrightnessType>
class ConstantBrightnessEvaluator : public CloneableBrightnessEvaluator<BrightnessType> {
    BrightnessType val_;
    uint16_t duration_;

 public:
    ConstantBrightnessEvaluator() = delete;
    explicit ConstantBrightnessEvaluator(BrightnessType val, uint16_t duration = 1)
        : val_(val), duration_(duration) {}
    BrightnessEvaluator<BrightnessType>* clone(void* ptr) const override {
        return new (ptr) ConstantBrightnessEvaluator(*this);
    }
    uint16_t Period() const override { return duration_; }
    BrightnessType Eval(uint32_t) const override { return val_; }
};

// BlinkBrightnessEvaluator does one on-off cycle in the specified period
template<typename BrightnessType>
class BlinkBrightnessEvaluator : public CloneableBrightnessEvaluator<BrightnessType> {
    uint16_t duration_on_, duration_off_;

 public:
    BlinkBrightnessEvaluator() = delete;
    BlinkBrightnessEvaluator(uint16_t duration_on, uint16_t duration_off)
        : duration_on_(duration_on), duration_off_(duration_off) {}
    BrightnessEvaluator<BrightnessType>* clone(void* ptr) const override {
        return new (ptr) BlinkBrightnessEvaluator(*this);
    }
    uint16_t Period() const override { return duration_on_ + duration_off_; }
    BrightnessType Eval(uint32_t t) const override {
        return (t < duration_on_) ? BrightnessTypeTraits<BrightnessType>::kFullBrightness
                                  : BrightnessTypeTraits<BrightnessType>::kZeroBrightness;
    }
};

// The breathe func is composed by fade-on, on and fade-off phases. For fading
// we approximate the following function:
//   y(x) = exp(sin((t-period/4.) * 2. * PI / period)) - 0.36787944) *  108.)
// idea see:
//   http://sean.voisen.org/blog/2011/10/breathing-led-with-arduino/
// But we do it with integers only.
template<typename BrightnessType>
class BreatheBrightnessEvaluator : public CloneableBrightnessEvaluator<BrightnessType> {
    uint16_t duration_fade_on_;
    uint16_t duration_on_;
    uint16_t duration_fade_off_;
    BrightnessType from_;
    BrightnessType to_;

 public:
    BreatheBrightnessEvaluator() = delete;
    explicit BreatheBrightnessEvaluator(
        uint16_t duration_fade_on,
        uint16_t duration_on,
        uint16_t duration_fade_off,
        BrightnessType from =
            BrightnessTypeTraits<BrightnessType>::kZeroBrightness,
        BrightnessType to =
            BrightnessTypeTraits<BrightnessType>::kFullBrightness)
        : duration_fade_on_(duration_fade_on),
          duration_on_(duration_on),
          duration_fade_off_(duration_fade_off),
          from_(from),
          to_(to) {}
    BrightnessEvaluator<BrightnessType>* clone(void* ptr) const override {
        return new (ptr) BreatheBrightnessEvaluator(*this);
    }
    uint16_t Period() const override {
        return duration_fade_on_ + duration_on_ + duration_fade_off_;
    }
    BrightnessType Eval(uint32_t t) const override {
        BrightnessType val = BrightnessTypeTraits<BrightnessType>::kZeroBrightness;
        if (t < duration_fade_on_)
            val = fadeon_func<BrightnessType>(t, duration_fade_on_);
        else if (t < duration_fade_on_ + duration_on_)
            val = BrightnessTypeTraits<BrightnessType>::kFullBrightness;
        else
            val = fadeon_func<BrightnessType>(Period() - t, duration_fade_off_);
        return lerp<BrightnessType>(val, from_, to_);
    }

    uint16_t DurationFadeOn() const { return duration_fade_on_; }
    uint16_t DurationFadeOff() const { return duration_fade_off_; }
    uint16_t DurationOn() const { return duration_on_; }
    BrightnessType From() const { return from_; }
    BrightnessType To() const { return to_; }
};

template<typename BrightnessType>
class CandleBrightnessEvaluator : public CloneableBrightnessEvaluator<BrightnessType> {
    uint8_t speed_;
    uint8_t jitter_;
    uint16_t period_;
    mutable BrightnessType last_;
    mutable uint32_t last_t_ = 0;

 public:
    CandleBrightnessEvaluator() = delete;

    // speed - speed of effect (0..15). 0 fastest. Each increment by 1
    //         halfes the speed.
    // jitter - amount of jittering to apply. 0 - no jitter, 15 - candle,
    //                                        64 - fire, 255 - storm
    CandleBrightnessEvaluator(uint8_t speed, uint8_t jitter, uint16_t period)
        : speed_(speed), jitter_(jitter), period_(period),
          last_(scale<BrightnessType>(BrightnessTypeTraits<BrightnessType>::kFullBrightness,
                                      static_cast<BrightnessType>(5))) {}

    BrightnessEvaluator<BrightnessType>* clone(void* ptr) const override {
        return new (ptr) CandleBrightnessEvaluator(*this);
    }

    uint16_t Period() const override { return period_; }
    BrightnessType Eval(uint32_t t) const override {
        // idea from
        // https://cpldcpu.wordpress.com/2013/12/08/hacking-a-candleflicker-led/
        // TODO(jd) finetune values
        // Table values are 8-bit, need to be scaled to BrightnessType
        static constexpr uint8_t kCandleTable[] = {
            5, 10, 20, 30, 50, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 255};
        if ((t >> speed_) == last_t_) return last_;
        last_t_ = (t >> speed_);
        const auto rnd = rand8() & 255;

        // Scale 8-bit table values to BrightnessType
        constexpr auto kFullBright = BrightnessTypeTraits<BrightnessType>::kFullBrightness;
        if (rnd >= jitter_) {
            last_ = kFullBright;
        } else {
            // Scale table value from 8-bit to BrightnessType
            const uint8_t table_val = 50 + kCandleTable[rnd & 0xf];
            // Use sizeof to determine type at compile time
            // (optimizes to same code as if constexpr)
            if (sizeof(BrightnessType) == 1) {
                last_ = table_val;
            } else {
                // For 16-bit: scale 8-bit value to 16-bit
                last_ = static_cast<BrightnessType>(
                    (static_cast<uint32_t>(table_val) * 65535) / 255);
            }
        }
        return last_;
    }
};

template <typename Hal, typename Clock, typename BrightnessType, typename Derived>
class TJLed {

 protected:
    // pointer to a (user defined) brightness evaluator.
    BrightnessEvaluator<BrightnessType>* brightness_eval_ = nullptr;
    // Hardware abstraction giving access to the MCU
    Hal hal_;

    // Evaluate effect(t) and scale to be within [minBrightness, maxBrightness]
    // assumes brigthness_eval_ is set as it is not checked here.
    BrightnessType Eval(uint32_t t) const { return brightness_eval_->Eval(t); }

    // Write val out to the "hardware", inverting signal when active-low is set.
    void Write(BrightnessType val) {
        constexpr auto kFullBright = BrightnessTypeTraits<BrightnessType>::kFullBrightness;
        hal_.template analogWrite<BrightnessType>(IsLowActive() ? kFullBright - val : val);
    }

 public:
    TJLed() = delete;
    explicit TJLed(const Hal& hal)
        : hal_{hal},
          state_{ST_INIT},
          bLowActive_{false},
          minBrightness_{BrightnessTypeTraits<BrightnessType>::kZeroBrightness},
          maxBrightness_{BrightnessTypeTraits<BrightnessType>::kFullBrightness} {}

    explicit TJLed(typename Hal::PinType pin) : TJLed{Hal{pin}} {}

    TJLed(const TJLed& rLed) : hal_{rLed.hal_} { *this = rLed; }

    Derived& operator=(const TJLed<Hal, Clock, BrightnessType, Derived>& rLed) {
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

        if (rLed.brightness_eval_ !=
            reinterpret_cast<const BrightnessEvaluator<BrightnessType>*>(
                rLed.brightness_eval_buf_)) {
            // nullptr or points to (external) user provided evaluator
            brightness_eval_ = rLed.brightness_eval_;
        } else {
            brightness_eval_ =
                (reinterpret_cast<const CloneableBrightnessEvaluator<BrightnessType>*>(
                     rLed.brightness_eval_))
                    ->clone(brightness_eval_buf_);
        }
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
        return Set(BrightnessTypeTraits<BrightnessType>::kFullBrightness, duration);
    }

    // turn LED off
    Derived& Off(uint16_t duration = 1) {
        return Set(BrightnessTypeTraits<BrightnessType>::kZeroBrightness, duration);
    }

    // Sets LED to given brightness. As for every effect, a duration can be
    // specified. Update() will return false after the duration elapsed.
    Derived& Set(BrightnessType brightness, uint16_t duration = 1) {
        // note: we use placement new and therefore not need to keep track of
        // mem allocated
        return SetBrightnessEval(
            new (brightness_eval_buf_)
                ConstantBrightnessEvaluator<BrightnessType>(brightness, duration));
    }

    // Fade LED on
    Derived& FadeOn(uint16_t duration,
                    BrightnessType from = BrightnessTypeTraits<BrightnessType>::kZeroBrightness,
                    BrightnessType to = BrightnessTypeTraits<BrightnessType>::kFullBrightness) {
        return SetBrightnessEval(
            new (brightness_eval_buf_)
                BreatheBrightnessEvaluator<BrightnessType>(duration, 0, 0, from, to));
    }

    // Fade LED off - acutally is just inverted version of FadeOn()
    Derived& FadeOff(uint16_t duration,
                     BrightnessType from = BrightnessTypeTraits<BrightnessType>::kFullBrightness,
                     BrightnessType to = BrightnessTypeTraits<BrightnessType>::kZeroBrightness) {
        return SetBrightnessEval(
            new (brightness_eval_buf_)
                BreatheBrightnessEvaluator<BrightnessType>(0, 0, duration, to, from));
    }

    // Fade from "from" to "to" with period "duration". Sets up the breathe
    // effect with the proper parameters and sets Min/Max brightness to reflect
    // levels specified by "from" and "to".
    Derived& Fade(BrightnessType from, BrightnessType to, uint16_t duration) {
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
        return SetBrightnessEval(
            new (brightness_eval_buf_) BreatheBrightnessEvaluator<BrightnessType>(
                duration_fade_on, duration_on, duration_fade_off));
    }

    // Set effect to Blink, with the given on- and off- duration values.
    Derived& Blink(uint16_t duration_on, uint16_t duration_off) {
        return SetBrightnessEval(
            new (brightness_eval_buf_)
                BlinkBrightnessEvaluator<BrightnessType>(duration_on, duration_off));
    }

    // Set effect to Candle light simulation
    Derived& Candle(uint8_t speed = 6, uint8_t jitter = 15,
                    uint16_t period = 0xffff) {
        return SetBrightnessEval(
            new (brightness_eval_buf_)
                CandleBrightnessEvaluator<BrightnessType>(speed, jitter, period));
    }

    // Use a user provided brightness evaluator.
    Derived& UserFunc(BrightnessEvaluator<BrightnessType>* user_eval) {
        return SetBrightnessEval(user_eval);
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
                      ? BrightnessTypeTraits<BrightnessType>::kZeroBrightness
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
    Derived& MinBrightness(BrightnessType level) {
        minBrightness_ = level;
        return static_cast<Derived&>(*this);
    }

    // Returns current minimum brightness level.
    BrightnessType MinBrightness() const { return minBrightness_; }

    // Sets the maximum brightness level.
    Derived& MaxBrightness(BrightnessType level) {
        maxBrightness_ = level;
        return static_cast<Derived&>(*this);
    }

    // Returns current maximum brightness level.
    BrightnessType MaxBrightness() const { return maxBrightness_; }

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
        if (state_ == ST_STOPPED || !brightness_eval_) return false;

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
            const auto val = lerp<BrightnessType>(Eval(t), minBrightness_, maxBrightness_);
            if (p) {
                *p = static_cast<int16_t>(val);
            }
            Write(val);
        };

        // t cycles in range [0..period+delay_after-1]
        const auto period = brightness_eval_->Period();

        if (!IsForever()) {
            const auto time_end = time_start_ +
                                  static_cast<uint32_t>(period + delay_after_) *
                                      num_repetitions_ -
                                  1;

            if (static_cast<int32_t>(t - time_end) >= 0) {
                // make sure final value of t = (period-1) is set
                state_ = ST_STOPPED;
                writeCur(period - 1, pLast);
                return false;
            }
        }

        t = (t - time_start_) % (period + delay_after_);
        if (t < period) {
            state_ = ST_RUNNING;
            writeCur(t, pLast);
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

    Derived& SetBrightnessEval(BrightnessEvaluator<BrightnessType>* be) {
        brightness_eval_ = be;
        // start over after the brightness evaluator changed
        return Reset();
    }

 public:
    // Number of bits used to control brightness with Min/MaxBrightness().
    static constexpr uint8_t kBitsBrightness = BrightnessTypeTraits<BrightnessType>::kBits;
    static constexpr BrightnessType kBrightnessStep = 1;

 private:
    static constexpr uint8_t ST_STOPPED = 0;
    static constexpr uint8_t ST_INIT = 1;
    static constexpr uint8_t ST_RUNNING = 2;
    static constexpr uint8_t ST_IN_DELAY_AFTER_PHASE = 3;

    uint8_t state_ : 2;
    uint8_t bLowActive_ : 1;
    BrightnessType minBrightness_;
    BrightnessType maxBrightness_;

    // this is where the BrightnessEvaluator object will be stored using
    // placment new.  Set MAX_SIZE to class occupying most memory
    static constexpr auto MAX_SIZE =
        __max(sizeof(CandleBrightnessEvaluator<BrightnessType>),
              __max(sizeof(BreatheBrightnessEvaluator<BrightnessType>),
                    __max(sizeof(ConstantBrightnessEvaluator<BrightnessType>),  // NOLINT
                          sizeof(BlinkBrightnessEvaluator<BrightnessType>))));
    alignas(alignof(
        CloneableBrightnessEvaluator<BrightnessType>)) char brightness_eval_buf_[MAX_SIZE];

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
template<typename BrightnessType>
BrightnessType scale(BrightnessType val, BrightnessType factor) {
    // Use sizeof to determine type at compile time (optimizes to same code as if constexpr)
    if (sizeof(BrightnessType) == 1) {
        return (static_cast<uint16_t>(val) * static_cast<uint16_t>(1+factor)) >> 8;
    } else {
        return (static_cast<uint32_t>(val) * static_cast<uint32_t>(1+factor)) >> 16;
    }
}

// Linear interpolation: map val from [0,max] to [a,b]
template<typename BrightnessType>
BrightnessType lerp(BrightnessType val, BrightnessType a, BrightnessType b) {
    constexpr auto kMax = BrightnessTypeTraits<BrightnessType>::kFullBrightness;
    // Optimize for most common case: full range
    if (a == 0 && b == kMax) return val;
    const BrightnessType delta = b - a;
    return a + scale<BrightnessType>(val, delta);
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
#endif  // SRC_JLED_BASE_H_
