// JLed Unit tests for the mbed_hal class (run on host).
// Copyright 2020 Jan Delgado jdelgado@gmx.net
#include "catch2/catch_amalgamated.hpp"

#include <mbed_hal.h>  // NOLINT
#include "mbed.h"      // NOLINT

using jled::MbedHal;

TEST_CASE("mbed_hal outputs 0 as 0 to the given pin using PwmOut (8-bit)",
          "[mbed_hal]") {
    mbedMockInit();
    constexpr auto kPin = 5;
    auto hal = MbedHal<>(kPin);

    hal.analogWrite<uint8_t>(0);

    REQUIRE(mbedMockGetPinState(kPin) == 0.);
}

TEST_CASE("mbed_hal outputs 255 as 1.0 to the given pin using PwmOut (8-bit)",
          "[mbed_hal]") {
    mbedMockInit();
    constexpr auto kPin = 5;
    auto hal = MbedHal<>(kPin);

    hal.analogWrite<uint8_t>(255);

    REQUIRE(mbedMockGetPinState(kPin) == 1.);
}

TEST_CASE("mbed_hal writes scaled value to the given pin using PwmOut (8-bit)",
          "[mbed_hal]") {
    mbedMockInit();
    constexpr auto kPin = 5;
    auto hal = MbedHal<>(kPin);

    hal.analogWrite<uint8_t>(127);

    REQUIRE_THAT(mbedMockGetPinState(kPin),
                 Catch::Matchers::WithinAbs(127 / 255., 0.0001));
}

TEST_CASE("mbed_hal outputs 16-bit values correctly (16-bit)",
          "[mbed_hal]") {
    mbedMockInit();
    constexpr auto kPin = 5;
    auto hal = MbedHal<16>(kPin);

    hal.analogWrite<uint16_t>(0);
    REQUIRE(mbedMockGetPinState(kPin) == 0.);

    hal.analogWrite<uint16_t>(65535);
    REQUIRE(mbedMockGetPinState(kPin) == 1.);

    hal.analogWrite<uint16_t>(32768);
    REQUIRE_THAT(mbedMockGetPinState(kPin),
                 Catch::Matchers::WithinAbs(32768 / 65535., 0.0001));
}

TEST_CASE("mbed_hal copy constructor writes to correct pin", "[mbed_hal]") {
    mbedMockInit();
    constexpr auto kPin = 5;
    auto hal = MbedHal<>(kPin);
    auto copy = hal;
    copy.analogWrite<uint8_t>(255);
    REQUIRE(mbedMockGetPinState(kPin) == 1.);
}

TEST_CASE("mbed_hal copy assignment writes to correct pin", "[mbed_hal]") {
    mbedMockInit();
    constexpr auto kPin = 5;
    MbedHal<> hal(kPin);
    MbedHal<> other(0);
    other = hal;
    other.analogWrite<uint8_t>(255);
    REQUIRE(mbedMockGetPinState(kPin) == 1.);
}

TEST_CASE("mbed_hal destructs without crash when uninitialised", "[mbed_hal]") {
    mbedMockInit();
    { MbedHal<> hal(5); }  // pwmout_ is null — must not crash
}
