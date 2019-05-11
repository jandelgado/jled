// JLed Unit tests  (run on host).
// Copyright 2017 Jan Delgado jdelgado@gmx.net
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

TEST_CASE("ledc correctly selects channel for same pin", "[esp32_hal]") {
    constexpr auto kPin = 10;

    // note: we test a static property here (auto incremented next channel
    // number). so test has side effects. TODO avoid?
    auto h1 = Esp32Hal(kPin);
    auto h2 = Esp32Hal(kPin);

    // same channel is to be selected, since pin did not change
    REQUIRE(h1.chan() == h2.chan());
}

TEST_CASE("ledc correctly autoselects channels", "[esp32_hal]") {
    constexpr auto kPin = 10;

    auto h1 = Esp32Hal(kPin);
    auto h2 = Esp32Hal(kPin + 1);

    // same channel is to be selected, since pin did not change
    REQUIRE(h1.chan() != h2.chan());
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
