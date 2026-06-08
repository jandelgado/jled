// JLed delayed turn-on demo. Turns on built-in LED after 2 seconds.
// Copyright 2017 by Jan Delgado. All rights reserved.
#include <jled.h>

// LED is connected to pin 9 (PWM capable) gpio
JLed led = JLed(9);

void setup() {
  led.On().Update();
  led.FadeOff(2000).DelayBefore(5000);
}

void loop() {
  led.Update();
}
