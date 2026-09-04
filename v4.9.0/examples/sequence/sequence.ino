// JLed multi LED demo. 'Play' multiple LEDs, one after another.
// Copyright 2019 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include <jled.h>

constexpr auto LED_PIN = 3;

JLed leds[] = {
    JLed(LED_PIN).Breathe(2000).Repeat(3),
    JLed(LED_PIN).Blink(750, 250).Repeat(3),
    JLed(LED_PIN).FadeOff(1000).Repeat(3),
    JLed(LED_PIN).Blink(500, 500).Repeat(3),
    JLed(LED_PIN).FadeOn(1000).Repeat(3),
    JLed(LED_PIN).Off()
};

JLedSequence sequence(JLedSequence::eMode::SEQUENCE, leds);

void setup() { }

void loop() {
    sequence.Update();
    delay(1);
}
