// Unit tests for RGBHal.
// Copyright 2026 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include "catch2/catch_amalgamated.hpp"
#include "hal_mock.h"    // NOLINT
#include "rgb_hal.h"     // NOLINT

using jled::RGBColor;
using jled::RGBHal;

TEST_CASE("RGBHal::analogWrite forwards each channel's value and invert flag", "[rgb_hal]") {
    RGBHal<HalMock> hal(1, 2, 3);
    hal.analogWrite(RGBColor<uint8_t>{10, 20, 30}, true);

    CHECK(hal.r().Value() == 10);
    CHECK(hal.g().Value() == 20);
    CHECK(hal.b().Value() == 30);
    CHECK(hal.r().Invert());
    CHECK(hal.g().Invert());
    CHECK(hal.b().Invert());

    hal.analogWrite(RGBColor<uint8_t>{1, 2, 3}, false);
    CHECK_FALSE(hal.r().Invert());
    CHECK_FALSE(hal.g().Invert());
    CHECK_FALSE(hal.b().Invert());
}

TEST_CASE("RGBHal::SetLowActive reaches all three channels", "[rgb_hal]") {
    RGBHal<HalMock> hal(1, 2, 3);
    hal.SetLowActive(true);

    CHECK(hal.r().SetLowActiveCallCount() == 1);
    CHECK(hal.g().SetLowActiveCallCount() == 1);
    CHECK(hal.b().SetLowActiveCallCount() == 1);
    CHECK(hal.r().SetLowActiveValue());
    CHECK(hal.g().SetLowActiveValue());
    CHECK(hal.b().SetLowActiveValue());
}

TEST_CASE("RGBHal exposes each channel's own pin", "[rgb_hal]") {
    RGBHal<HalMock> hal(1, 2, 3);
    CHECK(hal.r().Pin() == 1);
    CHECK(hal.g().Pin() == 2);
    CHECK(hal.b().Pin() == 3);
}

TEST_CASE("RGBHal constructor is not explicit: copy-list-init compiles", "[rgb_hal]") {
    // The VM design depends on exactly this braced-init form.
    RGBHal<HalMock> hals[] = {{1, 2, 3}, {4, 5, 6}};
    CHECK(hals[1].r().Pin() == 4);
}
