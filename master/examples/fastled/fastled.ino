// FastLED demo showing JLed driving FastLED by writing to an external CRGB[]
// array via WriterHal.
// Copyright 2026 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include <FastLED.h>
#include <jled.h>

constexpr auto FL_DATA_PIN = 6;  // Data pin of WS2812B LED stripe
constexpr auto NUM_LEDS = 8;     // number of LEDs
constexpr auto FL_MAX_BRIGHTNESS = 128;

// FastLedWriter is the bridge between the JLed WriterHAL and the FastLED CRGB[] color array.
// It is used by the FastLedHal HAL below. This is needed, because JLed does not know
// about FastLEDs CRGB type (or FastLED at all).
class FastLedWriter {
 public:
    static void Write(jled::RGBColor<uint8_t> val, CRGB* target) {
        target->r = val.r;
        target->g = val.g;
        target->b = val.b;
    }
};

// The "HAL" for FastLED simply writes out the calculated RGB triplets to the
// CRGB buffer we pass to FastLED using the provided FastLedWriter.
using FastLedHal = WriterHal<jled::RGBColor<uint8_t>, CRGB, FastLedWriter>;

// a JLed class to be used with a FastLED CRGB[] array
class JFastLed
    : public jled::TJLed<FastLedHal, jled::JLedClockType, jled::RGBColor<uint8_t>, JFastLed> {
    using Base = jled::TJLed<FastLedHal, jled::JLedClockType, jled::RGBColor<uint8_t>, JFastLed>;
 public:
    using Base::Base;
};

using JFastLedGroup = jled::TJLedGroup<jled::JLedClockType, JFastLed>;

// The shared color array: JLed writes, FastLED reads
CRGB leds[NUM_LEDS];

JFastLed group_leds[] = {
    JFastLed(&leds[0]).FadeOn(5000, jled::color::kOrangeRed).Forever(),
    JFastLed(&leds[1]).Fade(jled::color::kWhite, jled::color::kRed, 5000).Forever(),
    JFastLed(&leds[2]).Fade(jled::color::kBlue, jled::color::kLime, 5000).Forever(),
    JFastLed(&leds[3]).Fade(jled::color::kBlack, jled::color::kRed, 5000).Forever(),
    JFastLed(&leds[4]).Blink(500, 250, 1, jled::color::kLime, jled::color::kRed).Forever(),
    JFastLed(&leds[5]).Candle(jled::color::kYellow, jled::color::kRed).Forever(),
    JFastLed(&leds[6]).Breathe(5000, 1000, 5000, jled::color::kBlue, jled::color::kRed).Forever(),
    JFastLed(&leds[7]).FadeOff(5000, jled::color::kBlue).Forever(),
};

auto group = JFastLedGroup::Parallel(group_leds);

void setup() {
    FastLED.addLeds<WS2812B, FL_DATA_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(FL_MAX_BRIGHTNESS);
}

void loop() {
    group.Update();
    FastLED.show();
}
