// JLed Unit tests for the mbed_hal class (run on host).
// Copyright 2020 Jan Delgado jdelgado@gmx.net
#include "catch.hpp"

#include <mbed_hal.h>  // NOLINT
#include "mbed.h"      // NOLINT

using jled::MbedHal;

TEST_CASE("mbed_hal outputs 0 as 0 to the given pin using PwmOut",
          "[mbed_hal]") {
    mbedMockInit();
    constexpr auto kPin = 5;
    auto hal = MbedHal(kPin);

    hal.analogWrite(0);

    REQUIRE(mbedMockGetPinState(kPin) == 0.);
}

TEST_CASE("mbed_hal outputs 255 as 1.0 to the given pin using PwmOut",
          "[mbed_hal]") {
    mbedMockInit();
    constexpr auto kPin = 5;
    auto hal = MbedHal(kPin);

    hal.analogWrite(255);

    REQUIRE(mbedMockGetPinState(kPin) == 1.);
}

TEST_CASE("mbed_hal writes scaled value to the given pin using PwmOut",
          "[mbed_hal]") {
    mbedMockInit();
    constexpr auto kPin = 5;
    auto hal = MbedHal(kPin);

    hal.analogWrite(127);

    REQUIRE(mbedMockGetPinState(kPin) == Approx(127 / 255.));
}
