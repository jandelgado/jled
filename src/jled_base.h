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

// The JLed state machine

namespace jled {

// Forward declaration; TJLedGroup is defined in jled_group_base.h and needs friend
// access to TJLed's protected Pause(uint32_t, eIdleMode)/Resume(uint32_t).
template<typename Clock, typename ElementType>
class TJLedGroup;

// Non-template tag base, allows TJLedAny/TJLedRef to detect TJLed subclasses via
// std::is_base_of without requiring a common virtual interface.
class JLedBase {};

enum class eIdleMode { TO_MIN_BRIGHTNESS = 0, FULL_OFF, KEEP_CURRENT };

template<typename Hal, typename Clock, typename Value, typename Derived>
class TJLed : public JLedBase {
 protected:
    // Active brightness evaluator (discriminated union).
    EvalStorage<Value> eval_storage_;
    // Hardware abstraction giving access to the MCU
    Hal hal_;

    // Evaluate effect(t), assumes eval_storage_.IsSet().
    Value Eval(period_t t) const { return eval_storage_.Eval(t); }

 public:
    using value_t = Value;
    using level_t = typename ValueTraits<Value>::level_t;

    TJLed() = delete;
    explicit TJLed(const Hal& hal)
        : hal_{hal},
          state_{ST_INIT},
          bLowActive_{false},
          bPaused_{false},
          first_cycle_{false},
          done_{false},
          minBrightness_{ValueTraits<level_t>::kMinValue()},
          maxBrightness_{ValueTraits<level_t>::kMaxValue()} {}

    explicit TJLed(typename Hal::PinType pin) : TJLed(Hal{pin}) {}

    TJLed(const TJLed& rLed) : hal_{rLed.hal_} { *this = rLed; }

    Derived& operator=(const TJLed<Hal, Clock, Value, Derived>& rLed) {
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

    // Set or clear physical LED low-active polarity. When on, every signal
    // physically output to a pin is inverted. Therefore every HAL must implement
    // SetLowActive(bool). It is notified once here so a HAL with a hardware
    // inversion can use it instead of calculating the inverse in analogWrite()
    Derived& LowActive(bool on = true) {
        bLowActive_ = on;
        hal_.SetLowActive(bLowActive_);
        return static_cast<Derived&>(*this);
    }

    bool IsLowActive() const { return bLowActive_; }

    // turn LED on
    Derived& On(uint16_t duration = 1) {
        return Set(ValueTraits<Value>::kOnColor(), duration);
    }

    // turn LED off
    Derived& Off(uint16_t duration = 1) {
        return Set(ValueTraits<Value>::kOffColor(), duration);
    }

    // Sets LED to given brightness/color. As for every effect, a duration can be
    // specified. Update() will return false after the duration elapsed.
    Derived& Set(Value color, uint16_t duration = 1) {
        eval_storage_.type = EvalType::CONSTANT;
        eval_storage_.data.constant = {color, duration};
        return Reset();
    }

    // Fade LED on from `from_color` to `to_color`. Since you normally only specify
    // the target brightness/color, the `to_color` argument comes first, then the
    // `from_color` (this is reversed in `FadeOff()`)
    Derived& FadeOn(uint16_t duration,
                    Value to_color = ValueTraits<Value>::kOnColor(),
                    Value from_color = ValueTraits<Value>::kOffColor()) {
        eval_storage_.type = EvalType::BREATHE;
        eval_storage_.data.breathe = {duration, 0, 0, from_color, to_color};
        return Reset();
    }

    // Fade LED off - actually is just inverted version of `FadeOn()`
    Derived& FadeOff(uint16_t duration,
                     Value from_color = ValueTraits<Value>::kOnColor(),
                     Value to_color = ValueTraits<Value>::kOffColor()) {
        eval_storage_.type = EvalType::BREATHE;
        eval_storage_.data.breathe = {0, 0, duration, to_color, from_color};
        return Reset();
    }

    // Fade from "from_color" to "to_color" with the given "duration". Sets up the breathe
    // effect with the proper parameters.
    Derived& Fade(Value from_color, Value to_color, uint16_t duration) {
        return ValueTraits<Value>::IsBrighter(from_color, to_color)
            ? FadeOn(duration, to_color, from_color) : FadeOff(duration, from_color, to_color);
    }

    // Set effect to Breathe, with the given period time in ms.
    Derived& Breathe(uint16_t period) { return Breathe(period / 2, 0, period / 2); }

    // Set effect to Breathe, with the given fade on-, on- and fade off-
    // duration values and optional brightness/color values to move between.
    Derived& Breathe(uint16_t duration_fade_on, uint16_t duration_on, uint16_t duration_fade_off,
                     Value from_color = ValueTraits<Value>::kOffColor(),
                     Value to_color = ValueTraits<Value>::kOnColor()) {
        eval_storage_.type = EvalType::BREATHE;
        eval_storage_.data.breathe = {duration_fade_on, duration_on, duration_fade_off,
                                      from_color, to_color};
        return Reset();
    }

    // Set effect to Blink, with the given on- and off- duration values, an optional
    // number of iterations (default is 1) and optional brightness/color values to switch between.
    Derived& Blink(uint16_t duration_on, uint16_t duration_off, uint8_t n = 1,
                   Value color_on = ValueTraits<Value>::kOnColor(),
                   Value color_off = ValueTraits<Value>::kOffColor()) {
        eval_storage_.type = EvalType::BLINK;
        eval_storage_.data.blink = {duration_on, duration_off, n, color_on, color_off};
        return Reset();
    }

    // Set effect to Candle light simulation. Effect changes between given color_on and color_off
    // brightness/colors values. Offset controls how in sync individual candle effects are (0 for in
    // sync). When offset is omitted, a value derived from this instance's address and the current
    // time is used, so multiple LEDs with default parameters automatically flicker independently.
    // Pass an explicit offset (in ms) to control the phase precisely, e.g. to build a multi-LED
    // wave/chase effect.
    Derived& Candle(Value color_on = ValueTraits<Value>::kOnColor(),
                    Value color_off = ValueTraits<Value>::kOffColor(), uint8_t speed = 6,
                    uint8_t jitter = 15, uint16_t period = 0xffff,
                    uint16_t offset = kCandleOffsetAuto) {
        eval_storage_.type = EvalType::CANDLE;
        const uint16_t actual_offset =
            (offset == kCandleOffsetAuto)
                ? static_cast<uint16_t>(hash32(
                      static_cast<uint32_t>(reinterpret_cast<uintptr_t>(this)) ^ Clock::millis()))
                : offset;
        eval_storage_.data.candle = CandleBrightnessEvaluator<Value>(
            speed, jitter, period, actual_offset, color_on, color_off);
        return Reset();
    }

    // Use a user provided effect.
    Derived& UserFunc(BrightnessEvaluator<Value>* user_eval) {
        eval_storage_.type = EvalType::USER;
        eval_storage_.data.user = user_eval;
        return Reset();
    }

    // Set number of repetitions for effect.
    Derived& Repeat(uint16_t num_repetitions) {
        num_repetitions_ = num_repetitions;
        return static_cast<Derived&>(*this);
    }

    // Repeat Forever.
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
    // Update() will have no effect. `mode` controls what `off` means (
    // set to definied minimum brightness, turn fully off, or just keep the
    // current value).
    Derived& Stop(eIdleMode mode = eIdleMode::TO_MIN_BRIGHTNESS) {
        if (mode != eIdleMode::KEEP_CURRENT) {
            WriteRaw(mode == eIdleMode::FULL_OFF
                         ? ValueTraits<Value>::kOffColor()
                         : ValueTraits<Value>::ApplyBounds(ValueTraits<Value>::kOffColor(),
                                                           minBrightness_, maxBrightness_));
        }
        state_ = ST_STOPPED;
        bPaused_ = false;
        return static_cast<Derived&>(*this);
    }

    bool IsRunning() const { return state_ != ST_STOPPED; }

    void Pause(eIdleMode mode = eIdleMode::TO_MIN_BRIGHTNESS) { Pause(Clock::millis(), mode); }

    void Resume() { Resume(Clock::millis()); }

    bool IsPaused() const { return bPaused_; }

    // Reset to inital state (keeping the current effect, not changing output)
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
    Derived& MinBrightness(level_t level) {
        minBrightness_ = level;
        return static_cast<Derived&>(*this);
    }

    // Returns current minimum brightness level.
    level_t MinBrightness() const { return minBrightness_; }

    // Sets the maximum brightness level.
    Derived& MaxBrightness(level_t level) {
        maxBrightness_ = level;
        return static_cast<Derived&>(*this);
    }

    // Returns current maximum brightness level.
    level_t MaxBrightness() const { return maxBrightness_; }

    // Write a brightness/color value directly out to the hardware using the HAL. Bypasses the
    // effect state machine ("raw" = no Update()/Eval()). Used e.g. for forcing an output from a
    // lifecycle callback (see UpdateResult).
    Derived& WriteRaw(Value val) {
        hal_.analogWrite(val, IsLowActive());
        return static_cast<Derived&>(*this);
    }

    // update color/brightness of LED using the given effect evaluator and the
    // current time. Returns an UpdateResult carrying whether the effect is
    // still running, which lifecycle events fired this tick, and the
    // brightness value/color (if any) written to the output this tick (the
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
            return UpdateResult<Derived>(running, events, Value{}, false, self);
        };

