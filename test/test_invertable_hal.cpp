// JLed unit tests for the InvertableHal<Hal> adapter (run on host).
// Copyright 2026 Jan Delgado jdelgado@gmx.net
#include <Arduino.h>
#include <arduino_hal.h>     // NOLINT
#include <invertable_hal.h>  // NOLINT

#include "catch2/catch_amalgamated.hpp"
#include "hal_mock.h"  // NOLINT

using jled::ArduinoHal;
using jled::InvertableHal;

TEST_CASE("InvertableHal<HalMock> analogWrite", "[invertable_hal]") {
    InvertableHal<HalMock> hal(1);

    SECTION("passthrough when invert is false (uint8_t)") {
        hal.analogWrite<uint8_t>(123, false);
        REQUIRE(hal.Value() == 123);
    }

    SECTION("passthrough when invert is false (uint16_t)") {
        hal.analogWrite<uint16_t>(12345, false);
        REQUIRE(hal.Value() == 12345);
    }

    SECTION("inverts uint8_t value when invert is true") {
        hal.analogWrite<uint8_t>(0, true);
        REQUIRE(hal.Value() == 255);

        hal.analogWrite<uint8_t>(255, true);
        REQUIRE(hal.Value() == 0);

        hal.analogWrite<uint8_t>(100, true);
        REQUIRE(hal.Value() == 155);
    }

    SECTION("inverts uint16_t value when invert is true") {
        hal.analogWrite<uint16_t>(0, true);
        REQUIRE(hal.Value() == 65535);

        hal.analogWrite<uint16_t>(65535, true);
        REQUIRE(hal.Value() == 0);
    }
}

struct ArduinoMockFixture {
    ArduinoState mock{};
    ArduinoMockFixture() { arduinoMockSetInstance(&mock); }
    ~ArduinoMockFixture() { arduinoMockSetInstance(nullptr); }
};

TEST_CASE_METHOD(ArduinoMockFixture, "InvertableHal<ArduinoHal<8>> analogWrite",
                 "[invertable_hal]") {
    constexpr auto kPin = 10;
    InvertableHal<ArduinoHal<8>> hal(kPin);

    SECTION("first call sets pin mode to OUTPUT, same as unwrapped ArduinoHal") {
        REQUIRE(mock.getPinMode(kPin) == 0);
        hal.analogWrite<uint8_t>(123, false);
        REQUIRE(mock.getPinMode(kPin) == OUTPUT);
    }

    SECTION("passes value through unchanged when invert is false") {
        hal.analogWrite<uint8_t>(123, false);
        REQUIRE(mock.getPinState(kPin) == 123);
    }

    SECTION("inverts value before forwarding when invert is true") {
        hal.analogWrite<uint8_t>(0, true);
        REQUIRE(mock.getPinState(kPin) == 255);

        hal.analogWrite<uint8_t>(255, true);
        REQUIRE(mock.getPinState(kPin) == 0);
    }
}
