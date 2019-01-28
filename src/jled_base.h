// Copyright (c) 2017 Jan Delgado <jdelgado[at]gmx.net>
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
#ifdef HAS_TYPE_TRAITS
#include <type_traits>
#endif
#include "jled_hal.h"  // NOLINT

// JLed - non-blocking LED abstraction library.
//
// Example Arduino sketch:
//   JLed led = JLed(LED_BUILTIN).Blink(500, 500).Repeat(10).DelayBefore(1000);
//
//   void setup() {}
//
//   void loop() {
//     led.Update();
//   }

namespace jled {

static constexpr uint8_t kFullBrightness = 255;
static constexpr uint8_t kZeroBrightness = 0;

uint8_t fadeon_func(uint32_t t, uint16_t period);

// a function f(t,period,param) that calculates the LEDs brightness for a given
// point in time and the given period. param is an optionally user provided
// parameter. t will always be in range [0..period-1].
// f(period-1,period,param) will be called last to calculate the final state of
// the LED.
class BrightnessEvaluator {
 public:
    virtual uint16_t Period() const = 0;
    virtual uint8_t Eval(uint32_t t) const = 0;
    virtual BrightnessEvaluator* clone(void* ptr) const = 0;
    static void* operator new(size_t, void* ptr) { return ptr; }
    static void operator delete(void*) {}
};

class ConstantBrightnessEvaluator : public BrightnessEvaluator {
    uint8_t val_;
    using BrightnessEvaluator::BrightnessEvaluator;

 public:
    ConstantBrightnessEvaluator() = delete;
    explicit ConstantBrightnessEvaluator(uint8_t val) : val_(val) {}
    BrightnessEvaluator* clone(void* ptr) const override {
        return new (ptr) ConstantBrightnessEvaluator(*this);
    }
    uint16_t Period() const override { return 1; }
    uint8_t Eval(uint32_t) const override { return val_; }
};

// BlinkBrightnessEvaluator does one on-off cycle in the specified period
class BlinkBrightnessEvaluator : public BrightnessEvaluator {
    uint16_t duration_on_, duration_off_;

 public:
    BlinkBrightnessEvaluator() = delete;
    BlinkBrightnessEvaluator(uint16_t duration_on, uint16_t duration_off)
        : duration_on_(duration_on), duration_off_(duration_off) {}
    BrightnessEvaluator* clone(void* ptr) const override {
        return new (ptr) BlinkBrightnessEvaluator(*this);
    }
    uint16_t Period() const override { return duration_on_ + duration_off_; }
    uint8_t Eval(uint32_t t) const override {
        return (t < duration_on_) ? kFullBrightness : kZeroBrightness;
    }
};

// fade LED on
class FadeOnBrightnessEvaluator : public BrightnessEvaluator {
    uint16_t period_;
    using BrightnessEvaluator::BrightnessEvaluator;

 public:
    FadeOnBrightnessEvaluator() = delete;
    explicit FadeOnBrightnessEvaluator(uint16_t period) : period_(period) {}
    BrightnessEvaluator* clone(void* ptr) const override {
        return new (ptr) FadeOnBrightnessEvaluator(*this);
    }
    uint16_t Period() const override { return period_; }
    uint8_t Eval(uint32_t t) const override { return fadeon_func(t, period_); }
};

// fade LED off
class FadeOffBrightnessEvaluator : public BrightnessEvaluator {
    uint16_t period_;
    using BrightnessEvaluator::BrightnessEvaluator;

 public:
    FadeOffBrightnessEvaluator() = delete;
    explicit FadeOffBrightnessEvaluator(uint16_t period) : period_(period) {}
    BrightnessEvaluator* clone(void* ptr) const override {
        return new (ptr) FadeOffBrightnessEvaluator(*this);
    }
    uint16_t Period() const override { return period_; }
    uint8_t Eval(uint32_t t) const override {
        return fadeon_func(period_ - t, period_);
    }
};

// The breathe func is composed by fadein and fade-out with one each half
// period.  we approximate the following function:
//   y(x) = exp(sin((t-period/4.) * 2. * PI / period)) - 0.36787944) *  108.)
// idea see:
//   http://sean.voisen.org/blog/2011/10/breathing-led-with-arduino/
// But we do it with integers only.
class BreatheBrightnessEvaluator : public BrightnessEvaluator {
    uint16_t period_;
    using BrightnessEvaluator::BrightnessEvaluator;

 public:
    BreatheBrightnessEvaluator() = delete;
    explicit BreatheBrightnessEvaluator(uint16_t period) : period_(period) {}
    BrightnessEvaluator* clone(void* ptr) const override {
        return new (ptr) BreatheBrightnessEvaluator(*this);
    }
    uint16_t Period() const override { return period_; }
    uint8_t Eval(uint32_t t) const override {
        if (t + 1 >= period_) return kZeroBrightness;
        const decltype(period_) periodh = period_ >> 1;
        return t < periodh ? fadeon_func(t, periodh)
                           : fadeon_func(period_ - t, periodh);
    }
};

// set MAX_SIZE to class occupying most memory
constexpr auto MAX_SIZE = sizeof(BlinkBrightnessEvaluator);

template <typename HalType, typename B>
class TJLed {
#ifdef HAS_TYPE_TRAITS
    static_assert(std::is_base_of<JLedHal, HalType>::value,
                  "HalType must be of type JLedHal");
#endif

