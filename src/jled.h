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
 private:
  enum eState {BLINK, BREATHE, ON, OFF, IDLE};
  static constexpr uint16_t kRepeatForever = 65535;

  eState    state_;
  bool      high_active_;
  uint8_t   led_pin_;
  uint16_t  num_repetitions_;
  uint32_t  last_update_time_;
  uint16_t  delay_before_;
  uint16_t  delay_after_;

  union {
    struct {
      uint16_t  duration_on;
      bool      led_state;
      uint32_t  time_last_change;
    } blink_;

    struct {
      uint32_t  time_start;
      uint16_t  period;
    } breathe_;
  };

  void UpdateBlink();
  void UpdateBreath();

  // internal control of the LED, does not affect state and
  // honors high_active_ flag
  void DigitalWrite(bool on_off);
  void AnalogWrite(uint8_t val);

 public:
  explicit JLed(uint8_t led_pin);

  // call Update() from the loop() function to control the LED.
  void Update();

  // turn LED on, respecting delay_before
  JLed& On();

  // turn LED off, respecting delay_before
  JLed& Off();

  // turn LED on or off
  JLed& Set(bool on);

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

  // Set amount of time to wait after each iteration. Note: for symmetry
  // reasons, in blink mode setting this value effectively adds up to the
  // time the LED is turned off.
  JLed& DelayAfter(uint16_t delay_after) {
    delay_after_ = delay_after;
    return *this;
  }

  // Set LED tow be lo active, i.e. LED will be ON when output pin is set to LOW
  JLed& LowActive() {
    high_active_ = false;
    return *this;
  }

  // Set  effect to Breathe, with the given priod time in ms.
  JLed& Breathe(uint16_t period);

  // Set  effect to Blink, with the given on- and off- duration values.
  JLed& Blink(uint16_t duration_on, uint16_t duration_off);

  // Stop current effect. Turn led off
  void Stop();
};
#endif  // SRC_JLED_H_
