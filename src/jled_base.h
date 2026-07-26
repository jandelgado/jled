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

#include "jled_effects.h"  // brightness evaluators and effect helper functions
#include "jled_events.h"   // Event, EventSet, UpdateResult

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

// Non-template tag base, allows JLedAny to detect TJLed subclasses via
// std::is_base_of without requiring a common virtual interface.
class JLedBase {};

enum class eIdleMode { TO_MIN_BRIGHTNESS = 0, FULL_OFF, KEEP_CURRENT };

template<typename Hal, typename Clock, typename Brightness, typename Derived>
class TJLed : public JLedBase {
 protected:
    // Active brightness evaluator (discriminated union).
    EvalStorage<Brightness> eval_storage_;
    // Hardware abstraction giving access to the MCU
    Hal hal_;

    // Evaluate effect(t), assumes eval_storage_.IsSet().
    Brightness Eval(uint32_t t) const { return eval_storage_.Eval(t); }

 public:
    using brightness_t = Brightness;

    TJLed() = delete;
    explicit TJLed(const Hal& hal)
        : hal_{hal},
          state_{ST_INIT},
          bLowActive_{false},
          bPaused_{false},
          first_cycle_{false},
          done_{false},
          minBrightness_{BrightnessTraits<Brightness>::kZeroBrightness},
          maxBrightness_{BrightnessTraits<Brightness>::kFullBrightness} {}

    explicit TJLed(typename Hal::PinType pin)
        : hal_{pin},
          state_{ST_INIT},
          bLowActive_{false},
          bPaused_{false},
          first_cycle_{false},
          done_{false},
          minBrightness_{BrightnessTraits<Brightness>::kZeroBrightness},
          maxBrightness_{BrightnessTraits<Brightness>::kFullBrightness} {}

    TJLed(const TJLed& rLed) : hal_{rLed.hal_} { *this = rLed; }

