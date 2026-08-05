// JLed candle effect
// Copyright 2019 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include <jled.h>

auto led = JLed(5).Candle().Forever();

// change speed and jitter to turn the candle into a fire
// auto led = JLed(5).Candle(5 /* speed */, 100 /*jitter*/).Forever();

void setup() {
}

void loop() {
    led.Update();
}
