// Copyright (c) 2026 Jan Delgado <jdelgado[at]gmx.net>
// https://github.com/jandelgado/jled
//
// Unit tests for RGBColor<T>, ValueTraits<RGBColor<T>>, and hsv_to_rgb
// conversion (value_rgb.h/.cpp)
//
#include "catch2/catch_amalgamated.hpp"
#include "value_rgb.h"  // NOLINT

using jled::ValueTraits;
using jled::HSV;
using jled::hsv_to_rgb;
using jled::RGBColor;

TEST_CASE("hsv_to_rgb: full value, zero saturation is white regardless of hue", "[value_rgb]") {
    for (int h = 0; h <= 255; h += 5) {
        const auto rgb = hsv_to_rgb(HSV<uint8_t>{static_cast<uint8_t>(h), 0, 255});
        CHECK(rgb.r == 255);
        CHECK(rgb.g == 255);
        CHECK(rgb.b == 255);
    }
}

TEST_CASE("hsv_to_rgb: zero value is black regardless of hue/saturation", "[value_rgb]") {
    const auto rgb = hsv_to_rgb(HSV<uint8_t>{123, 200, 0});
    CHECK(rgb.r == 0);
    CHECK(rgb.g == 0);
    CHECK(rgb.b == 0);
}

TEST_CASE("hsv_to_rgb: hue 0 at full saturation and value is red", "[value_rgb]") {
    // Red is hue 0: sector 0 at offset 0, so chroma is exactly {255, 0, 0}.
    auto red = hsv_to_rgb(HSV<uint8_t>{0, 255, 255});
    CHECK(red.r > red.g);
    CHECK(red.r > red.b);
}

TEST_CASE("hsv_to_rgb: a non-degenerate rainbow sector (sector 2, yellow->green)", "[value_rgb]") {
    // Neither the s==0 nor the v==0 shortcut applies here, so this pins the
    // chromaticity table itself. Hue 85: bit 0x80 clear, bit 0x40 set, bit
    // 0x20 clear -> sector 2. offset = 85 & 0x1f = 21, offset8 = 21 << 3 = 168,
    // third = scale8(168, 85) = (168*86)>>8 = 56,
    // two_third = scale8(168, 170) = (168*171)>>8 = 112.
    // Sector 2 is r = 171 - two_third, g = 170 + third, b = 0.
    const auto rgb = hsv_to_rgb(HSV<uint8_t>{85, 255, 255});
    CHECK(static_cast<int>(rgb.r) == 59);   // 171 - 112
    CHECK(static_cast<int>(rgb.g) == 226);  // 170 + 56
    CHECK(static_cast<int>(rgb.b) == 0);
}

TEST_CASE("hsv_to_rgb: 0 < v < 255 scales every channel by v", "[value_rgb]") {
    // The value-scaling branch every Breathe/Fade frame will take. Hue 0 at
    // full saturation is chroma {255, 0, 0}, then each channel is scaled by v:
    // scale8(255, 128) = (255*129)>>8 = 128.
    const auto rgb = hsv_to_rgb(HSV<uint8_t>{0, 255, 128});
    CHECK(static_cast<int>(rgb.r) == 128);
    CHECK(static_cast<int>(rgb.g) == 0);
    CHECK(static_cast<int>(rgb.b) == 0);
}

TEST_CASE("hsv_to_rgb<uint16_t>: full value is exact bit-replication of the 8-bit chroma",
          "[value_rgb]") {
    const auto rgb16 = jled::hsv_to_rgb(HSV<uint16_t>{0, 65535, 65535});
    const auto rgb8 = jled::hsv_to_rgb(HSV<uint8_t>{0, 255, 255});
    CHECK(rgb16.r == static_cast<uint16_t>((rgb8.r << 8) | rgb8.r));
    CHECK(rgb16.g == static_cast<uint16_t>((rgb8.g << 8) | rgb8.g));
    CHECK(rgb16.b == static_cast<uint16_t>((rgb8.b << 8) | rgb8.b));
}

TEST_CASE("hsv_to_rgb<uint16_t>: v == 0 is black", "[value_rgb]") {
    const auto rgb16 = jled::hsv_to_rgb(HSV<uint16_t>{12345, 54321, 0});
    CHECK(rgb16.r == 0);
    CHECK(rgb16.g == 0);
    CHECK(rgb16.b == 0);
}

TEST_CASE("ValueTraits<RGBColor<uint8_t>>::kOffColor and kOnColor", "[value_rgb]") {
    CHECK(ValueTraits<RGBColor<uint8_t>>::kOffColor() == RGBColor<uint8_t>{0, 0, 0});
    CHECK(ValueTraits<RGBColor<uint8_t>>::kOnColor() == RGBColor<uint8_t>{255, 255, 255});
}

TEST_CASE("ValueTraits<RGBColor<uint8_t>>::ApplyBounds preserves hue, scales uniformly",
          "[value_rgb]") {
    const RGBColor<uint8_t> c{200, 100, 0};  // peak channel (r) = 200
    const auto bounded = ValueTraits<RGBColor<uint8_t>>::ApplyBounds(c, 0, 255);
    CHECK(bounded == c);  // identity pass-through at default bounds

    const auto bounded2 = ValueTraits<RGBColor<uint8_t>>::ApplyBounds(c, 10, 20);
    const uint8_t newPeak = jled::lerp<uint8_t>(200, 10, 20);
    CHECK(bounded2.r == newPeak);                  // peak channel hits newPeak exactly
    CHECK(bounded2.g == (100u * newPeak) / 200u);  // ratio to peak preserved
    CHECK(bounded2.b == 0);
}

TEST_CASE("ApplyBounds(kOffColor(), lo, hi) floors to gray {lo,lo,lo} for RGBColor",
          "[value_rgb]") {
    const auto result = ValueTraits<RGBColor<uint8_t>>::ApplyBounds(
        ValueTraits<RGBColor<uint8_t>>::kOffColor(), 30, 255);
    CHECK(result == RGBColor<uint8_t>{30, 30, 30});
}

TEST_CASE("ValueTraits<RGBColor<uint8_t>>::Blend interpolates each channel independently",
          "[value_rgb]") {
    // red -> yellow: R stays at max, G ramps up, B stays at 0. No hue
    // concept involved: a sweep between two differently-hued colors dips
    // through a desaturated midpoint rather than through hue space.
    const RGBColor<uint8_t> red{255, 0, 0};
    const RGBColor<uint8_t> yellow{255, 255, 0};
    const auto blended = ValueTraits<RGBColor<uint8_t>>::Blend(128, red, yellow);
    CHECK(blended.r == 255);
    CHECK(blended.g == jled::lerp_ordered<uint8_t>(128, 0, 255));
    CHECK(blended.b == 0);
}

TEST_CASE("ValueTraits<RGBColor<uint8_t>>::IsBrighter compares channel sums", "[value_rgb]") {
    CHECK(ValueTraits<RGBColor<uint8_t>>::IsBrighter(RGBColor<uint8_t>{0, 0, 10},
                                                     RGBColor<uint8_t>{0, 0, 20}));
    CHECK_FALSE(ValueTraits<RGBColor<uint8_t>>::IsBrighter(RGBColor<uint8_t>{10, 10, 10},
                                                           RGBColor<uint8_t>{5, 5, 5}));
}
