// Extra long running breathe example.
// Copyright 2026 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
//
// Normally an effect's period is limited to ca. 65s. However by scaling and explicitly
// passing the time to Update(), we can make the clock appear to run "slower" and
// have effects run longer.
#include <jled.h>

// make sure to use a MCU and GPIO that supports high resolution PWM. Otherwise
// brightness steps will become visible.
auto led = JLedHD(16).Breathe(65000).Forever();

void setup() {}

void loop() {
    // dividing the time by two effectively doubles the period
    led.Update(jled::JLedClockType::millis()/2);
}
