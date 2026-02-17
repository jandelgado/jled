// JLed user provided brightness function demo.
// Copyright 2017 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include <jled.h>

// this function returns changes between 0 and 255 and vice versa every 250 ms.
uint8_t blinkFunc(uint32_t t, uint16_t, uintptr_t) {
  return 255*((t/250)%2);
}

// Run blinkUserFunc for 5000ms
JLed led = JLed(LED_BUILTIN).UserFunc(blinkFunc, 5000);

void setup() {
}

void loop() {
  led.Update();
}
