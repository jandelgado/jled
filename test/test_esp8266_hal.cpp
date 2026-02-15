// JLed Unit tests  (run on host).
// Copyright 2017 Jan Delgado jdelgado@gmx.net
#include "catch2/catch_amalgamated.hpp"
#include <esp8266_hal.h>  // NOLINT

using jled::Esp8266Hal;

TEST_CASE("analogWrite() writes correct 8-bit value scaled to 10-bit",
          "[esp8266_analog_writer]") {
    arduinoMockInit();

    constexpr auto kPin = 10;
    auto aw = Esp8266Hal(kPin);

    // Test 8-bit input values are scaled to 10-bit correctly
    aw.analogWrite<uint8_t>(0);
    REQUIRE(arduinoMockGetPinState(kPin) == 0);

    aw.analogWrite<uint8_t>(127);
    REQUIRE(arduinoMockGetPinState(kPin) == (127 << 2) + 3);

    aw.analogWrite<uint8_t>(255);
    REQUIRE(arduinoMockGetPinState(kPin) == 1023);

    // Test that a specific 8-bit value scales correctly
    aw.analogWrite<uint8_t>(123);
    REQUIRE(arduinoMockGetPinState(kPin) == (123 << 2) + 3);
}

TEST_CASE("analogWrite() writes correct 16-bit value scaled to 10-bit",
          "[esp8266_analog_writer]") {
    arduinoMockInit();

    constexpr auto kPin = 10;
    auto aw = Esp8266Hal(kPin);

    // Test 16-bit input values are downscaled to 10-bit correctly
    aw.analogWrite<uint16_t>(0);
    REQUIRE(arduinoMockGetPinState(kPin) == 0);

    aw.analogWrite<uint16_t>(65535);  // max 16-bit value
    REQUIRE(arduinoMockGetPinState(kPin) == 1023);  // max 10-bit value

    aw.analogWrite<uint16_t>(32768);  // mid 16-bit value
    REQUIRE(arduinoMockGetPinState(kPin) == 512);  // mid 10-bit value
}

TEST_CASE("millis() returns correct time", "[esp8266_hal]") {
    arduinoMockInit();
    REQUIRE(jled::Esp8266Clock::millis() == 0);
    arduinoMockSetMillis(99);
    REQUIRE(jled::Esp8266Clock::millis() == 99);
}

