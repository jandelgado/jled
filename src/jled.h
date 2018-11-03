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
#ifndef SRC_JLED_H_
#define SRC_JLED_H_

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

#ifdef ESP32
#include "esp32_hal.h"  // NOLINT
#elif ESP8266
#include "esp8266_hal.h"  // NOLINT
#else
#include "arduino_hal.h"  // NOLINT
#endif

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
    virtual uint8_t Eval(uint32_t t) = 0;
    // placement new used to avoid dynamic memory allocations
    void* operator new(size_t size, void* ptr);
};

class ConstantBrightnessEvaluator : public BrightnessEvaluator {
    uint8_t val_;
    using BrightnessEvaluator::BrightnessEvaluator;

 public:
    explicit ConstantBrightnessEvaluator(uint8_t val) : val_(val) {}
    uint16_t Period() const override { return 1; }
    uint8_t Eval(uint32_t) override { return val_; }
};

// BlinkBrightnessEvaluator does one on-off cycle in the specified period
class BlinkBrightnessEvaluator : public BrightnessEvaluator {
    uint16_t duration_on_, duration_off_;

 public:
    BlinkBrightnessEvaluator() : duration_on_(0), duration_off_(0) {}
    BlinkBrightnessEvaluator(uint16_t duration_on, uint16_t duration_off)
        : duration_on_(duration_on), duration_off_(duration_off) {}
    uint16_t Period() const override { return duration_on_ + duration_off_; }
    uint8_t Eval(uint32_t t) override {
        return (t < duration_on_) ? kFullBrightness : kZeroBrightness;
    }
};

// fade LED on
class FadeOnBrightnessEvaluator : public BrightnessEvaluator {
    uint16_t period_;
    using BrightnessEvaluator::BrightnessEvaluator;

 public:
    explicit FadeOnBrightnessEvaluator(uint16_t period) : period_(period) {}
    uint16_t Period() const override { return period_; }
    uint8_t Eval(uint32_t t) override { return fadeon_func(t, period_); }
};

// fade LED off
class FadeOffBrightnessEvaluator : public BrightnessEvaluator {
    uint16_t period_;
    using BrightnessEvaluator::BrightnessEvaluator;

 public:
    explicit FadeOffBrightnessEvaluator(uint16_t period) : period_(period) {}
    uint16_t Period() const override { return period_; }
    uint8_t Eval(uint32_t t) override {
        return fadeon_func(period_ - t, period_);
    }
};

// The breathe func is composed by fadein and fade-out with one each
// half
// period.  we approximate the following function:
//   y(x) = exp(sin((t-period/4.) * 2. * PI / period)) - 0.36787944) *
//   108.)
// idea see:
// http://sean.voisen.org/blog/2011/10/breathing-led-with-arduino/
// But we do it with integers only.
class BreatheBrightnessEvaluator : public BrightnessEvaluator {
    uint16_t period_;
    using BrightnessEvaluator::BrightnessEvaluator;

 public:
    explicit BreatheBrightnessEvaluator(uint16_t period) : period_(period) {}
    uint16_t Period() const override { return period_; }
    uint8_t Eval(uint32_t t) override {
        if (t + 1 >= period_) return kZeroBrightness;
        const uint16_t periodh = period_ >> 1;
        return t < periodh ? fadeon_func(t, periodh)
                           : fadeon_func(period_ - t, periodh);
    }
};

// set MAX_SIZE to class occupying most memory
constexpr auto MAX_SIZE = sizeof(BlinkBrightnessEvaluator);

template <typename HalType, typename B>
class TJLedController {
 public:
    // TODO(jd) needed?
    // TJLedController(const TJLedController<HalType, B> &rLed) = default;

