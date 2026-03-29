// JLed user provided brightness function demo.
// Copyright 2017 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include <jled.h>

template<typename Brightness>
class UserEffect : public jled::BrightnessEvaluator<Brightness> {
 public:
    Brightness Eval(uint32_t t) const override {
        // this function changes between OFF and ON  every 250 ms.
        return jled::BrightnessTraits<Brightness>::kFullBrightness*((t/250)%2);
    }
    // duration of effect: 5 seconds.
    uint16_t Period() const override { return 5000; }
};

// example for the JLedHD high-definition resolution version
UserEffect<uint16_t> userEffect;
auto led = JLedHD(LED_BUILTIN).UserFunc(&userEffect);

// example for the JLed 8-bit resolution version
// UserEffect<uint8_t> userEffect;
// auto led = JLed(LED_BUILTIN).UserFunc(&userEffect);

void setup() {}

void loop() {
    led.Update();
}
