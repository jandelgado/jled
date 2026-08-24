// Unit tests for JLedRGBHD, the RGBColor<uint16_t> RGB LED.
// Copyright 2026 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include "catch2/catch_amalgamated.hpp"
#include "value_rgb.h"  // NOLINT
#include "hal_mock.h"   // NOLINT
#include "jled_rgb.h"   // NOLINT

using jled::RGBColor;
using jled::TJLedRGB;

// TestJLedRGBHD is jled::TJLedRGB (src/jled_rgb.h) instantiated over
// RGBHal<HalMock> instead of RGBHal<JLedHalHD>, so it can be driven and
// inspected on the host, exactly like TestJLedRGB in test_jled_rgb.cpp.
class TestJLedRGBHD : public TJLedRGB<HalMock, TimeMock, RGBColor<uint16_t>, TestJLedRGBHD> {
    using Base = TJLedRGB<HalMock, TimeMock, RGBColor<uint16_t>, TestJLedRGBHD>;

 public:
    using Base::Base;
};

TEST_CASE("JLedRGBHD On() is white at full 16-bit value", "[jled_rgb_hd]") {
    auto led = TestJLedRGBHD(1, 2, 3);
    led.On();
    TimeMock::set_millis(0);
    led.Update();
    CHECK(led.GetHal().r().Value() == 65535);
    CHECK(led.GetHal().g().Value() == 65535);
    CHECK(led.GetHal().b().Value() == 65535);
}

TEST_CASE("JLedRGBHD Set(color) writes the exact color, no conversion", "[jled_rgb_hd]") {
    // ConstantBrightnessEvaluator stays RGB-native: no conversion at all.
    const RGBColor<uint16_t> color{10000, 65535, 40000};
    auto led = TestJLedRGBHD(1, 2, 3);
    led.Set(color);
    TimeMock::set_millis(0);
    led.Update();
    CHECK(led.GetHal().r().Value() == color.r);
    CHECK(led.GetHal().g().Value() == color.g);
    CHECK(led.GetHal().b().Value() == color.b);
}

TEST_CASE("JLedRGBHD FadeOn resolves brightness below the 8-bit step size", "[jled_rgb_hd]") {
    // Per-channel Blend on a black -> white fade reduces to lerp_ordered on
    // each channel between 0 and 65535, the full 16-bit range: two
    // consecutive ticks mid-fade write different values that share the same
    // high byte, a difference an 8-bit JLedRGB could not represent at all.
    const RGBColor<uint16_t> black{0, 0, 0};
    const RGBColor<uint16_t> white{65535, 65535, 65535};
    auto led = TestJLedRGBHD(1, 2, 3).FadeOn(1000, white, black);
    TimeMock::set_millis(0);
    led.Update();  // anchors the effect's start time at t == 0

    TimeMock::set_millis(500);
    led.Update();
    CHECK(led.GetHal().r().Value() == 17545);

    TimeMock::set_millis(501);
    led.Update();
    CHECK(led.GetHal().r().Value() == 17635);  // 17545 and 17635 both have high byte 68
}

TEST_CASE("JLedRGBHD FadeOn from a near-gray 16-bit color blends per channel",
          "[jled_rgb_hd]") {
    // r (1300 -> 0) and b (1299 -> 0) are lerped independently, so they can
    // differ by a rounding step (at most 1) while both descend; g (1301 ->
    // 65535) rises above both throughout and lands exactly on the endpoint.
    const RGBColor<uint16_t> near_gray{1300, 1301, 1299};
    const RGBColor<uint16_t> green{0, 65535, 0};
    auto led = TestJLedRGBHD(1, 2, 3).FadeOn(100, green, near_gray);

    for (uint32_t t = 0; t < 100; t++) {
        TimeMock::set_millis(t);
        led.Update();
        const auto r = led.GetHal().r().Value();
        const auto b = led.GetHal().b().Value();
        CHECK((r > b ? r - b : b - r) <= 1);
        CHECK(led.GetHal().g().Value() >= r);
    }
    CHECK(led.GetHal().r().Value() == green.r);
    CHECK(led.GetHal().g().Value() == green.g);
    CHECK(led.GetHal().b().Value() == green.b);
}

TEST_CASE("JLedRGBHD Candle() writes the exact color, no conversion", "[jled_rgb_hd]") {
    // jitter == 0 makes candle_func always return kMaxValue() (255, see
    // jled_effects.cpp), so Blend(full, color_off, color_on) == color_on and
    // the written color is independent of offset - so an arbitrary literal
    // offset (0) is passed. CandleBrightnessEvaluator stays RGB-native, so
    // amber comes out exactly as given.
    const RGBColor<uint16_t> amber{60000, 40000, 8000};
    auto led = TestJLedRGBHD(1, 2, 3).Candle(amber,
                                             /*color_off=*/RGBColor<uint16_t>{0, 0, 0},
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
