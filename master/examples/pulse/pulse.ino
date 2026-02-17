// JLed pulse demo.
// Copyright 2022 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include <jled.h>

// "pulse" LED on GPIO pin 5. The "pulse" is accomplished by setting
// a minimal brightness so the LED will not be dark.
auto led = JLed(5).Breathe(2000).MinBrightness(20).Forever().DelayAfter(500);

void setup() {}

void loop() { led.Update(); }
