// JLed multi LED demo. 'Play' multiple LEDs, one after another.
// Copyright 2019 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include <jled.h>

// LEDs are connected to GPIO 3
JLed leds[] = {
    JLed(3).Breathe(2000).Repeat(3),
    JLed(3).Blink(750, 250).Repeat(3),
    JLed(3).FadeOff(1000).Repeat(3),
    JLed(3).Blink(500, 500).Repeat(3),
    JLed(3).FadeOn(1000).Repeat(3),
    JLed(3).Off()
};

JLedSequence sequence(JLedSequence::eMode::SEQUENCE, leds, 6);

void setup() { }

void loop() {
    sequence.Update();
    delay(1);
}
