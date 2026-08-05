// Toggle pause and resume of a breathing LED effect with a button press.
// Each press freezes the LED at its current brightness; the next press
// continues the effect from that exact point.
//
// dependency: arduinogetstarted/ezButton@1.0.6 to control the button
//
// Copyright 2026 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
//
#include <ezButton.h>  // arduinogetstarted/ezButton@1.0.6
#include <jled.h>

constexpr auto LED_PIN = 16;
constexpr auto BUTTON_PIN = 18;

auto button = ezButton(BUTTON_PIN);
auto led = JLed(LED_PIN).Breathe(2000).Forever();

void setup() {}

void loop() {
    button.loop();

    if (button.isPressed()) {
        if (led.IsPaused()) {
            led.Resume();
        } else {
            led.Pause();
        }
    }

    led.Update();
}
