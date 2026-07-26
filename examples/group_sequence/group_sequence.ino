// JLed sequential group demo. 'Play' multiple LEDs, one after.
// Copyright 2019-2026 by Jan Delgado. All rights reserved.
#include <jled.h>

constexpr auto LED_PIN = LED_BUILTIN;

JLedAny leds[] = {  // JLedGroup requires JLedAny, not JLed[]
    JLed(LED_PIN).Breathe(2000).Repeat(3),
    JLed(LED_PIN).Blink(750, 250).Repeat(3),
    JLed(LED_PIN).FadeOff(1000).Repeat(3),
    JLed(LED_PIN).Blink(500, 500).Repeat(3),
    JLed(LED_PIN).FadeOn(1000).Repeat(3),
    JLed(LED_PIN).Off()};

auto sequence = JLedGroup::Sequential(leds);

void setup() {}

void loop() {
    sequence.Update();
}
