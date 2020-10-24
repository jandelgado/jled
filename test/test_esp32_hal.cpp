// JLed Unit tests for the ESP32 HAL (run on host).
// Copyright 2017-2020 Jan Delgado jdelgado@gmx.net
#include "esp32.h"  // NOLINT
#include <esp32_hal.h>  // NOLINT
#include "catch.hpp"

using jled::Esp32ChanMapper;
using jled::Esp32Hal;

TEST_CASE("channel mapper returns new channels for different pins",
          "[esp32_hal]") {
    auto m = Esp32ChanMapper();

    // expect a new channel starting with 0 for different pins and the same
    // channel if a pin is used again
    REQUIRE(m.chanForPin(10) == 0);
    REQUIRE(m.chanForPin(15) == 1);
    REQUIRE(m.chanForPin(3) == 2);
    REQUIRE(m.chanForPin(1) == 3);

    // no change when same pins are requested
    REQUIRE(m.chanForPin(10) == 0);
    REQUIRE(m.chanForPin(15) == 1);
    REQUIRE(m.chanForPin(3) == 2);
    REQUIRE(m.chanForPin(1) == 3);

    REQUIRE(m.chanForPin(7) == 4);
}

TEST_CASE("channel mapper starts over when channels are exhausted",
          "[esp32_hal]") {
    auto m = Esp32ChanMapper();

    for (auto i = 0; i < Esp32ChanMapper::kLedcMaxChan; i++) {
        REQUIRE(m.chanForPin(i) == i);
    }

    // now all channels are used and the mapper starts over at 0
    REQUIRE(m.chanForPin(100) == 0);
    REQUIRE(m.chanForPin(101) == 1);
}

TEST_CASE("ledc ctor correctly initializes hardware", "[esp32_hal]") {
    esp32MockInit();

    // attach channel 2 to pin 1
    constexpr auto kChan = 5;
    constexpr auto kPin = 10;
    auto hal[[gnu::unused]] = Esp32Hal(kPin, kChan);

    // writer expected to used 5000Hz and 8 bits
    REQUIRE(arduinoMockGetLedcSetup(kChan).freq == 5000);
    REQUIRE(arduinoMockGetLedcSetup(kChan).bit_num == 8);
    // check that channel is connected to pin
    REQUIRE(arduinoMockGetLedcAttachPin(kPin) == kChan);
}

TEST_CASE("ledc selects same channel for same pin", "[esp32_hal]") {
    constexpr auto kPin = 10;

    // note: we test a static property here (auto incremented next channel
    // number). so test has side effects. TODO avoid?
    auto hal1 = Esp32Hal(kPin);
    auto hal2 = Esp32Hal(kPin);

    // same channel is to be selected, since pin did not change
    REQUIRE(hal1.chan() == hal2.chan());
}

TEST_CASE("ledc selects different channels for different pins", "[esp32_hal]") {
    constexpr auto kPin = 10;

    auto hal1 = Esp32Hal(kPin);
    auto hal2 = Esp32Hal(kPin + 1);

    REQUIRE(hal1.chan() != hal2.chan());
}

TEST_CASE("analogWrite() writes value using analogWrite", "[esp32_hal]") {
    esp32MockInit();

    // attach channel 2 to pin 1
    constexpr auto kChan = 5;
    constexpr auto kPin = 10;
    auto hal = Esp32Hal(kPin, kChan);

    hal.analogWrite(123);

    // note: value is written to channel, not pin.
    REQUIRE(arduinoMockGetLedcState(kChan) == 123);
}

TEST_CASE("analogWrite() writes 0 as 0", "[esp32_hal]") {
    esp32MockInit();

    // attach channel 2 to pin 1
    constexpr auto kChan = 5;
    constexpr auto kPin = 10;
    auto hal = Esp32Hal(kPin, kChan);

    hal.analogWrite(0);
    REQUIRE(arduinoMockGetLedcState(kChan) == 0);
}

TEST_CASE("analogWrite() writes 255 as 256", "[esp32_hal]") {
    esp32MockInit();

    constexpr auto kChan = 5;
    constexpr auto kPin = 10;
    auto hal = Esp32Hal(kPin, kChan);

    hal.analogWrite(255);
    REQUIRE(arduinoMockGetLedcState(kChan) == 256);
}

TEST_CASE("millis() returns correct time", "[esp32_hal]") {
    arduinoMockInit();
    auto hal = Esp32Hal(1);
    REQUIRE(hal.millis() == 0);
    arduinoMockSetMillis(99);
    REQUIRE(hal.millis() == 99);
}
