// JLed multi LED demo. 'Play' multiple LEDs, one after another.
// Copyright 2019 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include <jled.h>

constexpr auto PIN_LED = 3;

JLed leds[] = {
    JLed(PIN_LED).Breathe(2000).Repeat(3),
    JLed(PIN_LED).Blink(750, 250).Repeat(3),
    JLed(PIN_LED).FadeOff(1000).Repeat(3),
    JLed(PIN_LED).Blink(500, 500).Repeat(3),
    JLed(PIN_LED).FadeOn(1000).Repeat(3),
    JLed(PIN_LED).Off()
};

JLedSequence sequence(JLedSequence::eMode::SEQUENCE, leds);

void setup() { }

void loop() {
    sequence.Update();
    delay(1);
}
