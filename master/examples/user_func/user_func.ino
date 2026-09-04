// JLed user provided effect function demo.
// Copyright 2017-2026 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include <jled.h>

template<typename Value>
class UserEffect : public jled::BrightnessEvaluator<Value> {
    using Traits = jled::ValueTraits<Value>;

 public:
    Value Eval(jled::period_t t) const override {
        // this function changes between OFF and ON every 250 ms. kOffColor()
        // and kOnColor() keep the effect independent of the value type, so it
        // works with the scalar (JLed, JLedHD) as well as the RGB (JLedRGB,
        // JLedRGBHD) LEDs.
        return (t / 250) % 2 ? Traits::kOnColor() : Traits::kOffColor();
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

// example for the RGB version, LED connected to pins 13(R), 14(G), 15(B)
// UserEffect<jled::RGBColor<uint8_t>> userEffect;
// auto led = JLedRGB(13, 14, 15).UserFunc(&userEffect);

void setup() {}

void loop() {
    led.Update();
}
