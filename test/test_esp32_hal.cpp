// JLed Unit tests  (run on host).
// Copyright 2017 Jan Delgado jdelgado@gmx.net
#include "catch.hpp"
#include <esp32_hal.h>  // NOLINT

using jled::Esp32Hal;

TEST_CASE("ledc ctor correctly initializes hardware", "[esp32_hal]") {
    arduinoMockInit();

    // attach channel 2 to pin 1
    constexpr auto kChan = 5;
    constexpr auto kPin = 10;
    auto aw[[gnu::unused]] = Esp32Hal(kPin, kChan);

    // writer expected to used 5000Hz and 8 bits
    REQUIRE(arduinoMockGetLedcSetup(kChan).freq == 5000);
    REQUIRE(arduinoMockGetLedcSetup(kChan).bit_num == 8);
    // check that channel is connected to pin
    REQUIRE(arduinoMockGetLedcAttachPin(kPin) == kChan);
}

TEST_CASE("ledc correctly autoselects channels", "[esp32_hal]") {
    constexpr auto kPin = 10;

    // note: we test a static property here (auto incremented next channel
    // number). so test has side effects. TODO avoid?
    for (auto i = 0; i < 20; i++) {
        auto h = Esp32Hal(kPin);
        // there are 16 channels and the auto selected channel is expected
        // to wrap over
        REQUIRE(h.chan() == i % 16);
    }
}

TEST_CASE("ledc analogWrite() writes correct value", "[esp32_hal]") {
    arduinoMockInit();

    // attach channel 2 to pin 1
    constexpr auto kChan = 5;
    constexpr auto kPin = 10;
    auto aw = Esp32Hal(kPin, kChan);

    aw.analogWrite(123);

    // note: value is written to channel, not pin.
    REQUIRE(arduinoMockGetLedcState(kChan) == 123);
}

TEST_CASE("millis() returns correct time", "[esp32_hal]") {
    arduinoMockInit();
    auto h = Esp32Hal(1);
    REQUIRE(h.millis() == 0);
    arduinoMockSetMillis(99);
    REQUIRE(h.millis() == 99);
}
