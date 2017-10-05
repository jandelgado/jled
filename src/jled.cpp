// Copyright (c) 2017 Jan Delgado <jdelgado[at]gmx.net>
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
#include "jled.h"   // NOLINT

JLed::JLed(uint8_t led_pin) :
    effect_inverted_(false), low_active_(false),
    led_pin_(led_pin),  num_repetitions_(1), last_update_time_(0),
    delay_before_(0), delay_after_(0),
    time_start_(kTimeUndef), period_(0), brightness_func_(nullptr),
    effect_param_(0) {
  pinMode(led_pin_, OUTPUT);
}

void JLed::AnalogWrite(uint8_t val) {
  analogWrite(led_pin_, low_active_ ? 255 - val : val);
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
  if (!IsRunning()) {
    return;
  }
  const auto now = millis();

  // no need to process updates twice during one time tick.
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
      delay_before_ = max(0, static_cast<int64_t>(delay_before_) - delta_time); // NOLINT
    if (delay_before_ > 0)
        return;
  }

  const auto time_end = time_start_ +
        (uint32_t)(period_ + delay_after_) * num_repetitions_;

  if (!IsForever() && now >= time_end) {
    brightness_func_ = nullptr;
    return;
  }

  // t cycles in range [0..period+delay_after-1]
  auto t = (now - time_start_) % (period_ + delay_after_);

  if (t < period_) {
    auto val = brightness_func_(t, period_, effect_param_);
    AnalogWrite(effect_inverted_ ? 255 - val : val);
  } else {
      // in delay phase
      return;
  }
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
  return FadeOn(duration).Invert();
}

JLed& JLed::On() {
  period_ = 1;
  return Init(&JLed::OnFunc);
}

JLed& JLed::Off() {
  period_ = 1;
  return Init(&JLed::OffFunc);
}

JLed& JLed::Set(bool on) {
  return on ? On() : Off();
}

JLed& JLed::UserFunc(BrightnessEvalFunction func, uint16_t period) {
  period_ = period;
  return Init(func);
}
