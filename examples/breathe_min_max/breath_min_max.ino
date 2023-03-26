// JLed breathe demo.
// Copyright 2017 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include <jled.h>

int minbrightness = 10;
int maxbrightness = 200;

// breathe LED forever from minimum to maximum, LED is connected to pin 9 (PWM capable) gpio
// LED will take 4 seconds to go from minimum to maximum and 4 seconds from maximum to minimum
auto led = JLed(9).Breathe(4000).MinBrightness(minbrightness).MaxBrightness(maxbrightness).Forever();

void setup() {
}

void loop() {
  led.Update();
}
