// JLed Unit tests  (run on host).
// Copyright 2017 Jan Delgado jdelgado@gmx.net
#include <arduino_hal.h>  // NOLINT
#include "catch2/catch_amalgamated.hpp"

using jled::ArduinoHal;

TEST_CASE("first call to analogWrite() sets pin mode to OUTPUT (8-bit)",
          "[araduino_hal]") {
    arduinoMockInit();
    constexpr auto kPin = 10;
    auto h = ArduinoHal(kPin);
    REQUIRE(arduinoMockGetPinMode(kPin) == 0);
    h.analogWrite<uint8_t>(123);
    REQUIRE(arduinoMockGetPinMode(kPin) == OUTPUT);
}

TEST_CASE("analogWrite() writes correct 8-bit value", "[araduino_hal]") {
    arduinoMockInit();
    constexpr auto kPin = 10;
    auto h = ArduinoHal(kPin);
    h.analogWrite<uint8_t>(123);
    REQUIRE(arduinoMockGetPinState(kPin) == 123);
}

TEST_CASE("analogWrite() writes correct 16-bit value (downscaled)", "[araduino_hal]") {
    arduinoMockInit();
    constexpr auto kPin = 10;
    auto h = ArduinoHal(kPin);

    h.analogWrite<uint16_t>(0);
    REQUIRE(arduinoMockGetPinState(kPin) == 0);

    h.analogWrite<uint16_t>(65535);  // max 16-bit
    REQUIRE(arduinoMockGetPinState(kPin) == 255);  // downscaled to 8-bit

    h.analogWrite<uint16_t>(32768);  // mid 16-bit
    REQUIRE(arduinoMockGetPinState(kPin) == 128);  // mid 8-bit
}

TEST_CASE("millis() returns correct time", "[arduino_hal]") {
    arduinoMockInit();
    REQUIRE(jled::ArduinoClock::millis() == 0);
    arduinoMockSetMillis(99);
    REQUIRE(jled::ArduinoClock::millis() == 99);
}
