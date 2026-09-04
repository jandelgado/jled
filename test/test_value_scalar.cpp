// Copyright (c) 2025-2026 Jan Delgado <jdelgado[at]gmx.net>
// https://github.com/jandelgado/jled
//
// Unit tests for the scalar ValueTraits + Percentage (value_scalar.h)
//
#include <catch2/catch_amalgamated.hpp>

#include "value_scalar.h"  // NOLINT

using jled::Percentage;
using jled::operator""_pct;

TEST_CASE("Percentage - converts to uint8_t", "[value_scalar]") {
    REQUIRE(static_cast<uint8_t>(Percentage(0)) == 0);
    REQUIRE(static_cast<uint8_t>(Percentage(1)) == 2);     // 1*255/100
    REQUIRE(static_cast<uint8_t>(Percentage(50)) == 127);  // 50*255/100
    REQUIRE(static_cast<uint8_t>(Percentage(75)) == 191);  // 75*255/100
    REQUIRE(static_cast<uint8_t>(Percentage(99)) == 252);  // 99*255/100
    REQUIRE(static_cast<uint8_t>(Percentage(100)) == 255);
}

TEST_CASE("Percentage - converts to uint16_t", "[value_scalar]") {
    REQUIRE(static_cast<uint16_t>(Percentage(0)) == 0);
    REQUIRE(static_cast<uint16_t>(Percentage(1)) == 655);     // 1*65535/100
    REQUIRE(static_cast<uint16_t>(Percentage(50)) == 32767);  // 50*65535/100
    REQUIRE(static_cast<uint16_t>(Percentage(75)) == 49151);  // 75*65535/100
    REQUIRE(static_cast<uint16_t>(Percentage(99)) == 64879);  // 99*65535/100
    REQUIRE(static_cast<uint16_t>(Percentage(100)) == 65535);
}

TEST_CASE("Percentage - _pct literal matches Percentage constructor", "[value_scalar]") {
    REQUIRE(static_cast<uint8_t>(0_pct) == static_cast<uint8_t>(Percentage(0)));
    REQUIRE(static_cast<uint8_t>(50_pct) == static_cast<uint8_t>(Percentage(50)));
    REQUIRE(static_cast<uint8_t>(100_pct) == static_cast<uint8_t>(Percentage(100)));

    REQUIRE(static_cast<uint16_t>(0_pct) == static_cast<uint16_t>(Percentage(0)));
    REQUIRE(static_cast<uint16_t>(50_pct) == static_cast<uint16_t>(Percentage(50)));
    REQUIRE(static_cast<uint16_t>(100_pct) == static_cast<uint16_t>(Percentage(100)));
}

TEST_CASE("ValueTraits scalar no-op properties", "[value_scalar]") {
    using jled::ValueTraits;
    SECTION("kOnColor equals kMaxValue for scalars") {
        CHECK(ValueTraits<uint8_t>::kOnColor() == ValueTraits<uint8_t>::kMaxValue());
        CHECK(ValueTraits<uint16_t>::kOnColor() == ValueTraits<uint16_t>::kMaxValue());
    }
    SECTION("ApplyBounds and Blend reduce to lerp") {
        CHECK(ValueTraits<uint8_t>::ApplyBounds(128, 10, 200) == jled::lerp<uint8_t>(128, 10, 200));
        CHECK(ValueTraits<uint8_t>::Blend(64, 10, 200) == jled::lerp<uint8_t>(64, 10, 200));
    }
    SECTION("IsBrighter is a strict less-than") {
        CHECK(ValueTraits<uint8_t>::IsBrighter(10, 20));
        CHECK_FALSE(ValueTraits<uint8_t>::IsBrighter(20, 20));
        CHECK_FALSE(ValueTraits<uint8_t>::IsBrighter(20, 10));
    }
}

TEST_CASE("ValueTraits<uint8_t>::Blend ramps a descending pair monotonically", "[value_scalar]") {
    // Regression: the generic Breathe(fade_on, on, fade_off, from, to)
    // builder never reorders from/to (unlike FadeOn/FadeOff), so a scalar
    // JLed can hit Blend with from > to. Blend used the plain lerp<T>, which
    // computes delta = to - from in unsigned T and assumes to >= from, so a
    // descending pair wrapped delta and made the value climb past 255
    // instead of descending (200 -> 253 -> wraps to 10), instead of ramping.
    using jled::ValueTraits;

    CHECK(ValueTraits<uint8_t>::Blend(0, 200, 50) == 200);
    CHECK(ValueTraits<uint8_t>::Blend(255, 200, 50) == 50);

    int prev = 256;
    for (int alpha = 0; alpha <= 255; alpha++) {
        const int v = ValueTraits<uint8_t>::Blend(static_cast<uint8_t>(alpha), 200, 50);
        CHECK(v <= prev);
        CHECK(v >= 50);
        prev = v;
    }
}