 protected:
    // pointer to a (user defined) brightness evaluator.
    BrightnessEvaluator* brightness_eval_ = nullptr;
    // Hardware abstraction giving access to the MCU
    HalType hal_;

    void Write(uint8_t val) {
        hal_.analogWrite(IsLowActive() ? kFullBrightness - val : val);
    }

    B& SetFlags(uint8_t f, bool val) {
        if (val) {
            flags_ |= f;
        } else {
            flags_ &= ~f;
        }
        return static_cast<B&>(*this);
    }
    bool GetFlag(uint8_t f) const { return (flags_ & f) != 0; }

    void SetInDelayAfterPhase(bool f) { SetFlags(FL_IN_DELAY_AFTER_PHASE, f); }
    bool IsInDelayAfterPhase() const {
        return GetFlag(FL_IN_DELAY_AFTER_PHASE);
    }

 public:
    TJLed() = delete;
    explicit TJLed(const HalType& hal) : hal_{hal} {}
    explicit TJLed(uint8_t pin) : hal_{HalType{pin}} {}
    TJLed(const TJLed<HalType, B>& rLed) { *this = rLed; }

    B& operator=(const TJLed<HalType, B>& rLed) {
        flags_ = rLed.flags_;
        num_repetitions_ = rLed.num_repetitions_;
        last_update_time_ = rLed.last_update_time_;
        delay_before_ = rLed.delay_before_;
        delay_after_ = rLed.delay_after_;
        time_start_ = rLed.time_start_;
        hal_ = rLed.hal_;

        if (rLed.brightness_eval_ !=
            reinterpret_cast<const BrightnessEvaluator*>(
                rLed.brightness_eval_buf_)) {  // NOLINT
            // points to user provided evaluator
            brightness_eval_ = rLed.brightness_eval_;
        } else {
            brightness_eval_ = (reinterpret_cast<const BrightnessEvaluator*>(
                                    rLed.brightness_eval_))
                                   ->clone(brightness_eval_buf_);
        }
        return static_cast<B&>(*this);
    }

    HalType& Hal() { return hal_; }

    bool Update() { return Update(hal_.millis()); }

    // turn LED on
    B& On(uint8_t brightness = kFullBrightness) {
        // we use placement new and therefore not need to keep track of mem
        // allocated
        brightness_eval_ =
            new (brightness_eval_buf_) ConstantBrightnessEvaluator(brightness);
        return static_cast<B&>(*this);
    }

    // Set physical LED polarity to be low active. This inverts every
    // signal physically output to a pin.
    B& LowActive() { return SetFlags(FL_LOW_ACTIVE, true); }
    bool IsLowActive() const { return GetFlag(FL_LOW_ACTIVE); }

    // turn LED off
    B& Off() {
        brightness_eval_ = new (brightness_eval_buf_)
            ConstantBrightnessEvaluator(kZeroBrightness);
        return static_cast<B&>(*this);
    }

    // turn LED on or off, calls On() / Off()
    B& Set(bool on) { return on ? On() : Off(); }

    // Fade LED on
    B& FadeOn(uint16_t duration) {
        brightness_eval_ =
            new (brightness_eval_buf_) FadeOnBrightnessEvaluator(duration);
        return static_cast<B&>(*this);
    }

    // Fade LED off - acutally is just inverted version of FadeOn()
    B& FadeOff(uint16_t duration) {
        brightness_eval_ =
            new (brightness_eval_buf_) FadeOffBrightnessEvaluator(duration);
        return static_cast<B&>(*this);
    }

    // Set effect to Breathe, with the given period time in ms.
    B& Breathe(uint16_t period) {
        brightness_eval_ =
            new (brightness_eval_buf_) BreatheBrightnessEvaluator(period);
        return static_cast<B&>(*this);
    }

    // Set effect to Blink, with the given on- and off- duration values.
    B& Blink(uint16_t duration_on, uint16_t duration_off) {
        brightness_eval_ = new (brightness_eval_buf_)
            BlinkBrightnessEvaluator(duration_on, duration_off);
        return static_cast<B&>(*this);
    }

    // Use a user provided brightness evaluator.
    B& UserFunc(BrightnessEvaluator* ube) {
        brightness_eval_ = ube;
        return static_cast<B&>(*this);
    }

    // set number of repetitions for effect.
    B& Repeat(uint16_t num_repetitions) {
        num_repetitions_ = num_repetitions;
        return static_cast<B&>(*this);
    }

    // repeat Forever
    B& Forever() { return Repeat(kRepeatForever); }
    bool IsForever() const { return num_repetitions_ == kRepeatForever; }

