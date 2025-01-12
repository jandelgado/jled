// JLed Unit tests  (run on host).
// Copyright 2017 Jan Delgado jdelgado@gmx.net
#include <arduino_hal.h>  // NOLINT
#include "catch2/catch_amalgamated.hpp"

using jled::ArduinoHal;

TEST_CASE("first call to analogWrite() sets pin mode to OUTPUT",
          "[arduino_hal]") {
    arduinoMockInit();
    constexpr auto kPin = 10;
    auto h = ArduinoHal(kPin);
    REQUIRE(arduinoMockGetPinMode(kPin) == 0);

    h.analogWrite(123);
    CHECK(arduinoMockGetPinMode(kPin) == OUTPUT);
}

TEST_CASE("analogWrite() scales value to 8 bits in default setup", "[arduino_hal]") {
    arduinoMockInit();
    constexpr auto kPin = 10;
    auto h = ArduinoHal(kPin);

    h.analogWrite(0);
    CHECK(arduinoMockGetPinState(kPin) == 0);

    h.analogWrite(1024);
    CHECK(arduinoMockGetPinState(kPin) == 4);

    h.analogWrite(65535);
    CHECK(arduinoMockGetPinState(kPin) == 255);
}

TEST_CASE("analogWrite() scales value to configured resolution", "[arduino_hal]") {
    arduinoMockInit();
    constexpr auto kPin = 10;
    auto h = ArduinoHal(kPin);

    ArduinoHal::WriteResolution(12);

    h.analogWrite(0);
    CHECK(arduinoMockGetPinState(kPin) == 0);

    h.analogWrite(65535);
    CHECK(arduinoMockGetPinState(kPin) == 4095);
}

TEST_CASE("millis() returns time from arduino framework", "[arduino_hal]") {
    arduinoMockInit();
    REQUIRE(jled::ArduinoClock::millis() == 0);
    arduinoMockSetMillis(99);
    CHECK(jled::ArduinoClock::millis() == 99);
}
