// JLed group lifecycle demo. Shows events happening when a sequential group is played.
// Copyright 2026 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
//
// The Lifecycle status is printed to serial on change. Minimum brightness is increased
// on each run of the group. When done, the group is reset so it starts over forever.
//
// JLed sequential group and lifecycle demo.
//   group started
//   new iteration started
//   enter: 0
//   leave: 0
//   enter: 1
//   leave: 1
//   enter: 2
//   leave: 2
//   enter: 3
//   leave: 3
//   enter: 4
//   leave: 4
//   group finished -> Reset
//   group started
//   new iteration started
//   enter: 0
//   ...
#include <jled.h>

constexpr auto LED_PIN = LED_BUILTIN;

JLedAny leds[] = {  // JLedGroup requires JLedAny, not JLed[]
    JLed(LED_PIN).Breathe(2000).Repeat(3),
    JLed(LED_PIN).Blink(750, 250, 3),
    JLed(LED_PIN).FadeOff(1000).Repeat(3),
    JLed(LED_PIN).Blink(500, 500).Repeat(3),
    JLed(LED_PIN).FadeOn(1000).Repeat(3)};

auto sequence = JLedGroup::Sequential(leds);

// we start at MinBrightness(0), the default, then go up each full group iteration
uint8_t min_brightness = 20;

void setup() {
    Serial.begin(9600);

    // wait for max 2s for the Serial port to become ready
    const auto start = millis();
    while (!Serial && millis() - start < 2500) {
    }

    Serial.println("JLed sequential group and lifecycle demo.");
}

void loop() {
    sequence
        .Update()
        // demo: act on the groups lifecycle events. Watch the serial console
        .OnStart([](JLedGroup*) { Serial.println("group started"); })
        .OnRepeatStart([](JLedGroup*) { Serial.println("new iteration started"); })
        .OnElementLeave([](JLedGroup*, uint8_t i, JLedAny& e) {
            // called for each led after its effect is finished: adjust the MinBrightness
            // for the next run of the group. The element that just finished is handed to
            // us directly, no need to index the leds[] array ourselves.
            Serial.print("leave: ");
            Serial.println(static_cast<int>(i));

            if (auto* led = e.As<JLed>()) {
                led->MinBrightness(min_brightness);
            }
        })
        .OnElementEnter([](JLedGroup*, uint8_t i, JLedAny&) {
            Serial.print("enter: ");
            Serial.println(static_cast<int>(i));
        })
        .OnDone([](JLedGroup* g) {
            // called when the group is done playing after the last effect. Increase
            // minimum brightness for the next run and start over by calling Reset()
            Serial.println("group finished -> Reset");

            min_brightness = (min_brightness > 150) ? 20 : min_brightness + 25;
            g->Reset();
        });
}
