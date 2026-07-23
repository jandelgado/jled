// JLed accelerating blink example using lifecycle events.
// Copyright 2026 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include <jled.h>

constexpr auto kMaxBlinkTime = 500;
constexpr auto kMinBlinkTime = 20;
constexpr auto kTimes = 1;
uint16_t blink_time = kMaxBlinkTime;  // ms
auto accelerate = true;

auto led = JLed(LED_BUILTIN).Blink(blink_time, blink_time, kTimes);

void setup() {
}

void loop() {
    led.Update().OnDone([](JLed* l) {
        // after each complete run of the effect, change the blink time and start over
        if (accelerate) {
            blink_time = (blink_time * 9) / 10;
            if (blink_time < kMinBlinkTime) {
                blink_time = kMinBlinkTime;
                accelerate = false;
            }
        } else {
            // decelerate
            blink_time = (blink_time * 10) / 9;
            if (blink_time > kMaxBlinkTime) {
                blink_time = kMaxBlinkTime;
                accelerate = true;
            }
        }
        l->Blink(blink_time, blink_time, kTimes);
    });
}
