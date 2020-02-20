// JLed Unit tests  (run on host).
// Copyright 2017 Jan Delgado jdelgado@gmx.net

#include "catch.hpp"
#include <Arduino.h>

TEST_CASE("arduino mock correctly initialized", "[mock]") {
    arduinoMockInit();
    for (auto i = 0; i < ARDUINO_PINS; i++) {
        REQUIRE(arduinoMockGetPinMode(i) == 0);
        REQUIRE(arduinoMockGetPinState(i) == 0);
    }
    REQUIRE(millis() == 0);
}

TEST_CASE("arduino mock set time", "[mock]") {
    arduinoMockInit();
    REQUIRE(millis() == 0);
    arduinoMockSetMillis(6502);
    REQUIRE(millis() == 6502);
}

TEST_CASE("arduino mock analog write", "[mock]") {
    constexpr auto kTestPin = 10;
    arduinoMockInit();
    analogWrite(kTestPin, 99);
    REQUIRE(arduinoMockGetPinState(kTestPin) == 99);
}
