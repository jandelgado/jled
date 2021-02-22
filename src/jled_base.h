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

static constexpr uint8_t kFullBrightness = 255;
static constexpr uint8_t kZeroBrightness = 0;

uint8_t fadeon_func(uint32_t t, uint16_t period);
uint8_t rand8();
void rand_seed(uint32_t s);
uint8_t scale5(uint8_t val, uint8_t factor);

// a function f(t,period,param) that calculates the LEDs brightness for a given
// point in time and the given period. param is an optionally user provided
// parameter. t will always be in range [0..period-1].
// f(period-1,period,param) will be called last to calculate the final state of
// the LED.
class BrightnessEvaluator {
 public:
    virtual uint16_t Period() const = 0;
    virtual uint8_t Eval(uint32_t t) const = 0;
};

class CloneableBrightnessEvaluator : public BrightnessEvaluator {
 public:
    virtual BrightnessEvaluator* clone(void* ptr) const = 0;
    static void* operator new(size_t, void* ptr) { return ptr; }
    static void operator delete(void*) {}
};

class ConstantBrightnessEvaluator : public CloneableBrightnessEvaluator {
    uint8_t val_;

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
class BlinkBrightnessEvaluator : public CloneableBrightnessEvaluator {
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
class FadeOnBrightnessEvaluator : public CloneableBrightnessEvaluator {
    uint16_t period_;

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
class FadeOffBrightnessEvaluator : public CloneableBrightnessEvaluator {
    uint16_t period_;

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
class BreatheBrightnessEvaluator : public CloneableBrightnessEvaluator {
    uint16_t period_;

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

class CandleBrightnessEvaluator : public CloneableBrightnessEvaluator {
    uint8_t speed_;
    uint8_t jitter_;
    uint16_t period_;
    mutable uint8_t last_ = 5;
    mutable uint32_t last_t_ = 0;

 public:
    CandleBrightnessEvaluator() = delete;

    // speed - speed of effect (0..15). 0 fastest. Each increment by 1
    //         halfes the speed.
    // jitter - amount of jittering to apply. 0 - no jitter, 15 - candle,
    //                                        64 - fire, 255 - storm
    CandleBrightnessEvaluator(uint8_t speed, uint8_t jitter, uint16_t period)
        : speed_(speed), jitter_(jitter), period_(period) {}

    BrightnessEvaluator* clone(void* ptr) const override {
        return new (ptr) CandleBrightnessEvaluator(*this);
    }

    uint16_t Period() const override { return period_; }
    uint8_t Eval(uint32_t t) const override {
        // idea from
        // https://cpldcpu.wordpress.com/2013/12/08/hacking-a-candleflicker-led/
        // TODO(jd) finetune values
        static constexpr uint8_t kCandleTable[] = {
            5, 10, 20, 30, 50, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 255};
        if ((t >> speed_) == last_t_) return last_;
        last_t_ = (t >> speed_);
        const auto rnd = rand8() & 255;
        last_ = (rnd >= jitter_) ? 255 : (50 + kCandleTable[rnd & 0xf]);
        return last_;
    }
};

template <typename HalType, typename B>
class TJLed {
 protected:
    // pointer to a (user defined) brightness evaluator.
    BrightnessEvaluator* brightness_eval_ = nullptr;
    // Hardware abstraction giving access to the MCU
    HalType hal_;

    void Write(uint8_t val) {
        val = scale5(val, maxBrightness_);
        hal_.analogWrite(IsLowActive() ? kFullBrightness - val : val);
    }

 public:
    TJLed() = delete;
    explicit TJLed(const HalType& hal)
        : hal_{hal},
          state_{ST_RUNNING},
          bLowActive_{false},
          maxBrightness_{(1 << kBitsBrightness) - 1} {}

    explicit TJLed(typename HalType::PinType pin) : TJLed{HalType{pin}} {}

    TJLed(const TJLed& rLed) : hal_{rLed.hal_} { *this = rLed; }

