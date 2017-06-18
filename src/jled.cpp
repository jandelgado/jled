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
#include <Arduino.h>
#include "jled.h"   // NOLINT

JLed::JLed(uint8_t led_pin) : state_(IDLE), high_active_(true),
    led_pin_(led_pin),  num_repetitions_(1), last_update_time_(0),
    delay_before_(0), delay_after_(0)  {
  pinMode(led_pin_, OUTPUT);
}

void JLed::DigitalWrite(bool on) {
  digitalWrite(led_pin_, on ^ !high_active_);
}

void JLed::AnalogWrite(uint8_t val) {
  analogWrite(led_pin_, high_active_ ? val : 255 - val);
}

JLed& JLed::Off() {
  state_ = OFF;
  return *this;
}

JLed& JLed::On() {
  state_ = ON;
  return *this;
}

JLed& JLed::Set(bool on) {
  return on ? On(): Off();
}

void JLed::Stop() {
  // Immediately turn LED off
  state_ = IDLE;
  DigitalWrite(false);
}

void JLed::UpdateBlink() {
  if (num_repetitions_ == 0) {
    state_ = IDLE;
    return;
  }

  const auto now = millis();
  const auto timespan_last_change = now  - blink_.time_last_change;

  // check if we have to switch the state?
  if ((blink_.led_state && timespan_last_change >= blink_.duration_on)) {
    // on -> off change
    blink_.time_last_change = now;
    blink_.led_state = false;
    DigitalWrite(blink_.led_state);

    // count full on-off cycles
    if (!IsForever()) {
      num_repetitions_--;
    }

  } else if (!blink_.led_state &&
         timespan_last_change >=  delay_after_) {
    // off -> on change
    blink_.time_last_change = now;
    blink_.led_state = true;
    DigitalWrite(blink_.led_state);
  }
}

// LED breathe effect. LED must be connected to a PWM capable gpio.
// idea see http://sean.voisen.org/blog/2011/10/breathing-led-with-arduino/
// for details.
void JLed::UpdateBreath() {
  const auto now = millis();
  if (breathe_.time_start == 0) {
    breathe_.time_start = now;
  }

  const auto time_end = breathe_.time_start +
        (uint32_t)(breathe_.period + delay_after_) * num_repetitions_;

  if (!IsForever() && now >= time_end) {
    state_ = IDLE;
    return;
  }

  const auto t = (now - breathe_.time_start) % (breathe_.period + delay_after_);
  // TODO(jdelgado) use taylor series?
  // if t >= period, we are in the delay_after phase and the LED is off.
  const float led_val = (t >= breathe_.period) ? 0 :
        (exp(sin((t-breathe_.period/4.)*2.*PI/breathe_.period))
         -0.36787944)*108.0;
  AnalogWrite(led_val);
}

// update LED state according to current state.
void JLed::Update() {
  if (state_ == IDLE) {
    return;
  }
  // no need to process updates twice during one time tick.
  const auto now = millis();

  if (last_update_time_ == 0) {
    last_update_time_ = now;
  }
  const auto delta_time = now - last_update_time_;
  if (delta_time == 0) {
    return;
  }
  last_update_time_ = now;

  if (delay_before_ > 0) {
    delay_before_ = max(0, static_cast<int64_t>(delay_before_) - delta_time); // NOLINT
    return;
  }

  switch (state_) {
    case BLINK:   UpdateBlink(); break;
    case BREATHE: UpdateBreath(); break;
    case OFF:     DigitalWrite(false); state_ = IDLE; break;
    case ON:      DigitalWrite(true); state_ = IDLE; break;
    default:      break;
  }
}

JLed& JLed::Blink(uint16_t duration_on, uint16_t duration_off) {
  blink_.duration_on = duration_on;
  delay_after_ = duration_off;
  state_ = BLINK;

  // force change to ON on first call to UpdateBlink()
  blink_.led_state = false;
  blink_.time_last_change = -1;
  return *this;
}

JLed& JLed::Breathe(uint16_t period) {
  breathe_.time_start = 0;  // actual time will be set on first call to Update
  breathe_.period = period;
  state_ = BREATHE;
  return *this;
}