        if (bPaused_) return noResult(true, 0);
        if (state_ == ST_STOPPED) {
            // A run ends either by completing naturally (kDone emitted on the
            // terminal tick below) or via Stop(). Either way kDone fires exactly
            // once and done_ gates it.
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

        auto writeCur = [this](period_t t) {
            const auto val =
                ValueTraits<Value>::ApplyBounds(Eval(t), minBrightness_, maxBrightness_);
            WriteRaw(val);
            return val;
        };

        const auto period = eval_storage_.Period();
        // 32 bit on purpose: both operands are uint16_t, so letting this deduce
        // would evaluate the sum and the cycle_period * num_repetitions_ product
        // below in int, which is only 16 bits wide on AVR and wraps there for
        // ordinary run lengths (e.g. Blink(500, 500).Repeat(100) = 100000ms).
        const uint32_t cycle_period = static_cast<uint32_t>(period) + delay_after_;

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
            WriteRaw(mode == eIdleMode::FULL_OFF
                         ? ValueTraits<Value>::kOffColor()
                         : ValueTraits<Value>::ApplyBounds(ValueTraits<Value>::kOffColor(),
                                                           minBrightness_, maxBrightness_));
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
    template<typename GroupClock, typename GroupElementType>
    friend class TJLedGroup;

 public:
    // Number of bits used to control brightness with Min/MaxBrightness().
    static constexpr uint8_t kBitsBrightness = ValueTraits<Value>::kBits;

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
    level_t minBrightness_;
    level_t maxBrightness_;

    static constexpr uint16_t kRepeatForever = 65535;
    uint16_t num_repetitions_ = 1;

    // Sentinel for Candle()'s offset parameter, meaning "derive an offset automatically".
    static constexpr uint16_t kCandleOffsetAuto = 0xffff;

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
