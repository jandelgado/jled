// JLed 'hello, world.'. Blinks built in LED 5 times.
// Copyright 2017 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include <jled.h>

// blink builtin LED for 5 times
auto led2 = JLed16(16).Breathe(18000).Forever();
auto led3 = JLed(17).Breathe(18000).Forever();

void setup() {
}

void loop() {
  led2.Update();
  led3.Update();
}
