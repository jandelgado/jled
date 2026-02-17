// JLed demo for the Raspberry Pi Pico
// Fade the built-in LED (GPIO 25) and a LED on GPIO 16.
//
// Copyright 2021 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
//
#include "pico/stdlib.h"    // NOLINT
#include "jled.h"           // NOLINT

int main() {
    auto led1 = JLed(25).FadeOff(2000).DelayAfter(1000).Forever();
    auto led2 = JLed(16).FadeOn(2000).DelayAfter(1000).Forever();

    while (true) {
        led1.Update();
        led2.Update();
    }
}
