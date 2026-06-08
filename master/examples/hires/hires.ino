// JLed high resolution demo
// This demo needs to be run on a MCU that supports higher resolution PWM like the RP2040 or ESP32
// See the platform section in README.md for specific limitations regarding PWM, e.g., not
// every board supports multiple resolutions simultanously.
// Copyright 2026 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include <jled.h>

// slowly breathe the first LED with the default 8-bit resolution. The brightness steps
// are clearly visible in the beginning and the end of the animation.
auto led1 = JLed(16).Breathe(25000).Forever();

// the second LED leverages the full PWM resolution that the MCU supports. The high
// PWM resolution results in a smooth effect.
auto led2 = JLedHD(18).Breathe(25000).Forever();

void setup() {
}

void loop() {
  led1.Update();
  led2.Update();
}
