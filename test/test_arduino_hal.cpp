// JLed Unit tests  (run on host).
// Copyright 2017 Jan Delgado jdelgado@gmx.net
#include "catch.hpp"
#include <arduino_hal.h>  // NOLINT

using jled::ArduinoHal;

TEST_CASE("ctor sets pin mode to OUTPUT", "[arduino_hal]") {
    arduinoMockInit();
    constexpr auto kPin = 10;
    REQUIRE(arduinoMockGetPinMode(kPin) == 0);
    auto h[[gnu::unused]] = ArduinoHal(kPin);
    REQUIRE(arduinoMockGetPinMode(kPin) == OUTPUT);
}

TEST_CASE("analogWrite() writes correct value", "[araduino_hal]") {
    arduinoMockInit();
    constexpr auto kPin = 10;
    auto h = ArduinoHal(kPin);
    h.analogWrite(123);
    REQUIRE(arduinoMockGetPinState(kPin) == 123);
}

TEST_CASE("millis() returns correct time", "[arduino_hal]") {
    arduinoMockInit();
    auto h = ArduinoHal(1);
    REQUIRE(h.millis() == 0);
    arduinoMockSetMillis(99);
    REQUIRE(h.millis() == 99);
}

