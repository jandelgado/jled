// JLed breathe demo.
// Copyright 2017 by Jan Delgado. All rights reserved.
#include <jled.h>

// connect an LED to pin 9 (PWM capable) gpio
constexpr auto LED_PIN_BREATHE = 9;

// breate LED for 5 times.
JLed led = JLed(LED_PIN_BREATHE).Breathe(2000).Repeat(5).DelayAfter(1000);

void setup() {
}

void loop() {
  led.Update();
}