    // Set amount of time to initially wait before effect starts. Time
    // is
    // relative to first call of Update() method and specified in ms.
    B& DelayBefore(uint16_t delay_before) {
        delay_before_ = delay_before;
        return static_cast<B&>(*this);
    }

    // Set amount of time to wait in ms after each iteration.
    B& DelayAfter(uint16_t delay_after) {
        delay_after_ = delay_after;
        return static_cast<B&>(*this);
    }

    // Stop current effect and turn LED immeadiately off. Further calls to
    // Update() will have no effect.
    B& Stop() {
        Write(kZeroBrightness);
        return SetFlags(FL_STOPPED, true);
    }
    bool IsRunning() const { return !GetFlag(FL_STOPPED); }

    // Reset to inital state
    B& Reset() {
        time_start_ = kTimeUndef;
        last_update_time_ = kTimeUndef;
        return SetFlags(FL_STOPPED | FL_IN_DELAY_AFTER_PHASE, false);
    }

 protected:
    // update brightness of LED using the given brightness evaluator
    //  (brightness)                       ________________
    // on 255 |                         ¸-'
    //        |                      ¸-'
    //        |                   ¸-'
    // off 0  |________________¸-'
    //        |<-delay before->|<--period-->|<-delay after-> (time)
    //                         | func(t)    |
    //                         |<- num_repetitions times  ->
    bool Update(uint32_t now) {
        if (!IsRunning() || !brightness_eval_) return false;

        // no need to process updates twice during one time tick.
        if (last_update_time_ == now) {
            return true;
        }

        if (last_update_time_ == kTimeUndef) {
            last_update_time_ = now;
            time_start_ = now + delay_before_;
            SetInDelayAfterPhase(false);
        }
        last_update_time_ = now;

        if (now < time_start_) return true;

        // t cycles in range [0..period+delay_after-1]
        const auto period = brightness_eval_->Period();
        const auto t = (now - time_start_) % (period + delay_after_);

        if (t < period) {
            Write(brightness_eval_->Eval(t));
        } else if (!IsInDelayAfterPhase()) {
            // when in delay after phase, just call Write()
            // once at the beginning.
            SetInDelayAfterPhase(true);
            Write(brightness_eval_->Eval(period - 1));
        }

        if (IsForever()) return true;

        const auto time_end =
            time_start_ + (uint32_t)(period + delay_after_) * num_repetitions_ -
            1;

        if (now >= time_end) {
            // make sure final value of t = (period-1) is set
            Write(brightness_eval_->Eval(period - 1));
            SetFlags(FL_STOPPED, true);
            return false;
        }
        return true;
    }

 private:
    static constexpr uint8_t FL_IN_DELAY_AFTER_PHASE = (1 << 0);
    static constexpr uint8_t FL_STOPPED = (1 << 1);
    static constexpr uint8_t FL_LOW_ACTIVE = (1 << 3);
    // this is where the BrightnessEvaluator object will be stored using
    // placment new.
    char brightness_eval_buf_[MAX_SIZE];

    static constexpr uint16_t kRepeatForever = 65535;
    static constexpr uint32_t kTimeUndef = -1;
    uint8_t flags_ = 0;
    uint16_t num_repetitions_ = 1;
    uint32_t last_update_time_ = kTimeUndef;
    uint16_t delay_before_ = 0;  // delay before the first effect starts
    uint16_t delay_after_ = 0;   // delay after each repetition
    uint32_t time_start_ = kTimeUndef;
};

// a group of JLed objects which can be controlled simultanously, in parallel
// or sequentially.
template <typename T>
class TJLedSequence {
 protected:
    // update all leds parallel. Returns true while any of the JLeds is
    // active, else false
    bool UpdateParallel() {
        auto result = false;
        for (auto i = 0u; i < n_; i++) {
            result |= leds_[i].Update();
        }
        return result;
    }

    // update all leds sequentially. Returns true while any of the JLeds is
    // active, else false
    bool UpdateSequentially() {
        if (cur_ >= n_) {
            return false;
        }
        if (!leds_[cur_].Update()) {
            cur_++;
        }
        return true;
    }

 public:
    enum eMode { SEQUENCE, PARALLEL };
    TJLedSequence() = delete;

    template<size_t N>
    TJLedSequence(eMode mode, T (&leds)[N]) : TJLedSequence(mode, leds, N) {}

    TJLedSequence(eMode mode, T* leds, size_t n)
        : mode_{mode}, leds_{leds}, cur_{0}, n_{n} {}

    bool Update() {
        return mode_ == eMode::PARALLEL ? UpdateParallel()
                                        : UpdateSequentially();
    }

    void Reset() {
        for (auto i = 0u; i < n_; i++) {
            leds_[i].Reset();
        }
        cur_ = 0;
    }

    void Stop() {
        for (auto i = 0u; i < n_; i++) {
            leds_[i].Stop();
        }
    }

 private:
    const eMode mode_;
    T* leds_;
    size_t cur_;
    const size_t n_;
};

};  // namespace jled

#endif  // SRC_JLED_BASE_H_
