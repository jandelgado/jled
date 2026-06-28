// JLed Unit tests for the mbed_hal class (run on host).
// Copyright 2020 Jan Delgado jdelgado@gmx.net
#include "catch2/catch_amalgamated.hpp"

#include <mbed_hal.h>  // NOLINT
#include "mbed.h"      // NOLINT

using jled::MbedHal;

struct MbedMockFixture {
    MbedState mock{};
    MbedMockFixture()  { mbedMockSetInstance(&mock); }
    ~MbedMockFixture() { mbedMockSetInstance(nullptr); }
};

TEST_CASE_METHOD(MbedMockFixture, "MbedHal<> analogWrite 8-bit", "[mbed_hal]") {
    constexpr auto kPin = 5;
    auto hal = MbedHal<>(kPin);

    SECTION("0 outputs 0") {
        hal.analogWrite<uint8_t>(0);
        REQUIRE(mock.getPinState(kPin) == 0.);
    }

    SECTION("255 outputs 1.0") {
        hal.analogWrite<uint8_t>(255);
        REQUIRE(mock.getPinState(kPin) == 1.);
    }

    SECTION("127 outputs scaled value") {
        hal.analogWrite<uint8_t>(127);
        REQUIRE_THAT(mock.getPinState(kPin),
                     Catch::Matchers::WithinAbs(127 / 255., 0.0001));
    }
}

TEST_CASE_METHOD(MbedMockFixture, "MbedHal<16> analogWrite 16-bit", "[mbed_hal]") {
    constexpr auto kPin = 5;
    auto hal = MbedHal<16>(kPin);

    SECTION("0 outputs 0") {
        hal.analogWrite<uint16_t>(0);
        REQUIRE(mock.getPinState(kPin) == 0.);
    }

    SECTION("max outputs 1.0") {
        hal.analogWrite<uint16_t>(65535);
        REQUIRE(mock.getPinState(kPin) == 1.);
    }

    SECTION("mid outputs scaled value") {
        hal.analogWrite<uint16_t>(32768);
        REQUIRE_THAT(mock.getPinState(kPin),
                     Catch::Matchers::WithinAbs(32768 / 65535., 0.0001));
    }
}

TEST_CASE_METHOD(MbedMockFixture, "MbedHal<> copy and assignment", "[mbed_hal]") {
    constexpr auto kPin1 = 5;
    constexpr auto kPin2 = 6;

    SECTION("assignment uninitializes old PwmOut") {
        auto hal1 = MbedHal<>(kPin1);
        auto hal2 = MbedHal<>(kPin2);
        hal1.analogWrite<uint16_t>(65535);  // force lazy initialization of internal PwmOut

        REQUIRE(mock.getDtorCalled(kPin1) == 0);
        REQUIRE(mock.getDtorCalled(kPin2) == 0);
        hal1 = hal2;
        REQUIRE(mock.getDtorCalled(kPin1) == 1);
        REQUIRE(mock.getDtorCalled(kPin2) == 0);
    }

    SECTION("copy ctor writes to correct pin") {
        auto hal = MbedHal<>(kPin1);
        auto copy = hal;
        copy.analogWrite<uint8_t>(255);
        REQUIRE(mock.getPinState(kPin1) == 1.);
    }

    SECTION("copy assignment writes to correct pin") {
        MbedHal<> hal(kPin1);
        MbedHal<> other(0);
        other = hal;
        other.analogWrite<uint8_t>(255);
        REQUIRE(mock.getPinState(kPin1) == 1.);
    }

    SECTION("destructs without crash when uninitialized") {
        { MbedHal<> hal(5); }  // pwmout_ is null, must not crash
    }
}
