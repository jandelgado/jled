// JLed sequential group demo. 'Play' multiple LEDs, one after.
// Copyright 2019-2026 by Jan Delgado. All rights reserved.
//
// Also demonstrates the group's various lifecycle events: Lifecycle status is
// printed to serial on change. When done, the group is reset. So the group runs
// for evever, without having "Forever()" called on the group.
// https://github.com/jandelgado/jled
//
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

void setup() {
    Serial.begin(9600);

    const auto start = millis();
    while (!Serial && millis() - start < 2000) {
    }

    Serial.println("JLed sequential group and lifecycle demo.");
}

void loop() {
    sequence.Update()
        // act on the groups lifecycle events
        .OnStart([](JLedGroup*) { Serial.println("group started"); })
        .OnRepeatStart([](JLedGroup*) { Serial.println("new iteration started"); })
        .OnElementChanged([](JLedGroup*) { Serial.println("next effect playing"); })
        .OnDone([](JLedGroup*g) {
                Serial.println("group finished -> Reset");
                g->Reset();
        });
}
