// JLed 'hello, rgb.'. Blinks an RGB LED red/green 5 times.
// Copyright 2026 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include <jled.h>

// gives access to RGB color constants kRed, kGreen etc.
using namespace jled::color;  // NOLINT

// blink RGB LED connected to pins 13(Red), 14(Green), 15(Blue) red/green 5 times.
// LED is low-active ("common anode"), hence LowActive() is called.
auto led = JLedRGB(13, 14, 15).Blink(1000, 1000, 5, kRed, kGreen).LowActive();

void setup() {}

void loop() {
    led.Update();
}
