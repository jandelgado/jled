// JLed Unit tests  (run on host).
// Copyright 2017 Jan Delgado jdelgado@gmx.net
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <jled.h>  // NOLINT

TEST_CASE("empty sequence returns false on Update()", "[jled_sequence]") {}

TEST_CASE("simple sequence performs all updates", "[jled_sequence]") {
    arduinoMockInit();

    constexpr auto kPin = 1;
    JLed leds[] = {JLed(1).Blink(1, 1).Repeat(1),
                   JLed().Blink(1, 1).Invert().Repeat(1)};
    JLedSequence seq(leds, 2);
    constexpr uint8_t expected[] = {255,  // first led
                                    0,
                                    0,   // second led
                                    255, 
                                    255};  // final state

    uint32_t time = 0;
    for (const auto val : expected) {
        seq.Update();
        REQUIRE(arduinoMockGetPinState(kPin) == val);
        arduinoMockSetMillis(++time);
    }
}
