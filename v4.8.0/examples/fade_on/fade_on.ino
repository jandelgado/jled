// JLed delayed fade-on demo. Fades on LED after 2 seconds.
// Copyright 2017 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include <jled.h>

// LED is connected to pin 9 (PWM capable) gpio
auto led = JLed(9).FadeOn(1000).DelayBefore(2000);

void setup() {
}

void loop() {
  led.Update();
}
