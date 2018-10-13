// JLed multi LED demo. control multiple LEDs in-sync.
// Copyright 2017 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include <jled.h>

JLed leds[] = {
    JLed(3).Breathe(2000),
    JLed(3).Blink(750, 250),
    JLed(3).FadeOff(1000),
    JLed(3).Blink(500, 500),
    JLed(3).FadeOn(1000),
    JLed(3).Blink(2500, 500)
};

JLedSequence seq( leds, 6 );
void setup() {
}

void loop() {
    seq.Update();
    delay(1);
}
