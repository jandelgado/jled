// JLed fade from-to example. Example randomly fades to a new level with
// a random duration.
// Copyright 2022 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include <jled.h>

auto led = JLed(5).On(1);  // start with LED turned on

void setup() {}

void loop() {
    static JLed::brightness_t last_to = 255;  // uint8_t for JLed, uint16_t for JLedHD

    if (!led.Update()) {
        // when effect is done (Update() returns false),
        // reconfigure fade effect using random values
        auto new_from = last_to;
        auto new_to = random(256);
        auto duration = 250 + random(256) * 4;
        last_to = new_to;
        led.Fade(new_from, new_to, duration).Repeat(1);
    }
}
