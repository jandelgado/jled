// JLed lifecycle example.
// Copyright 2026 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
//
// FadeOn 5 times with increasing brightness between each fade and
// turn the LED off when entering the 1 second delay-after phase. When done, also
// turn the LED off. For a better understanding of what's happening, the callbacks
// print out some info to the Serial port.
//
// Expected console output:
//   JLed lifecycle demo
//   OnStart
//   OnFirstOutput - first output
//   OnRepeatStart
//   OnEnterDelayAfter
//   OnRepeatStart
//   OnEnterDelayAfter
//   OnRepeatStart
//   OnEnterDelayAfter
//   OnRepeatStart
//   OnEnterDelayAfter
//   OnRepeatStart
//   OnEnterDelayAfter
//   OnDone
//
#include <jled.h>

JLed::brightness_t max_br = 20;
auto led = JLed(LED_BUILTIN).FadeOn(2000).DelayAfter(1000).MaxBrightness(max_br).Repeat(5);

void setup() {
    Serial.begin(9600);
    const auto start = millis();
    while (!Serial && millis() - start < 2000) {
    }
    Serial.println("JLed lifecycle demo");
}

void loop() {
    led.Update()
        .OnStart([](JLed*) { Serial.println("OnStart"); })
        .OnFirstOutput([](JLed*) { Serial.println("OnFirstOutput - first output"); })
        .OnRepeatStart([](JLed*) { Serial.println("OnRepeatStart"); })
        // every time the delay-after phase begins, increase the max brightness and
        // turn the LED off for the duration of the delay-after phase
        .OnEnterDelayAfter([](JLed* l) {
            Serial.println("OnEnterDelayAfter");
            max_br += 40;
            l->MaxBrightness(max_br);
            l->WriteRaw(0);
        })
        // when finally done, turn the LED off
        .OnDone([](JLed* l) {
            Serial.println("OnDone");
            l->Stop();
        });
}