    B& operator=(const TJLed<HalType, B>& rLed) {
        state_ = rLed.state_;
        bLowActive_ = rLed.bLowActive_;
        maxBrightness_ = rLed.maxBrightness_;
        num_repetitions_ = rLed.num_repetitions_;
        last_update_time_ = rLed.last_update_time_;
        delay_before_ = rLed.delay_before_;
        delay_after_ = rLed.delay_after_;
        time_start_ = rLed.time_start_;
        hal_ = rLed.hal_;

        if (rLed.brightness_eval_ !=
            reinterpret_cast<const BrightnessEvaluator*>(
                rLed.brightness_eval_buf_)) {
            // nullptr or points to (external) user provided evaluator
            brightness_eval_ = rLed.brightness_eval_;
        } else {
            brightness_eval_ =
                (reinterpret_cast<const CloneableBrightnessEvaluator*>(
                     rLed.brightness_eval_))
                    ->clone(brightness_eval_buf_);
        }
        return static_cast<B&>(*this);
    }

    HalType& Hal() { return hal_; }

    bool Update() { return Update(hal_.millis()); }

    // Set physical LED polarity to be low active. This inverts every
    // signal physically output to a pin.
    B& LowActive() {
        bLowActive_ = true;
        return static_cast<B&>(*this);
    }

    bool IsLowActive() const { return bLowActive_; }

    // turn LED on
    B& On() { return Set(kFullBrightness); }

    // turn LED off
    B& Off() { return Set(kZeroBrightness); }

    // Sets LED to given brightness
    B& Set(uint8_t brightness) {
        // note: we use placement new and therefore not need to keep track of
        // mem allocated
        return SetBrightnessEval(new (brightness_eval_buf_)
                                     ConstantBrightnessEvaluator(brightness));
    }

    // Fade LED on
    B& FadeOn(uint16_t duration) {
        return SetBrightnessEval(new (brightness_eval_buf_)
                                     FadeOnBrightnessEvaluator(duration));
    }

    // Fade LED off - acutally is just inverted version of FadeOn()
    B& FadeOff(uint16_t duration) {
        return SetBrightnessEval(new (brightness_eval_buf_)
                                     FadeOffBrightnessEvaluator(duration));
    }

    // Set effect to Breathe, with the given period time in ms.
    B& Breathe(uint16_t period) {
        return SetBrightnessEval(new (brightness_eval_buf_)
                                     BreatheBrightnessEvaluator(period));
    }

    // Set effect to Blink, with the given on- and off- duration values.
    B& Blink(uint16_t duration_on, uint16_t duration_off) {
        return SetBrightnessEval(
            new (brightness_eval_buf_)
                BlinkBrightnessEvaluator(duration_on, duration_off));
    }

    // Set effect to Candle light simulation
    B& Candle(uint8_t speed = 6, uint8_t jitter = 15,
              uint16_t period = 0xffff) {
        return SetBrightnessEval(
            new (brightness_eval_buf_)
                CandleBrightnessEvaluator(speed, jitter, period));
    }

    // Use a user provided brightness evaluator.
    B& UserFunc(BrightnessEvaluator* user_eval) {
        return SetBrightnessEval(user_eval);
    }

    // set number of repetitions for effect.
    B& Repeat(uint16_t num_repetitions) {
        num_repetitions_ = num_repetitions;
        return static_cast<B&>(*this);
    }

    // repeat Forever
    B& Forever() { return Repeat(kRepeatForever); }
    bool IsForever() const { return num_repetitions_ == kRepeatForever; }

    // Set amount of time to initially wait before effect starts. Time is
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
        state_ = ST_STOPPED;
        return static_cast<B&>(*this);
    }

    bool IsRunning() const { return state_ != ST_STOPPED; }

    // Reset to inital state
    B& Reset() {
        time_start_ = kTimeUndef;
        last_update_time_ = kTimeUndef;
        state_ = ST_RUNNING;
        return static_cast<B&>(*this);
    }

    // Sets the maximum brightness level. 255 is full brightness, 0 turns the
    // effect off. Currently, only upper 5 bits of the provided value are used
    // and stored.
    B& MaxBrightness(uint8_t level) {
        maxBrightness_ = level >> (8 - kBitsBrightness);
        return static_cast<B&>(*this);
    }

