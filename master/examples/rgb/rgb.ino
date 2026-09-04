// JLedRGB demo showing RGB versions of various effects. A 3-leg RGB LED is connected
// to pins 13,14,15. The builtin LED blinks if the RGB effect changes.
//
// Copyright 2026 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include <jled.h>

// gives access to RGB color constants kRed, kGreen etc.
using namespace jled::color;    // NOLINT
using jled::HSV;
using jled::hsv_to_rgb;

constexpr auto kOnTime = 750;
constexpr auto kOffTime = 250;
constexpr auto kBlinkRepeat = 2;
constexpr auto kCandleTime = 10000;
constexpr auto kSweepTime = 15000;
constexpr auto kFadeTime = 5000;


// create an RGB led connected to pins 13(Red), 14(Green), 15(Blue). In loop()
// the LED will be reconfigured each time an effect ist done playing. A low active
// RGB-Led is used here, that why LowActive() must be called.
JLedRGB led = JLedRGB(13, 14, 15).Off(1).LowActive();
uint8_t effect_index = 0;

// signals when a new effect starts on the RGB LED
auto signal_led = JLed(LED_BUILTIN).Blink(250, 250);

void setup() {}

void loop() {
    signal_led.Update();
    led.Update().OnDone([](JLedRGB& l) {
        signal_led.Reset();
        // clang-format off
        switch (effect_index++) {
            default: effect_index = 1;  // fall through to 0
            case 0:  l.FadeOn(kFadeTime, kRed); break;
            case 1:  l.FadeOff(kFadeTime, kGreen); break;
            case 2:  l.FadeOn(kFadeTime, kBlue); break;
            case 3:  l.Candle(kRed, kBlack, 6, 15, kCandleTime); break;
            case 4:  l.Breathe(kFadeTime, kFadeTime/2, kFadeTime,  kBlack, kMagenta); break;
            case 5:  l.Candle(kRed, kYellow, 5, 100, kCandleTime); break;
            case 6:  l.Breathe(kFadeTime, 0, kFadeTime, kGreen, kBlue); break;
            case 7:  l.Blink(kOnTime, kOffTime, kBlinkRepeat, kRed, kBlack); break;
            case 8:  l.Blink(kOnTime, kOffTime, kBlinkRepeat, kGreen, kBlack); break;
            case 9:  l.Blink(kOnTime, kOffTime, kBlinkRepeat, kBlue, kBlack); break;
            case 10: l.Blink(kOnTime, kOffTime, kBlinkRepeat, kMagenta, kBlack); break;
            case 11: l.Blink(kOnTime, kOffTime, kBlinkRepeat, kCyan, kBlack); break;
            case 12: l.Blink(kOnTime, kOffTime, kBlinkRepeat, kYellow, kBlack); break;
            case 13: l.Blink(kOnTime, kOffTime, kBlinkRepeat, kWhite, kBlack); break;
            case 14: l.Fade(hsv_to_rgb(HSV<uint8_t>{0, 255, 255}),
                             hsv_to_rgb(HSV<uint8_t>{192, 255, 255}), kSweepTime); break;
            case 15: l.Blink(kOnTime, kOffTime, kBlinkRepeat, kRed, kBlue); break;
        }
        // clang-format on
    });
}
