// JLed multi led demo. control multiple LEDs syncronized.
// Copyright 2017 by Jan Delgado. All rights reserved.

#include <jled.h>

JLed leds[5] = {
    JLed(3).Breathe(2000).Forever(),
    JLed(4).Blink(750, 250).Forever(),
    JLed(5).FadeOff(1000).Forever(),
    JLed(6).FadeOn(1000).Forever(),
    JLed(LED_BUILTIN).Blink(500, 500).Forever()};

void setup() {
}

void loop() {
    for (auto& led : leds) {led.Update();}
    delay(1);
}
