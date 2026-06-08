// JLed user provided brightness function demo using a lambda function.
// Copyright 2017 by Jan Delgado. All rights reserved.
#include <jled.h>

// this function returns changes between 0 and 255 and vice versa every 250 ms.
auto blinkFunc = [] (uint32_t t, uint16_t) -> uint8_t {return 255*((t/250)%2);};

// Run blinkUserFunc for 5000ms
JLed led = JLed(LED_BUILTIN).UserFunc(blinkFunc, 5000);

void setup() {
}

void loop() {
  led.Update();
}
