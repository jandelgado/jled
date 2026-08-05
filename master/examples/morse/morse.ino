// Copyright (c) 2019 Jan Delgado <jdelgado[at]gmx.net>
// JLed user defined brightness morse example
// https://github.com/jandelgado/jled
#include <Arduino.h>
#include <jled.h>

#include "morse.h"         // NOLINT
#include "morse_effect.h"  // NOLINT

MorseEffect morseEffect("HELLO JLED");
auto morseLed = JLed(LED_BUILTIN).UserFunc(&morseEffect).DelayAfter(2000).Forever();

void setup() {}

void loop() {
    morseLed.Update();
}
