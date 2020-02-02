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
    JLed leds[] = {JLed(PB_5).Blink(750, 250).Forever(),
                   JLed(LED1).Breathe(2000).Forever(),
                   JLed(PB_10).FadeOff(1000).Forever(),
                   JLed(PB_6).FadeOn(1000).Forever(),
                   JLed(PC_7).Blink(500, 500).Forever()};

    JLedSequence sequence(JLedSequence::eMode::PARALLEL, leds);

    while (1) {
        sequence.Update();
    }
    return 0;
}
