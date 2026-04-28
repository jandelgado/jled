// JLed 'hello, world.'. Blinks built in LED 5 times.
// Copyright 2017 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include <jled.h>

// blink builtin LED for 5 times
auto led = JLed(LED_BUILTIN).Blink(1000, 1000).Repeat(5);

void setup() {
}

void loop() {
  led.Update();
}
