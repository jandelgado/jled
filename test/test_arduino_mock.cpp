// JLed Unit tests  (run on host).
// Copyright 2017 Jan Delgado jdelgado@gmx.net

#include "catch2/catch_amalgamated.hpp"
#include <Arduino.h>

TEST_CASE("arduino mock correctly initialized", "[mock]") {
    ArduinoState state{};
    arduinoMockSetInstance(&state);
    for (auto i = 0; i < ARDUINO_PINS; i++) {
        REQUIRE(state.getPinMode(i) == 0);
        REQUIRE(state.getPinState(i) == 0);
    }
    REQUIRE(millis() == 0);
    arduinoMockSetInstance(nullptr);
}

TEST_CASE("arduino mock set time", "[mock]") {
    ArduinoState state{};
    arduinoMockSetInstance(&state);
    REQUIRE(millis() == 0);
    state.setMillis(6502);
    REQUIRE(millis() == 6502);
    arduinoMockSetInstance(nullptr);
}

TEST_CASE("arduino mock analog write", "[mock]") {
    constexpr auto kTestPin = 10;
    ArduinoState state{};
    arduinoMockSetInstance(&state);
    analogWrite(kTestPin, 99);
    REQUIRE(state.getPinState(kTestPin) == 99);
    arduinoMockSetInstance(nullptr);
}
