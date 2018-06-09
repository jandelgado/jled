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
#include "jled.h"  // NOLINT

// uncomment to enable workaround (does not solve the problem totally) for
// flickering LED when turning LED off.  see
// https://arduino.stackexchange.com/questions/17946/why-does-an-led-sometimes-flash-when-increasing-brightness
// #define PWM_FLICKER_WORKAROUND_ENABLE

constexpr uint8_t JLed::kFadeOnTable[];

JLed::JLed(uint8_t led_pin) : led_pin_(led_pin) {
    pinMode(led_pin, OUTPUT);
}

JLed& JLed::SetFlags(uint8_t f, bool val) {
    if (val) {
        flags_ |= f;
    } else {
        flags_ &= ~f;
    }
    return *this;
}

bool JLed::GetFlag(uint8_t f) const { return (flags_ & f) != 0; }

void JLed::SetInDelayAfterPhase(bool f) { SetFlags(FL_IN_DELAY_PHASE, f); }

bool JLed::IsInDelayAfterPhase() const { return GetFlag(FL_IN_DELAY_PHASE); }

JLed& JLed::Repeat(uint16_t num_repetitions) {
    num_repetitions_ = num_repetitions;
    return *this;
}

JLed& JLed::Forever() { return Repeat(kRepeatForever); }

bool JLed::IsForever() const { return num_repetitions_ == kRepeatForever; }

JLed& JLed::DelayBefore(uint16_t delay_before) {
    delay_before_ = delay_before;
    return *this;
}

JLed& JLed::DelayAfter(uint16_t delay_after) {
    delay_after_ = delay_after;
    return *this;
}

JLed& JLed::Invert() { return SetFlags(FL_INVERTED, true); }

bool JLed::IsInverted() const { return GetFlag(FL_INVERTED); }

JLed& JLed::LowActive() { return SetFlags(FL_LOW_ACTIVE, true); }

bool JLed::IsLowActive() const { return GetFlag(FL_LOW_ACTIVE); }

void JLed::AnalogWrite(uint8_t val) {
    auto new_val = IsLowActive() ? 255 - val : val;
#ifdef PWM_FLICKER_WORKAROUND_ENABLE
    if (new_val == 0) {
        analogWrite(led_pin_, 1);
    }
#endif
#ifdef ESP8266
    // ESP8266 uses 10bit PWM range per default, scale value up
    new_val <<= 2;
#endif
    analogWrite(led_pin_, new_val);
}

void JLed::Stop() {
    // Immediately turn LED off and stop effect.
    brightness_func_ = nullptr;
    AnalogWrite(0);
}

// update brightness of LED using the given brightness function
//  (brightness)                     _________________
// on 255 |                       ¸-'
//        |                    ¸-'
//        |                 ¸-'
// off 0  |______________¸-'
//        |<delay before>|<--period-->|<-delay after-> (time)
//                       | func(t)    |
//                       |<- num_repetitions times  ->
void JLed::Update() {
    if (!brightness_func_) {
        return;
    }
    const auto now = millis();

    // no need to process updates twice during one time tick.
    if (last_update_time_ == now) {
        return;
    }

    // last_update_time_ will be 0 on initialization, so this fails on
    // first call to this method.
    if (last_update_time_ == kTimeUndef) {
        last_update_time_ = now;
        time_start_ = now + delay_before_;
    }
    const auto delta_time = now - last_update_time_;
    last_update_time_ = now;
    // wait until delay_before time is elapsed before actually doing anything
    if (delay_before_ > 0) {
        delay_before_ =
            max(static_cast<int64_t>(0), static_cast<int64_t>(delay_before_) - delta_time);  // NOLINT
        if (delay_before_ > 0) return;
    }

    const auto time_end =
        time_start_ + (uint32_t)(period_ + delay_after_) * num_repetitions_;

    if (!IsForever() && now >= time_end) {
        // make sure final value is set
        AnalogWrite(EvalBrightness(period_ - 1));
        brightness_func_ = nullptr;
        return;
    }

    // t cycles in range [0..period+delay_after-1]
    const auto t = (now - time_start_) % (period_ + delay_after_);

    if (t < period_) {
        SetInDelayAfterPhase(false);
        AnalogWrite(EvalBrightness(t));
    } else {
        if (IsInDelayAfterPhase()) {
            return;
        } else {
            SetInDelayAfterPhase(true);
            AnalogWrite(EvalBrightness(period_ - 1));
        }
    }
}

