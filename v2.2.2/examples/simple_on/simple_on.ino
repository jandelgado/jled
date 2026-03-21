// JLed delayed turn-on demo. Turns on built-in LED after 2 seconds.
// Copyright 2017 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include <jled.h>

JLed led = JLed(LED_BUILTIN).On().DelayBefore(2000);

void setup() {
}

void loop() {
  led.Update();
}
