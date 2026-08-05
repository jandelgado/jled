// JLed Unit tests  (run on host).
// Copyright 2017 Jan Delgado jdelgado@gmx.net
#include <arduino_hal.h>  // NOLINT

#include "catch2/catch_amalgamated.hpp"

using jled::ArduinoHal;

struct ArduinoMockFixture {
    ArduinoState mock{};
    ArduinoMockFixture() { arduinoMockSetInstance(&mock); }
    ~ArduinoMockFixture() { arduinoMockSetInstance(nullptr); }
};

TEST_CASE_METHOD(ArduinoMockFixture, "ArduinoHal<> analogWrite", "[arduino_hal]") {
    constexpr auto kPin = 10;
    auto h = ArduinoHal<>(kPin);

    SECTION("constructor sets pin mode to OUTPUT immediately") {
        REQUIRE(mock.getPinMode(kPin) == OUTPUT);
    }

    SECTION("correct 8-bit value written") {
        h.analogWrite<uint8_t>(123);
        REQUIRE(mock.getPinState(kPin) == 123);
    }

    SECTION("16-bit value downscaled to 8-bit") {
        h.analogWrite<uint16_t>(0);
        REQUIRE(mock.getPinState(kPin) == 0);

        h.analogWrite<uint16_t>(65535);
        REQUIRE(mock.getPinState(kPin) == 255);

        h.analogWrite<uint16_t>(32768);
        REQUIRE(mock.getPinState(kPin) == 128);
    }

    SECTION("analogWriteResolution() not called for 8-bit HAL") {
        h.analogWrite<uint8_t>(128);
        REQUIRE(mock.getAnalogWriteResolution() == 0);
    }
}

TEST_CASE_METHOD(ArduinoMockFixture, "ArduinoHal<10> analogWrite", "[arduino_hal]") {
    constexpr auto kPin = 10;
    auto h = ArduinoHal<10>(kPin);

    SECTION("8-bit input upscaled to 10-bit") {
        h.analogWrite<uint8_t>(0);
        REQUIRE(mock.getPinState(kPin) == 0);

        h.analogWrite<uint8_t>(255);
        REQUIRE(mock.getPinState(kPin) == 1023);

        h.analogWrite<uint8_t>(1);
        REQUIRE(mock.getPinState(kPin) == 4);

        h.analogWrite<uint8_t>(128);
        REQUIRE(mock.getPinState(kPin) == 514);
    }

    SECTION("16-bit input downscaled to 10-bit") {
        h.analogWrite<uint16_t>(0);
        REQUIRE(mock.getPinState(kPin) == 0);

        h.analogWrite<uint16_t>(65535);
        REQUIRE(mock.getPinState(kPin) == 1023);

        h.analogWrite<uint16_t>(32768);
        REQUIRE(mock.getPinState(kPin) == 512);
    }

    SECTION("analogWriteResolution() called once with kResBits, immediately upon construction") {
        REQUIRE(mock.getAnalogWriteResolution() == 10);

        mock.analog_write_resolution = 0;
        h.analogWrite<uint8_t>(64);
        REQUIRE(mock.getAnalogWriteResolution() == 0);
    }
}

TEST_CASE_METHOD(ArduinoMockFixture, "ArduinoHal<12> analogWrite", "[arduino_hal]") {
    constexpr auto kPin = 10;
    auto h = ArduinoHal<12>(kPin);

    SECTION("8-bit input upscaled to 12-bit") {
        h.analogWrite<uint8_t>(0);
        REQUIRE(mock.getPinState(kPin) == 0);

        h.analogWrite<uint8_t>(255);
        REQUIRE(mock.getPinState(kPin) == 4095);

        h.analogWrite<uint8_t>(1);
        REQUIRE(mock.getPinState(kPin) == 16);
    }

    SECTION("16-bit input downscaled to 12-bit") {
        h.analogWrite<uint16_t>(0);
        REQUIRE(mock.getPinState(kPin) == 0);

        h.analogWrite<uint16_t>(65535);
        REQUIRE(mock.getPinState(kPin) == 4095);

        h.analogWrite<uint16_t>(32768);
        REQUIRE(mock.getPinState(kPin) == 2048);
    }
}

TEST_CASE_METHOD(ArduinoMockFixture, "ArduinoHal<10, true> analogWrite (lazy init, e.g. STM32)",
                 "[arduino_hal]") {
    constexpr auto kPin = 10;
    auto h = ArduinoHal<10, true>(kPin);

    SECTION("constructor does not set pin mode or PWM resolution") {
        REQUIRE(mock.getPinMode(kPin) == 0);
        REQUIRE(mock.getAnalogWriteResolution() == 0);
    }

    SECTION("first call to analogWrite() sets pin mode to OUTPUT and PWM resolution") {
        h.analogWrite<uint8_t>(123);
        REQUIRE(mock.getPinMode(kPin) == OUTPUT);
        REQUIRE(mock.getAnalogWriteResolution() == 10);
    }

    SECTION("analogWriteResolution() called only once, not on subsequent writes") {
        h.analogWrite<uint8_t>(123);
        mock.analog_write_resolution = 0;
        h.analogWrite<uint8_t>(64);
        REQUIRE(mock.getAnalogWriteResolution() == 0);
    }

    SECTION("correct 10-bit value written") {
        h.analogWrite<uint8_t>(255);
        REQUIRE(mock.getPinState(kPin) == 1023);
    }
}

TEST_CASE_METHOD(ArduinoMockFixture, "ArduinoClock::millis()", "[arduino_hal]") {
    SECTION("returns 0 at start") {
        REQUIRE(jled::ArduinoClock::millis() == 0);
    }

    SECTION("returns set value") {
        mock.setMillis(99);
        REQUIRE(jled::ArduinoClock::millis() == 99);
    }
}