    // Returns current maximum brightness level. Since currently only upper 5
    // bits are used, lower 3 bits will always be 0.
    uint8_t MaxBrightness() const {
        return maxBrightness_ << (8 - kBitsBrightness);
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
        if (state_ == ST_STOPPED || !brightness_eval_) return false;

        // no need to process updates twice during one time tick.
        if (last_update_time_ == now) return true;

        if (last_update_time_ == kTimeUndef) {
            time_start_ = now + delay_before_;
        }
        last_update_time_ = now;

        if (now < time_start_) return true;

        // t cycles in range [0..period+delay_after-1]
        const auto period = brightness_eval_->Period();
        const auto t = (now - time_start_) % (period + delay_after_);

        if (t < period) {
            state_ = ST_RUNNING;
            Write(brightness_eval_->Eval(t));
        } else {
            if (state_ == ST_RUNNING) {
                // when in delay after phase, just call Write()
                // once at the beginning.
                state_ = ST_IN_DELAY_AFTER_PHASE;
                Write(brightness_eval_->Eval(period - 1));
            }
        }

        if (IsForever()) return true;

        const auto time_end =
            time_start_ + (uint32_t)(period + delay_after_) * num_repetitions_ -
            1;

        if (now >= time_end) {
            // make sure final value of t = (period-1) is set
            state_ = ST_STOPPED;
            Write(brightness_eval_->Eval(period - 1));
            return false;
        }
        return true;
    }

    B& SetBrightnessEval(BrightnessEvaluator* be) {
        brightness_eval_ = be;
        // start over after the brightness evaluator changed
        return Reset();
    }

 private:
    static constexpr uint8_t ST_STOPPED = 0;
    static constexpr uint8_t ST_RUNNING = 1;
    static constexpr uint8_t ST_IN_DELAY_AFTER_PHASE = 2;

    uint8_t state_ : 2;
    uint8_t bLowActive_ : 1;

    // Number of bits used to control brightness with MaxBrightness(). Using
    // only 5 bits here saves us a byte, since summing up with previous defs.
 public:
    static constexpr uint8_t kBitsBrightness = 5;
    static constexpr uint8_t kBrightnessStep = 1 << (8 - kBitsBrightness);

 private:
    uint8_t maxBrightness_ : kBitsBrightness;

    // this is where the BrightnessEvaluator object will be stored using
    // placment new.  Set MAX_SIZE to class occupying most memory
    static constexpr auto MAX_SIZE = sizeof(CandleBrightnessEvaluator);
    alignas(alignof(
        CloneableBrightnessEvaluator)) char brightness_eval_buf_[MAX_SIZE];

    static constexpr uint16_t kRepeatForever = 65535;
    uint16_t num_repetitions_ = 1;

    static constexpr uint32_t kTimeUndef = -1;
    uint32_t last_update_time_ = kTimeUndef;
    uint32_t time_start_ = kTimeUndef;

    uint16_t delay_before_ = 0;  // delay before the first effect starts
    uint16_t delay_after_ = 0;   // delay after each repetition
};

template<typename T> T* ptr(T &obj) {return &obj;}  // NOLINT
template<typename T> T* ptr(T *obj) {return obj;}

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
            result |= ptr(leds_[i])->Update();
        }
        return result;
    }

    // update all leds sequentially. Returns true while any of the JLeds is
    // active, else false
    bool UpdateSequentially() {
        if (cur_ >= n_) {
            return false;
        }
        if (!ptr(leds_[cur_])->Update()) {
            return ++cur_ < n_;
        }
        return true;;
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
    TJLedSequence(eMode mode, T (&leds)[N]) : TJLedSequence(mode, leds, N) {}

    TJLedSequence(eMode mode, T* leds, size_t n)
        : mode_{mode}, leds_{leds}, cur_{0}, n_{n} {}

    bool Update() {
        if (!is_running_) {
            return false;
        }

        const auto led_running = (mode_ == eMode::PARALLEL) ? UpdateParallel()
                                                  : UpdateSequentially();
        if (led_running) {
            return true;
        }

        // start next iteration of sequence
        cur_ = 0;
        ResetLeds();

        is_running_ = ++iteration_ < num_repetitions_  ||
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
        for (auto i = 0u; i < n_; i++) {
            leds_[i].Stop();
        }
    }

    // set number of repetitions for the sequence
    TJLedSequence<T> Repeat(uint16_t num_repetitions) {
        num_repetitions_ = num_repetitions;
        return *this;
    }

    // repeat Forever
    TJLedSequence<T> Forever() { return Repeat(kRepeatForever); }
    bool IsForever() const { return num_repetitions_ == kRepeatForever; }

 private:
    const eMode mode_;
    T* leds_;
    size_t cur_;
    const size_t n_;
    static constexpr uint16_t kRepeatForever = 65535;
    uint16_t num_repetitions_ = 1;
    uint16_t iteration_ = 0;
    bool is_running_ = true;
};

};  // namespace jled

#endif  // SRC_JLED_BASE_H_
