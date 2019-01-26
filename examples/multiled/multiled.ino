// JLed multi LED demo. control multiple LEDs in-sync.
// Copyright 2017 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include <jled.h>

JLed leds[] = {
    JLed(4).Blink(750, 250).Forever(),
    JLed(3).Breathe(2000).Forever(),
    JLed(5).FadeOff(1000).Forever(),
    JLed(6).FadeOn(1000).Forever(),
    JLed(LED_BUILTIN).Blink(500, 500).Forever()
};

JLedSequence sequence(JLedSequence::eMode::PARALLEL, leds, 5);

void setup() { }

void loop() {
    sequence.Update();
    delay(1);
}
