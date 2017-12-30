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
    // a function f(t,period,param) that calculates the LEDs brightness for a
    // given point in time and the given period. param is an optionally user
    // provided parameter. t will always be in range [0..period-1].
    // f(period-1,period,param) will be called last to calculate the final
    // state of the LED.
    using BrightnessEvalFunction = uint8_t (*)(uint32_t t, uint16_t period,
                                               uintptr_t param);


    JLed() = delete;
    explicit JLed(uint8_t led_pin);

    // call Update() from the loop() function to update LED state.
    void Update();

    // turn LED on, respecting delay_before
    JLed& On();

    // turn LED off, respecting delay_before
    JLed& Off();

    // turn LED on or off, calls On() / Off()
    JLed& Set(bool on);

    // Fade LED on
    JLed& FadeOn(uint16_t duration);

    // Fade LED off - acutally is just inverted version of FadeOn()
    JLed& FadeOff(uint16_t duration);

    // Set effect to Breathe, with the given period time in ms.
    JLed& Breathe(uint16_t period);

    // Set effect to Blink, with the given on- and off- duration values.
    JLed& Blink(uint16_t duration_on, uint16_t duration_off);

    // Use user provided function func as brightness function.
    JLed& UserFunc(BrightnessEvalFunction func, uint16_t period,
                   uintptr_t user_param = 0);

    // set number of repetitions for effect.
    JLed& Repeat(uint16_t num_repetitions);

    // repeat Forever
    JLed& Forever();
    bool IsForever() const;

    // Set amount of time to initially wait before effect starts. Time is
    // relative to first call of Update() method and specified in ms.
    JLed& DelayBefore(uint16_t delay_before);

    // Set amount of time to wait in ms after each iteration.
    JLed& DelayAfter(uint16_t delay_after);

    // Invert effect. If set, every effect calculation will be inverted, i.e.
    // instead of a, 255-a will be used.
    JLed& Invert();
    bool IsInverted() const;

    // Set physical LED polarity to be low active. This inverts every signal
    // physically output to a pin.
    JLed& LowActive();
    bool IsLowActive() const;

    // Stop current effect and turn LED immeadiately off
    void Stop();

 protected:
    BrightnessEvalFunction brightness_func_ = nullptr;
    uintptr_t effect_param_ = 0;  // optional additional effect paramter.

    // internal control of the LED, does not affect
    // state and honors low_active_ flag
    void AnalogWrite(uint8_t val);

    JLed& Init(BrightnessEvalFunction func);

    JLed& SetFlags(uint8_t f, bool val);
    bool GetFlag(uint8_t f) const;

    void SetInDelayAfterPhase(bool f);
    bool IsInDelayAfterPhase() const;

    uint8_t EvalBrightness(uint32_t t) const;

    // permanently turn LED on
    static uint8_t OnFunc(uint32_t, uint16_t, uintptr_t);

    // permanently turn LED off
    static uint8_t OffFunc(uint32_t, uint16_t, uintptr_t);

    // BlincFunc does one on-off cycle in the specified period. The effect_param
    // specifies the time the effect is on.
    static uint8_t BlinkFunc(uint32_t t, uint16_t period, uintptr_t on_time);

    // fade LED on
    // https://www.wolframalpha.com/input/?i=plot+(exp(sin((x-100%2F2.)*PI%2F100))-0.36787944)*108.0++x%3D0+to+100
    static uint8_t FadeOnFunc(uint32_t t, uint16_t period, uintptr_t);

    // Fade LED off - inverse of FadeOnFunc()
    static uint8_t FadeOffFunc(uint32_t t, uint16_t period, uintptr_t);

    // LED breathe effect. Composition of fade-on and fade-off with half
    // the period each.
    static uint8_t BreatheFunc(uint32_t t, uint16_t period, uintptr_t);

 private:
    // pre-calculated fade-on function. This table samples the function
    //   y(x) =  exp(sin((t - period / 2.) * PI / period)) - 0.36787944) * 108.
    // at x={0,32,...,256}. In FadeOnFunc() we us linear interpolation to
    // approximate the original function (so we do not need fp-ops).
    // fade-off and breath functions are all derived from fade-on, see below.
    // (To save some additional bytes, we could place it in PROGMEM sometime)
    static constexpr uint8_t kFadeOnTable[] = {0, 3, 13, 33, 68, 118, 179, 232,
        255};

    static constexpr uint16_t kRepeatForever = 65535;
    static constexpr uint32_t kTimeUndef = -1;

    static constexpr uint8_t FL_INVERTED = (1 << 0);
    static constexpr uint8_t FL_LOW_ACTIVE = (1 << 1);
    static constexpr uint8_t FL_IN_DELAY_PHASE = (1 << 2);
    uint8_t flags_ = 0;

    uint8_t led_pin_;
    uint16_t num_repetitions_ = 1;
    uint32_t last_update_time_ = kTimeUndef;
    uint16_t delay_before_ = 0;  // delay before the first effect starts
    uint16_t delay_after_ = 0;   // delay after each repetition
    uint32_t time_start_ = kTimeUndef;
    uint16_t period_ = 0;
};
#endif  // SRC_JLED_H_
