// JLed dual led demo.
// Copyright 2017 by Jan Delgado. All rights reserved.
#include <jled.h>

// the LED connected to gpio 9 is in breathe mode, the builtin LED
// LED is blinking with the same frequency.
JLed led1 = JLed(9).Breathe(2000).Forever().DelayAfter(2000);
JLed led2 = JLed(LED_BUILTIN).Blink(2000,2000).Forever();

void setup() {
}

void loop() {
  led1.Update();
  led2.Update();
}
