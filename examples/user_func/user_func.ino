// JLed user provided brightness function demo.
// Copyright 2017 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include <jled.h>

class UserEffect : public jled::BrightnessEvaluator {
    uint8_t Eval(uint32_t t) const override {
        // this function returns changes between 0 and 255 and
        // vice versa every 250 ms.
        return 255*((t/250)%2);
    }
    uint16_t Period() const override { return 5000; }
};

UserEffect userEffect;
auto led = JLed(LED_BUILTIN).UserFunc(&userEffect);

void setup() {}

void loop() {
    led.Update();
}
