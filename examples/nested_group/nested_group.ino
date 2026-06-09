// JLed nested group demo. Mix JLed, JLedHD and nested groups in a single sequence.
// Copyright 2026 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include <jled.h>

JLedAny inner[] = {
      JLed(5).Blink(250, 250).Repeat(2),
      JLedHD(6).FadeOn(1000)
};
JLedAny leds[] = {
    JLed(4).Blink(750, 250).Repeat(2),
    JLedHD(3).Breathe(2000),
    JLedGroup::Parallel(inner).Repeat(2)
};

auto group = JLedGroup::Sequential(leds);

void setup() { }

void loop() {
    group.Update();
}
