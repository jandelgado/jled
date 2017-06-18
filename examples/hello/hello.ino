// JLed 'hello, world.'. Blinks built in LED.
// Copyright 2017 by Jan Delgado. All rights reserved.
#include <jled.h>

JLed led = JLed(LED_BUILTIN).Blink(1000, 1000).Forever();

void setup() {
}

void loop() {
  led.Update();
}
