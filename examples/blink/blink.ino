// JLed blink example. Blinks built in LED 3 times, then waits 1s. Repeats forever.
// Copyright 2026 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include <jled.h>

// blink builtin LED 3 times (250ms on, 500ms off), wait 1000ms, repeat forever
auto led = JLed(LED_BUILTIN).Blink(250, 500, 3).DelayAfter(1000).Forever();

void setup() {
}

void loop() {
  led.Update();
}
