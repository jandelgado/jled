// JLed Unit tests  (run on host).
// Copyright 2017 Jan Delgado jdelgado@gmx.net
#include <arduino_hal.h>  // NOLINT
#include "catch.hpp"

using jled::ArduinoHal;

TEST_CASE("first call to analogWrite() sets pin mode to OUTPUT",
          "[araduino_hal]") {
    arduinoMockInit();
    constexpr auto kPin = 10;
    auto h = ArduinoHal(kPin);
    REQUIRE(arduinoMockGetPinMode(kPin) == 0);
    h.analogWrite(123);
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
