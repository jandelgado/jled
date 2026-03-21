// JLed breathe demo.
// Copyright 2017 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include <jled.h>

// breathe LED for 5 times, LED is connected to pin 9 (PWM capable) gpio
auto led = JLed(9).Breathe(2000).Repeat(5).DelayAfter(2000);

void setup() {
}

void loop() {
  led.Update();
}
