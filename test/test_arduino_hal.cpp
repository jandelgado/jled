// JLed Unit tests  (run on host).
// Copyright 2017 Jan Delgado jdelgado@gmx.net
#include <arduino_hal.h>  // NOLINT
#include "catch2/catch_amalgamated.hpp"

using jled::ArduinoHal;

TEST_CASE("first call to analogWrite() sets pin mode to OUTPUT (8-bit)",
          "[araduino_hal]") {
    arduinoMockInit();
    constexpr auto kPin = 10;
    auto h = ArduinoHal<>(kPin);
    REQUIRE(arduinoMockGetPinMode(kPin) == 0);
    h.analogWrite<uint8_t>(123);
    REQUIRE(arduinoMockGetPinMode(kPin) == OUTPUT);
}

TEST_CASE("analogWrite() writes correct 8-bit value", "[araduino_hal]") {
    arduinoMockInit();
    constexpr auto kPin = 10;
    auto h = ArduinoHal<>(kPin);
    h.analogWrite<uint8_t>(123);
    REQUIRE(arduinoMockGetPinState(kPin) == 123);
}

TEST_CASE("analogWrite() writes correct 16-bit value (downscaled)", "[araduino_hal]") {
    arduinoMockInit();
    constexpr auto kPin = 10;
    auto h = ArduinoHal<>(kPin);

    h.analogWrite<uint16_t>(0);
    REQUIRE(arduinoMockGetPinState(kPin) == 0);

    h.analogWrite<uint16_t>(65535);  // max 16-bit
    REQUIRE(arduinoMockGetPinState(kPin) == 255);  // downscaled to 8-bit

    h.analogWrite<uint16_t>(32768);  // mid 16-bit
    REQUIRE(arduinoMockGetPinState(kPin) == 128);  // mid 8-bit
}

TEST_CASE("ArduinoHal<10>: 8-bit input upscaled to 10-bit", "[arduino_hal]") {
    arduinoMockInit();
    constexpr auto kPin = 10;
    auto h = ArduinoHal<10>(kPin);

    h.analogWrite<uint8_t>(0);
    REQUIRE(arduinoMockGetPinState(kPin) == 0);

    h.analogWrite<uint8_t>(255);
    REQUIRE(arduinoMockGetPinState(kPin) == 1023);  // (1<<10)-1

    h.analogWrite<uint8_t>(1);
    REQUIRE(arduinoMockGetPinState(kPin) == 4);  // 1 << (10-8)

    h.analogWrite<uint8_t>(128);
    REQUIRE(arduinoMockGetPinState(kPin) == 512);  // 128 << 2
}

TEST_CASE("ArduinoHal<10>: 16-bit input downscaled to 10-bit", "[arduino_hal]") {
    arduinoMockInit();
    constexpr auto kPin = 10;
    auto h = ArduinoHal<10>(kPin);

    h.analogWrite<uint16_t>(0);
    REQUIRE(arduinoMockGetPinState(kPin) == 0);

    h.analogWrite<uint16_t>(65535);
    REQUIRE(arduinoMockGetPinState(kPin) == 1023);  // 65535 >> 6

    h.analogWrite<uint16_t>(32768);
    REQUIRE(arduinoMockGetPinState(kPin) == 512);  // 32768 >> 6
}

TEST_CASE("ArduinoHal<12>: 8-bit input upscaled to 12-bit", "[arduino_hal]") {
    arduinoMockInit();
    constexpr auto kPin = 10;
    auto h = ArduinoHal<12>(kPin);

    h.analogWrite<uint8_t>(0);
    REQUIRE(arduinoMockGetPinState(kPin) == 0);

    h.analogWrite<uint8_t>(255);
    REQUIRE(arduinoMockGetPinState(kPin) == 4095);  // (1<<12)-1

    h.analogWrite<uint8_t>(1);
    REQUIRE(arduinoMockGetPinState(kPin) == 16);  // 1 << (12-8)
}

TEST_CASE("ArduinoHal<12>: 16-bit input downscaled to 12-bit", "[arduino_hal]") {
    arduinoMockInit();
    constexpr auto kPin = 10;
    auto h = ArduinoHal<12>(kPin);

    h.analogWrite<uint16_t>(0);
    REQUIRE(arduinoMockGetPinState(kPin) == 0);

    h.analogWrite<uint16_t>(65535);
    REQUIRE(arduinoMockGetPinState(kPin) == 4095);  // 65535 >> 4

    h.analogWrite<uint16_t>(32768);
    REQUIRE(arduinoMockGetPinState(kPin) == 2048);  // 32768 >> 4
}

TEST_CASE("millis() returns correct time", "[arduino_hal]") {
    arduinoMockInit();
    REQUIRE(jled::ArduinoClock::millis() == 0);
    arduinoMockSetMillis(99);
    REQUIRE(jled::ArduinoClock::millis() == 99);
}
