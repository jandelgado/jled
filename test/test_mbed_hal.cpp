// JLed Unit tests for the mbed_hal class (run on host).
// Copyright 2020 Jan Delgado jdelgado@gmx.net
#include "catch2/catch_amalgamated.hpp"

#include <mbed_hal.h>  // NOLINT
#include "mbed.h"      // NOLINT

using jled::MbedHal;

TEST_CASE("mbed_hal scales uint16 to [0..1f]", "[mbed_hal]") {
    mbedMockInit();
    constexpr auto kPin = 5;
    auto hal = MbedHal(kPin);

    hal.analogWrite(0);
    REQUIRE(mbedMockGetPinState(kPin) == 0.);

    hal.analogWrite(65535);
    REQUIRE(mbedMockGetPinState(kPin) == 1.);

    hal.analogWrite(32767);
    REQUIRE_THAT(mbedMockGetPinState(kPin),
                 Catch::Matchers::WithinAbs(.5, 0.00001));
}

