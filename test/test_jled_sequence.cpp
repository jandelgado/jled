// JLed Unit tests  (run on host).
// Copyright 2017 Jan Delgado jdelgado@gmx.net
#include "catch.hpp"

#include <jled.h>  // NOLINT

// TODO(jd) implement test
TEST_CASE("empty sequence returns false on Update()", "[jled_sequence]") {}

TEST_CASE("sequential sequence performs all updates", "[jled_sequence]") {
    arduinoMockInit();

    JLed leds[] = {
        JLed(1).Blink(1, 1).Repeat(1),
        JLed(2).Blink(1, 1).Repeat(1).LowActive(),
    };
    JLedSequence seq(JLedSequence::eMode::PARALLEL, leds, 2);
    constexpr uint8_t expected1[] = {255, 0, 0};
    constexpr uint8_t expected2[] = {0, 255, 255};

    uint32_t time = 0;
    for (size_t i = 0; i < 3; i++) {
        seq.Update();
        REQUIRE(expected1[i] == arduinoMockGetPinState(1));
        REQUIRE(expected2[i] == arduinoMockGetPinState(2));
        arduinoMockSetMillis(++time);
    }
}

TEST_CASE("sequence performs all updates", "[jled_sequence]") {
    arduinoMockInit();

    JLed leds[] = {JLed(1).Blink(1, 1).Repeat(1),
                   JLed(1).Blink(1, 1).Repeat(1)};
    JLedSequence seq(JLedSequence::eMode::SEQUENCE, leds, 2);
    constexpr uint8_t expected[] = {255,  // first led
                                    0,
                                    255,    // second led
                                    0, 0};  // final state
    uint32_t time = 0;
    for (const auto val : expected) {
        seq.Update();
        REQUIRE(val == arduinoMockGetPinState(1));
        arduinoMockSetMillis(++time);
    }
}