uint8_t JLed::EvalBrightness(uint32_t t) const {
    const auto val = brightness_func_(t, period_, effect_param_);
    return IsInverted() ? 255 - val : val;
}

JLed& JLed::Init(BrightnessEvalFunction func) {
    brightness_func_ = func;
    last_update_time_ = kTimeUndef;
    time_start_ = kTimeUndef;
    return *this;
}

JLed& JLed::Blink(uint16_t duration_on, uint16_t duration_off) {
    period_ = duration_on + duration_off;
    effect_param_ = duration_on;
    return Init(&JLed::BlinkFunc);
}

JLed& JLed::Breathe(uint16_t period) {
    period_ = period;
    return Init(&JLed::BreatheFunc);
}

JLed& JLed::FadeOn(uint16_t duration) {
    period_ = duration;
    return Init(&JLed::FadeOnFunc);
}

JLed& JLed::FadeOff(uint16_t duration) {
    period_ = duration;
    return Init(&JLed::FadeOffFunc);
}

JLed& JLed::On() {
    period_ = 1;
    return Init(&JLed::OnFunc);
}

JLed& JLed::Off() {
    period_ = 1;
    return Init(&JLed::OffFunc);
}

JLed& JLed::Set(bool on) { return on ? On() : Off(); }

JLed& JLed::UserFunc(BrightnessEvalFunction func, uint16_t period,
                     uintptr_t user_param) {
    effect_param_ = user_param;
    period_ = period;
    return Init(func);
}

uint8_t JLed::OnFunc(uint32_t, uint16_t, uintptr_t) { return 255; }

uint8_t JLed::OffFunc(uint32_t, uint16_t, uintptr_t) { return 0; }

uint8_t JLed::BlinkFunc(uint32_t t, uint16_t period, uintptr_t effect_param) {
    return (t < effect_param) ? 255 : 0;
}

// The breathe func is composed by fadein and fade-out with one each half
// period.  we approximate the following function:
//   y(x) = exp(sin((t-period/4.) * 2. * PI / period)) - 0.36787944) * 108.)
// idea see:  http://sean.voisen.org/blog/2011/10/breathing-led-with-arduino/
// But we do it with integers only.
uint8_t JLed::BreatheFunc(uint32_t t, uint16_t period, uintptr_t) {
    if (t + 1 >= period) return 0;
    const uint16_t periodh = period >> 1;
    return t < periodh ? FadeOnFunc(t, periodh, 0)
                       : FadeOffFunc(t - periodh, periodh, 0);
}

// The fade-on func is an approximation of
//   y(x) = exp(sin((t-period/2.) * PI / period)) - 0.36787944) * 108.)
uint8_t JLed::FadeOnFunc(uint32_t t, uint16_t period, uintptr_t) {
    if (t + 1 >= period) return 255;

    // approximate by linear interpolation.
    // scale t according to period to 0..255
    t = ((t << 8) / period) & 0xff;
    const auto i = (t >> 5);  // -> i will be in range 0 .. 7
    const auto y0 = kFadeOnTable[i];
    const auto y1 = kFadeOnTable[i + 1];
    const auto x0 = i << 5;  // *32

    // y(t) = mt+b, with m = dy/dx = (y1-y0)/32 = (y1-y0) >> 5
    return (((t - x0) * (y1 - y0)) >> 5) + y0;
}

uint8_t JLed::FadeOffFunc(uint32_t t, uint16_t period, uintptr_t) {
    return FadeOnFunc(period-t, period, 0);
}
