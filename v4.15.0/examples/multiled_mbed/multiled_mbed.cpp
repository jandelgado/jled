// JLed multi LED demo for mbed. Controls multiple LEDs parallel in-sync.
//
// mbed version tested with ST Nucleo F401RE
//
// See https://os.mbed.com/platforms/ST-Nucleo-F401RE/ for pin names and
// assignments.
//
// Copyright 2020 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
//
#include <jled.h>
#include <mbed.h>

int main() {
    JLed leds[] = {JLed(LED1).Blink(750, 250).Forever(),
                   JLed(PA_8).Breathe(2000).Forever(),
                   JLed(PB_10).FadeOff(1000).Forever(),
                   JLed(PB_4).FadeOn(1000).Forever(),
                   JLed(PB_3).Blink(500, 500).Forever()};

    JLedSequence sequence(JLedSequence::eMode::PARALLEL, leds);

    while (1) {
        sequence.Update();
    }
    return 0;
}
