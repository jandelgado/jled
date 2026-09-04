// Copyright (c) 2026 Jan Delgado <jdelgado[at]gmx.net>
// https://github.com/jandelgado/jled
//
// Unit tests for HSV<T> (value_hsv.h)
//
#include "catch2/catch_amalgamated.hpp"
#include "value_hsv.h"  // NOLINT

using jled::HSV;

TEST_CASE("HSV::WithV keeps hue and saturation, replaces value", "[value_hsv]") {
    constexpr HSV<uint8_t> c{10, 20, 30};
    CHECK(c.WithV(99) == HSV<uint8_t>{10, 20, 99});
}

TEST_CASE("HSV::WithH keeps saturation and value, replaces hue", "[value_hsv]") {
    constexpr HSV<uint8_t> c{10, 20, 30};
    CHECK(c.WithH(99) == HSV<uint8_t>{99, 20, 30});
}

TEST_CASE("HSV equality", "[value_hsv]") {
    CHECK(HSV<uint8_t>{1, 2, 3} == HSV<uint8_t>{1, 2, 3});
    CHECK(HSV<uint8_t>{1, 2, 3} != HSV<uint8_t>{1, 2, 4});
    CHECK_FALSE(HSV<uint8_t>{1, 2, 3} == HSV<uint8_t>{9, 2, 3});
}
