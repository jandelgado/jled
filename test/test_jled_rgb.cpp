// Unit tests for JLedRGB, effects end to end.
// Copyright 2026 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include "catch2/catch_amalgamated.hpp"
#include "value_rgb.h"  // NOLINT
#include "hal_mock.h"   // NOLINT
#include "jled_rgb.h"   // NOLINT

using jled::RGBColor;
using jled::TJLedRGB;

// TestJLedRGB is jled::TJLedRGB (src/jled_rgb.h) instantiated over
// RGBHal<HalMock> instead of RGBHal<JLedHal>, so it can be driven and
// inspected on the host like TestJLed in test_jled.cpp.
class TestJLedRGB : public TJLedRGB<HalMock, TimeMock, RGBColor<uint8_t>, TestJLedRGB> {
    using Base = TJLedRGB<HalMock, TimeMock, RGBColor<uint8_t>, TestJLedRGB>;

 public:
    using Base::Base;
};

TEST_CASE("JLedRGB Set(color) writes the exact color, no conversion", "[jled_rgb]") {
    // Set() uses ConstantBrightnessEvaluator, which stays RGB-native: no
    // conversion at all, so any RGBColor<uint8_t> comes out exactly as given.
    const RGBColor<uint8_t> red{255, 0, 0};
    auto led = TestJLedRGB(1, 2, 3);
    led.Set(red);
    TimeMock::set_millis(0);
    led.Update();
    CHECK(led.GetHal().r().Value() == red.r);
    CHECK(led.GetHal().g().Value() == red.g);
    CHECK(led.GetHal().b().Value() == red.b);
}

TEST_CASE("JLedRGB FadeOn(duration, color) fades on TO color, from black", "[jled_rgb]") {
    // FadeOn(duration, to = kOnColor(), from = kOffColor()): the single
    // positional color argument is the destination, matching what the name
    // "FadeOn" implies ("fade on, to this color") - not the starting point.
    const RGBColor<uint8_t> green{0, 255, 0};
    auto led = TestJLedRGB(1, 2, 3).FadeOn(100, green);

    TimeMock::set_millis(0);
    led.Update();
    CHECK(led.GetHal().r().Value() == 0);
    CHECK(led.GetHal().g().Value() == 0);
    CHECK(led.GetHal().b().Value() == 0);

    TimeMock::set_millis(99);
    led.Update();
    CHECK(led.GetHal().r().Value() == green.r);
    CHECK(led.GetHal().g().Value() == green.g);
    CHECK(led.GetHal().b().Value() == green.b);
}

TEST_CASE("JLedRGB Fade hits exact endpoints and takes the FadeOff branch on an equal-sum tie",
          "[jled_rgb]") {
    // ValueTraits<RGBColor<T>>::IsBrighter compares channel sums: green
    // (0,255,0) and red (255,0,0) both sum to 255, so this is a tie and
    // (per IsBrighter's strict <) takes the FadeOff branch.
    const RGBColor<uint8_t> green{0, 255, 0};
    const RGBColor<uint8_t> red{255, 0, 0};
    auto led = TestJLedRGB(1, 2, 3).Fade(green, red, 100);

    TimeMock::set_millis(0);
    led.Update();
    CHECK(led.GetHal().r().Value() == green.r);
    CHECK(led.GetHal().g().Value() == green.g);

    TimeMock::set_millis(99);
    led.Update();
    // Blending is per channel, so the endpoint is exactly the given color.
    CHECK(led.GetHal().r().Value() == red.r);
    CHECK(led.GetHal().g().Value() == red.g);
}

