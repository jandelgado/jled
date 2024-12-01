// Stops an effect when a button is pressed (and hold). When the button is
// released, the LED will fade to off with starting the brightness value it had
// when the effect was stopped.
//
// dependency: arduinogetstarted/ezButton@1.0.6 to control the button
//
// Copyright 2024 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
//
#include <ezButton.h>  // arduinogetstarted/ezButton@1.0.6
#include <jled.h>

constexpr auto LED_PIN = 16;
constexpr auto BUTTON_PIN = 18;

auto button = ezButton(BUTTON_PIN);

// start with a pulse effect
auto led =
    JLed(LED_PIN).DelayBefore(1000).Breathe(2000).Forever().MinBrightness(25);

void setup() {}

void loop() {
    static int16_t lastBrightness = 0;

    button.loop();
    led.Update(&lastBrightness);

    if (button.isPressed()) {
        // when the button is pressed, stop the effect on led, but keep the LED
        // on with it's current brightness ...
        led.Stop(JLed::KEEP_CURRENT);
    } else if (button.isReleased()) {
        // when the button is released, fade from the last brightness to 0
        led = JLed(LED_PIN).Fade(lastBrightness, 0, 1000);
    }
}
