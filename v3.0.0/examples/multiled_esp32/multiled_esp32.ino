// JLed ESP32 multi LED demo. control multiple LEDs in-sync.
// Copyright 2018 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include <jled.h>

// Note:
// JLed will automatically pick a new ledc-channel each time a JLed
// object is constructed. For full control, you can explicitly specify
// the details:
//   JLed led = JLed(Esp32AnalogWriter(4, 1)).FadeOff(1000);
// Where 4 is the pin where the led is connected to and 1 is the ledc
// channel to use.

JLed leds[] = {
    JLed(4).Breathe(2000).Forever(),
    JLed(2).Blink(750, 250).Forever(),
    JLed(15).FadeOff(1000).Forever()
};

void setup() {
}

void loop() {
    for (auto& led : leds) {led.Update();}
    delay(1);
}