TEST_CASE("JLedRGB Candle() writes the exact color, no conversion", "[jled_rgb]") {
    // jitter == 0 makes candle_func always return kMaxValue() (255, see
    // jled_effects.cpp), so Blend(255, color_off, color_on) == color_on and
    // the written color is independent of offset - so an arbitrary literal
    // offset (0) is passed. CandleBrightnessEvaluator stays RGB-native
    // (Blend interpolates each channel independently), so amber comes out
    // exactly as given.
    const RGBColor<uint8_t> amber{255, 147, 41};
    auto led = TestJLedRGB(1, 2, 3).Candle(amber,
                                           /*color_off=*/RGBColor<uint8_t>{0, 0, 0},
                                           /*speed=*/6,
                                           /*jitter=*/0,
                                           /*period=*/0xffff,
                                           /*offset=*/0);

    TimeMock::set_millis(0);
    led.Update();
    CHECK(led.GetHal().r().Value() == amber.r);
    CHECK(led.GetHal().g().Value() == amber.g);
    CHECK(led.GetHal().b().Value() == amber.b);
}

TEST_CASE("JLedRGB Candle() flickers towards color_off, not implicit black", "[jled_rgb]") {
    // Same slot=0 scenario as CandleBrightnessEvaluator's table lookup test
    // in test_jled.cpp (factor=21 at t=0), driven end-to-end through
    // JLedRGB::Candle() with a non-black color_off: a red/yellow flicker.
    // Both endpoints share R=255 and B=0, so those channels staying pinned
    // at the low factor - rather than dimming towards 0 like the old
    // implicit-black behavior would - is what proves color_off took effect.
    const RGBColor<uint8_t> red{255, 0, 0};
    const RGBColor<uint8_t> yellow{255, 255, 0};
    auto led = TestJLedRGB(1, 2, 3).Candle(yellow,
                                           red,
                                           /*speed=*/0,
                                           /*jitter=*/255,
                                           /*period=*/1000,
                                           /*offset=*/0);
    TimeMock::set_millis(0);
    led.Update();
    CHECK(led.GetHal().r().Value() == 255);
    CHECK(led.GetHal().g().Value() == jled::lerp_ordered<uint8_t>(21, 0, 255));
    CHECK(led.GetHal().b().Value() == 0);
}

TEST_CASE("JLedRGB Candle() derives a per-instance automatic offset", "[jled_rgb]") {
    // speed == 0 makes every tick its own candle_func slot and jitter == 255
    // makes practically every slot flicker, so each LED's output is a
    // pseudo-random sequence phase-shifted by its own auto offset. Two
    // instances have different addresses, hence different offsets, hence
    // different sequences. With the defaults (speed 6, jitter 15) the slot is
    // constant over a short window and neither LED flickers at all, so both
    // would sit at full brightness and this would not discriminate.
    //
    // offset is passed explicitly as 0xffff - that's the same value Candle()'s
    // own default already uses, so this still exercises the auto-derivation
    // path under test, not a fixed offset.
    const RGBColor<uint8_t> amber{255, 147, 41};
    // Named objects, not builder chains off temporaries: TJLed::Candle derives
    // the offset from `this` at call time, and two temporaries in the same
    // scope routinely reuse one stack slot, which would hand both LEDs the
    // same address and therefore the same offset.
    TestJLedRGB led1(1, 2, 3);
    TestJLedRGB led2(4, 5, 6);
    led1.Candle(amber,
                RGBColor<uint8_t>{0, 0, 0},
                /*speed=*/0,
                /*jitter=*/255,
                /*period=*/0xffff,
                /*offset=*/0xffff);
    led2.Candle(amber,
                RGBColor<uint8_t>{0, 0, 0},
                /*speed=*/0,
                /*jitter=*/255,
                /*period=*/0xffff,
                /*offset=*/0xffff);

    bool any_pin_differs = false;
    for (uint32_t t = 0; t < 50; t++) {
        TimeMock::set_millis(t);
        led1.Update();
        led2.Update();
        if (led1.GetHal().r().Value() != led2.GetHal().r().Value()) any_pin_differs = true;
    }
    CHECK(any_pin_differs);  // different instance addresses -> different auto offsets
}

