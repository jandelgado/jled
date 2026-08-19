// JLed Cylon/Larson-scanner demo: 7 LEDs light up one at a time, left to
// right, then right to left, repeating forever. Uses the group's Reverse(),
// Skip() and OnDone() handler features.
// Copyright 2026 by Jan Delgado. All rights reserved.
#include <jled.h>

constexpr auto kTimeOn = 150;

JLed leds[] = {
    // on for kTimeOn ms, then off, before moving on to the next LED
    // The group's OnDone() handler switches the direction and starts over.
    JLed(32).Blink(kTimeOn, 1),
    JLed(33).Blink(kTimeOn, 1),
    JLed(25).Blink(kTimeOn, 1),
    JLed(26).Blink(kTimeOn, 1),
    JLed(19).Blink(kTimeOn, 1),
    JLed(18).Blink(kTimeOn, 1),
    JLed(5).Blink(kTimeOn, 1),
};

// The group is restarted in the OnDone() handler, so no Forever() is needed
// (nor possible here: with Forever(), OnDone() would never be called)
auto group = JLedGroup::Sequential(leds);

void setup() {}

void loop() {
    group.Update().OnDone([](JLedGroup& g) {
            // group is done playing: reverse the direction, start over and
            // skip the last active LED to directly proceed to the previous one.
            g.Reverse().Reset().Skip();
    });
}