    // update brightness of LED using the given brightness evaluator
    //  (brightness)                     _________________
    // on 255 |                       ¸-'
    //        |                    ¸-'
    //        |                 ¸-'
    // off 0  |______________¸-'
    //        |<delay before>|<--period-->|<-delay after-> (time)
    //                       | func(t)    |
    //                       |<- num_repetitions times  ->
    bool Update(HalType* hal, BrightnessEvaluator* eval) {
        if (!IsRunning() || !eval) {
            return false;
        }
        const auto now = hal->millis();

        // no need to process updates twice during one time tick.
        if (last_update_time_ == now) {
            return true;
        }

        // last_update_time_ will be 0 on initialization, so this fails
        // on first call to this method.
        if (last_update_time_ == kTimeUndef) {
            last_update_time_ = now;
            time_start_ = now + delay_before_;
        }
        last_update_time_ = now;

        if (now < time_start_) {
            return true;
        }

        // t cycles in range [0..period+delay_after-1]
        const auto period = eval->Period();
        const auto t = (now - time_start_) % (period + delay_after_);

        if (t < period) {
            SetInDelayAfterPhase(false);
            AnalogWrite(hal, EvalBrightness(eval, t));
        } else {
            if (!IsInDelayAfterPhase()) {
                // when in delay after phase, just call AnalogWrite()
                // once at the beginning.
                SetInDelayAfterPhase(true);
                AnalogWrite(hal, EvalBrightness(eval, period - 1));
            }
        }

        if (!IsForever()) {
            const auto time_end =
                time_start_ +
                (uint32_t)(period + delay_after_) * num_repetitions_ - 1;

            if (now >= time_end) {
                // make sure final value of t = period-1 is set
                AnalogWrite(hal, EvalBrightness(eval, period - 1));
                SetFlags(FL_STOPPED, true);
                return false;
            }
        }
        return true;
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

    // Set physical LED polarity to be low active. This inverts every
    // signal physically output to a pin.
    B& LowActive() { return SetFlags(FL_LOW_ACTIVE, true); }
    bool IsLowActive() const { return GetFlag(FL_LOW_ACTIVE); }

    // Stop current effect and turn LED immeadiately off
    void Stop(HalType* hal) {
        // Immediately turn LED off and stop effect.
        SetFlags(FL_STOPPED, true);
        AnalogWrite(hal, kZeroBrightness);
    }
    bool IsRunning() const { return !GetFlag(FL_STOPPED); }

    // Reset to inital state
    void Reset() {
        time_start_ = kTimeUndef;
        last_update_time_ = kTimeUndef;
        SetFlags(FL_STOPPED | FL_IN_DELAY_AFTER_PHASE, false);
    }

 protected:
    // internal control of the LED, does not affect
    // state and honors low_active_ flag
    void AnalogWrite(HalType* hal, uint8_t val) const {
        hal->analogWrite(IsLowActive() ? kFullBrightness - val : val);
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

    uint8_t EvalBrightness(BrightnessEvaluator* eval, uint32_t t) const {
        return eval->Eval(t);
    }

 private:
    static constexpr uint16_t kRepeatForever = 65535;
    static constexpr uint32_t kTimeUndef = -1;
    static constexpr uint8_t FL_LOW_ACTIVE = (1 << 0);
    static constexpr uint8_t FL_IN_DELAY_AFTER_PHASE = (1 << 1);
    static constexpr uint8_t FL_STOPPED = (1 << 2);
    uint8_t flags_ = 0;
    uint16_t num_repetitions_ = 1;
    uint32_t last_update_time_ = kTimeUndef;
    uint16_t delay_before_ = 0;  // delay before the first effect starts
    uint16_t delay_after_ = 0;   // delay after each repetition
    uint32_t time_start_ = kTimeUndef;
};

template <typename B, typename HalType>
class TJLed : public TJLedController<HalType, B> {
    using TJLedController<HalType, B>::TJLedController;

    // this is where the BrightnessEvaluator object will be stored using
    // placment new.
    char brightness_eval_buf_[MAX_SIZE];

 protected:
    // optional pointer to a user defined brightness evaluator.
    BrightnessEvaluator* brightness_eval_ = nullptr;
    // Hardware abstraction giving access to the MCU
    HalType hal_;

 public:
    TJLed() = delete;
    explicit TJLed(const HalType& hal) : hal_(hal) {}
    explicit TJLed(uint8_t pin) : hal_(HalType(pin)) {}
    TJLed(const TJLed<B, HalType>& rLed) {
        *this = rLed;
    }

    HalType& Hal() {return hal_;}

    B& operator=(const TJLed<B, HalType>& rLed) {
        TJLedController<HalType, B>::operator=(rLed);
        if (rLed.brightness_eval_ !=
            reinterpret_cast<const BrightnessEvaluator*>(rLed.brightness_eval_buf_)) {  // NOLINT
            brightness_eval_ = rLed.brightness_eval_;
        } else {
            memcpy(brightness_eval_buf_, rLed.brightness_eval_buf_, MAX_SIZE);
            brightness_eval_ =
                reinterpret_cast<BrightnessEvaluator*>(brightness_eval_buf_);
        }
        hal_ = rLed.hal_;
        return static_cast<B&>(*this);
    }

    bool Update() {
        return TJLedController<HalType, B>::Update(&hal_, brightness_eval_);
    }

    void Stop() { return TJLedController<HalType, B>::Stop(&hal_); }

    // turn LED on
    B& On(uint8_t brightness = kFullBrightness) {
        // we use placement new and therefore not need to keep track of mem
        // allocated
        brightness_eval_ =
            new (brightness_eval_buf_) ConstantBrightnessEvaluator(brightness);
        return static_cast<B&>(*this);
    }

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
};

#ifdef ESP32
using JLedHalType = Esp32Hal;
#elif ESP8266
using JLedHalType = Esp8266Hal;
#else
using JLedHalType = ArduinoHal;
#endif

class JLed : public TJLed<JLed, JLedHalType> {
    using TJLed<JLed, JLedHalType>::TJLed;
};

// a group of JLed objects which can be controlled simultanously
class JLedSequence {
 public:
    JLedSequence() = delete;
    JLedSequence(JLed* items, size_t n) : items_(items), n_(n) {}

    bool Update() {
        if (cur_ >= n_) {
            return false;
        }
        auto& led = items_[cur_];
        if (!led.Update()) {
            cur_++;
        }
        return true;
    }

 private:
    JLed* items_;
    size_t cur_ = 0;
    size_t n_;
};

};  // namespace jled

using JLed = jled::JLed;
using JLedSequence = jled::JLedSequence;

#endif  // SRC_JLED_H_
