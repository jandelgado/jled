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
#ifndef SRC_JLED_H_
#define SRC_JLED_H_

#include <Arduino.h>

// Non-blocking LED abstraction class.
//
// Example Arduino sketch:
//   JLed led = JLed(LED_BUILTIN).Blink(500, 500).Repeat(10).DelayBefore(1000);
//
//   void setup() {}
//
//   void loop() {
//     led.Update();
//   }
//

class JLed {
 public:
  // a function f(t,period) that calculates the LEDs brightness for a given
  // point in time and the given period. t is in range [0..period-1].
  // f(peroid-1,period) will always be called and can be used to set the final
  // state.
  using  BrightnessEvalFunction = uint8_t(*)(uint32_t t, uint16_t period);

 private:
  enum eState { RUNNING, IDLE };
  static constexpr uint16_t kRepeatForever = 65535;

  eState    state_;
  bool      high_active_;
  uint8_t   led_pin_;
  uint16_t  num_repetitions_;
  uint32_t  last_update_time_;
  uint16_t  delay_before_;    // delay before the first effect starts
  uint16_t  delay_after_;     // delay after each repetition
  bool      in_delay_phase_;  // true if in delay phase
  uint32_t  time_start_;
  uint16_t  period_;
  BrightnessEvalFunction brightness_func_;

  // internal control of the LED, does not affect
  // state and honors high_active_ flag
  void AnalogWrite(uint8_t val);

  JLed& Init(BrightnessEvalFunction func);

 public:
  explicit JLed(uint8_t led_pin);

  // call Update() from the loop() function to control the LED.
  void Update();

  // turn LED on, respecting delay_before
  JLed& On();

  // turn LED off, respecting delay_before
  JLed& Off();

  // turn LED on or off, calls On() / Off()
  JLed& Set(bool on);

  // Fade LED on
  JLed& FadeOn(uint16_t duration);

  // Fade LED off
  JLed& FadeOff(uint16_t duration);

  // set number of repetitions for Blink() or Breathe()
  JLed& Repeat(uint16_t num_repetitions) {
    num_repetitions_ = num_repetitions;
    return *this;
  }

  // repeat Forever
  JLed& Forever() {
    return Repeat(kRepeatForever);
  }

  bool IsForever() const {
    return num_repetitions_ == kRepeatForever;
  }

  // Set amount of time to initially wait before updating LED. Time is relative
  // to first call of Update() method.
  JLed& DelayBefore(uint16_t delay_before) {
    delay_before_ = delay_before;
    return *this;
  }

  // Set amount of time to wait after each iteration.
  JLed& DelayAfter(uint16_t delay_after) {
    delay_after_ = delay_after;
    return *this;
  }

  // Set LED to be lo active, i.e. LED will be ON when output pin is set to LOW
  JLed& LowActive() {
    high_active_ = false;
    return *this;
  }

  // Set effect to Breathe, with the given priod time in ms.
  JLed& Breathe(uint16_t period);

  // Set effect to Blink, with the given on- and off- duration values.
  JLed& Blink(uint16_t duration_on, uint16_t duration_off);

  // Use user provided function func as effect.
  JLed& UserFunc(BrightnessEvalFunction func, uint16_t period);

  // Stop current effect and turn LED off
  void Stop();

 private:
  // permanently turn LED on
  static uint8_t OnFunc(uint32_t, uint16_t) {
    return 255;
  }

  // permanently turn LED off
  static uint8_t OffFunc(uint32_t, uint16_t) {
    return 0;
  }

  // LED breathe effect. LED must be connected to a PWM capable gpio.
  // idea see http://sean.voisen.org/blog/2011/10/breathing-led-with-arduino/
  // for details.
  static uint8_t BreatheFunc(uint32_t t, uint16_t period) {
    return (t > period) ? 0 :
           static_cast<uint8_t>((exp(sin((t-period/4.)
                                 *2.*PI/period))-0.36787944)*108.0);
  }

  // turn LED off if t == period-1, before entering delay_after phase
  static uint8_t BlinkFunc(uint32_t t, uint16_t period) {
    return (t < period-1) ? 255 : 0;
  }

  // fade LED on
  static uint8_t FadeOnFunc(uint32_t t, uint16_t period) {
    return  (t >= period-1) ? 255 :
              static_cast<uint8_t>((exp(sin((t-period/2.)
                                    *PI/period))-0.36787944)*108.0);
  }

  // fade led off
  static uint8_t FadeOffFunc(uint32_t t, uint16_t period) {
    return  255-FadeOnFunc(t, period);
  }
};
#endif  // SRC_JLED_H_