    Derived& operator=(const TJLed<Hal, Clock, Brightness, Derived>& rLed) {
        state_ = rLed.state_;
        bLowActive_ = rLed.bLowActive_;
        bPaused_ = rLed.bPaused_;
        first_cycle_ = rLed.first_cycle_;
        done_ = rLed.done_;
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
    Derived& Breathe(uint16_t duration_fade_on, uint16_t duration_on, uint16_t duration_fade_off) {
        eval_storage_.type = EvalType::BREATHE;
        eval_storage_.data.breathe = {duration_fade_on,
                                      duration_on,
                                      duration_fade_off,
                                      BrightnessTraits<Brightness>::kZeroBrightness,
                                      BrightnessTraits<Brightness>::kFullBrightness};
        return Reset();
    }

    // Set effect to Blink, with the given on- and off- duration values.
    Derived& Blink(uint16_t duration_on, uint16_t duration_off, uint8_t n = 1) {
        eval_storage_.type = EvalType::BLINK;
        eval_storage_.data.blink = {duration_on, duration_off, n};
        return Reset();
    }

    // Set effect to Candle light simulation.
    // When offset is omitted, the address of this JLed instance is used as a
    // random offset so multiple LEDs with default parameters automatically
    // flicker independently. Pass an explicit offset (in ms) to control the
    // phase precisely.
    Derived& Candle(uint8_t speed = 6, uint8_t jitter = 15, uint16_t period = 0xffff,
                    uint16_t offset = kCandleOffsetAuto) {
        eval_storage_.type = EvalType::CANDLE;
        const uint16_t actual_offset =
            (offset == kCandleOffsetAuto) ? static_cast<uint16_t>(reinterpret_cast<uintptr_t>(this))
                                          : offset;
        eval_storage_.data.candle =
            CandleBrightnessEvaluator<Brightness>(speed, jitter, period, actual_offset);
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
    Derived& Stop(eIdleMode mode = eIdleMode::TO_MIN_BRIGHTNESS) {
        if (mode != eIdleMode::KEEP_CURRENT) {
            WriteRaw(mode == eIdleMode::FULL_OFF ? BrightnessTraits<Brightness>::kZeroBrightness
                                                 : minBrightness_);
        }
        state_ = ST_STOPPED;
        bPaused_ = false;
        return static_cast<Derived&>(*this);
    }

    bool IsRunning() const { return state_ != ST_STOPPED; }

    void Pause(eIdleMode mode = eIdleMode::TO_MIN_BRIGHTNESS) { Pause(Clock::millis(), mode); }
    void Resume() { Resume(Clock::millis()); }

    bool IsPaused() const { return bPaused_; }

    // Reset to inital state
    Derived& Reset() {
        time_start_ = 0;
        last_update_time_ = 0;
        bPaused_ = false;
        first_cycle_ = false;
        done_ = false;
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

    // Write val directly out to the hardware, inverting signal when
    // active-low is set. Bypasses the effect state machine entirely, e.g.
    // for forcing an output from a lifecycle callback (see UpdateResult).
    Derived& WriteRaw(Brightness val) {
        constexpr auto kFullBright = BrightnessTraits<Brightness>::kFullBrightness;
        hal_.template analogWrite<Brightness>(IsLowActive() ? kFullBright - val : val);
        return static_cast<Derived&>(*this);
    }

    // update brightness of LED using the given brightness evaluator and the
    // current time. Returns an UpdateResult carrying whether the effect is
    // still running, which lifecycle events fired this tick, and the
    // brightness value (if any) written to the output this tick (the
    // calculated value after min- and max-brightness scaling was applied).
    //
    //  (brightness)                       ________________
    // on 255 |                         ¸-'
    //        |                      ¸-'
    //        |                   ¸-'
    // off 0  |________________¸-'
    //        |<-delay before->|<--period-->|<-delay after-> (time)
    //                         | func(t)    |
    //                         |<- num_repetitions times  ->
    //
    // Where the UpdateResult events (see jled_events.h) fire on this
    // timeline, shown for two repetitions with a configured delay_after:
    //
    //                              ____________|        ________________
    //                        //////            |  //////
    //        ________________                  |__
    //        |<--before-->|<-period->|<-after->|<-period->|<--after--->|
    //        A            B          C         D          C            E
    //
    //   A  kStart            fires once, on the very first Update() call
    //   B  kRepeatStart,      first repetition begins; kFirstOutput fires
    //      kFirstOutput       only here, on the first output tick of the run
    //   C  kEnterDelayAfter   fires once per repetition, entering its
    //                         delay_after phase (only if delay_after > 0)
    //   D  kRepeatStart       every following repetition begins (kFirstOutput
    //                         does not fire again)
    //   E  kDone              fires once, on the very last tick of the run
    UpdateResult<Derived> Update() { return Update(Clock::millis()); }

    UpdateResult<Derived> Update(uint32_t t) {
        auto* self = static_cast<Derived*>(this);
        auto noResult = [self](bool running, EventSet events) {
            return UpdateResult<Derived>(running, events, Brightness{}, false, self);
        };

        if (bPaused_) return noResult(true, 0);
        if (state_ == ST_STOPPED) {
            // A run ends either by completing naturally (kDone emitted on the
            // terminal tick below) or via Stop(). Either way kDone fires exactly
            // once. done_ gates it: the terminal path sets done_ = true as it
            // emits kDone; Stop() leaves done_ = false, so the first Update()
            // after Stop() emits kDone here, then done_ suppresses repeats. An
            // effect-less LED never emits kDone.
            if (!done_ && eval_storage_.IsSet()) {
                done_ = true;
                return noResult(false, static_cast<EventSet>(Event::kDone));
            }
            return noResult(false, 0);
        }
        if (!eval_storage_.IsSet()) return noResult(false, 0);

        const bool was_init = (state_ == ST_INIT);
        if (was_init) {
            time_start_ = t + delay_before_;
            state_ = ST_RUNNING;
        } else {
            // no need to process updates twice during one time tick.
            if ((t & 255) == last_update_time_) return noResult(true, 0);
        }

        // track the last update time
        last_update_time_ = (t & 255);

        EventSet events = 0;
        if (was_init) events |= Event::kStart;

        if (static_cast<int32_t>(t - time_start_) < 0) return noResult(true, events);

        auto writeCur = [this](uint32_t t) {
            const auto val = lerp<Brightness>(Eval(t), minBrightness_, maxBrightness_);
            WriteRaw(val);
            return val;
        };

        const auto period = eval_storage_.Period();
        const auto cycle_period = period + delay_after_;

        // Cycle position is computed before the terminal check (and the
        // resulting event bits accumulated regardless of which path
        // ultimately returns), because an effect can stop on the very tick
        // that would otherwise be a repeat-start or delay-after-entry tick.
        const uint32_t elapsed = t - time_start_;
        const uint32_t t_cycle = elapsed % cycle_period;
        const auto state_before_cycle = state_;

        if (t_cycle == 0) {
            events |= Event::kRepeatStart;
            if (!first_cycle_) {
                events |= Event::kFirstOutput;
                first_cycle_ = true;
            }
        }
        if (t_cycle == period && delay_after_ > 0 && state_before_cycle == ST_RUNNING) {
            events |= Event::kEnterDelayAfter;
        }

        if (!IsForever()) {
            const auto time_end = time_start_ + cycle_period * num_repetitions_ - 1;

            if (static_cast<int32_t>(t - time_end) >= 0) {
                // make sure final value of t = (period-1) is set
                state_ = ST_STOPPED;
                done_ = true;
                events |= Event::kDone;
                return UpdateResult<Derived>(false, events, writeCur(period - 1), true, self);
            }
        }

        if (t_cycle < period) {
            state_ = ST_RUNNING;
            return UpdateResult<Derived>(true, events, writeCur(t_cycle), true, self);
        } else {
            if (state_before_cycle == ST_RUNNING) {
                // when in delay after phase, just call WriteRaw()
                // once at the beginning.
                state_ = ST_IN_DELAY_AFTER_PHASE;
                return UpdateResult<Derived>(true, events, writeCur(period - 1), true, self);
            }
        }
        return noResult(true, events);
    }

 protected:
    void Pause(uint32_t t, eIdleMode mode = eIdleMode::TO_MIN_BRIGHTNESS) {
        if (bPaused_ || state_ == ST_STOPPED) return;
        bPaused_ = true;
        if (mode != eIdleMode::KEEP_CURRENT) {
            WriteRaw(mode == eIdleMode::FULL_OFF ? BrightnessTraits<Brightness>::kZeroBrightness
                                                 : minBrightness_);
        }
        if (state_ != ST_INIT) time_start_ = t - time_start_;  // encode elapsed_so_far in place
        // ST_INIT: time_start_ is 0 and not yet meaningful; Update() resets it on resume
    }

    void Resume(uint32_t t) {
        if (!bPaused_) return;
        bPaused_ = false;
        if (state_ != ST_INIT) time_start_ = t - time_start_;  // restore epoch: t - elapsed_so_far
        // ST_INIT: Update() will set time_start_ fresh on next call
    }

    template<size_t N>
    friend struct TJLedAny;
    friend class TJLedRef;

 public:
    // Number of bits used to control brightness with Min/MaxBrightness().
    static constexpr uint8_t kBitsBrightness = BrightnessTraits<Brightness>::kBits;
    static constexpr Brightness kBrightnessStep = 1;

 private:
    enum State : uint8_t {
        ST_STOPPED = 0,
        ST_INIT = 1,
        ST_RUNNING = 2,
        ST_IN_DELAY_AFTER_PHASE = 3
    };

    // state_ and bPaused_ model two orthogonal dimensions. state_ tracks the
    // progression through the effect (init -> running <-> delay-after ->
    // stopped), while bPaused_ records whether that progression is currently
    // frozen. Keeping pause as a separate flag preserves the underlying state_
    // across a pause/resume cycle (needed to continue where we left off) and
    // avoids the combinatorial blow-up of state_ x paused that a dedicated
    // ST_PAUSED state would require (ST_INIT_PAUSED, ST_RUNNING_PAUSED, ...).
    uint8_t state_ : 2;  // stored as uint8_t to avoid GCC warning about enum bit-field signedness
    uint8_t bLowActive_ : 1;
    uint8_t bPaused_ : 1;
    uint8_t first_cycle_ : 1;  // gates kFirstOutput to the first repeat-start of a run
    uint8_t done_ : 1;         // gates kDone to the single tick that observes the stop
    Brightness minBrightness_;
    Brightness maxBrightness_;

    static constexpr uint16_t kRepeatForever = 65535;
    static constexpr uint16_t kCandleOffsetAuto = 0xffff;
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

};  // namespace jled
