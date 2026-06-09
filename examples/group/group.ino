// JLed multi LED demo. control multiple LEDs in-sync.
// Copyright (c) 2017-2026 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include <jled.h>

JLedAny leds[] = {
    JLed(4).Blink(750, 250).Repeat(2),
    JLed(3).Breathe(2000),
    JLed(5).FadeOff(1000).Repeat(2),
    JLed(6).FadeOn(1000).Repeat(2),
    JLed(LED_BUILTIN).Blink(500, 500).Repeat(2)
};

auto group = JLedGroup::Parallel(leds).Repeat(5);

void setup() { }

void loop() {
    group.Update();
}
