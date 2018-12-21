// JLed Unit tests  (run on host).
// Copyright 2017 Jan Delgado jdelgado@gmx.net
#include "catch.hpp"

#include <jled_base.h>  // NOLINT
#include "hal_mock.h"  // NOLINT

using jled::TJLed;
using jled::TJLedSequence;

// TestJLed is a JLed class using the HalMock for tests. This allows to
// test the code abstracted from the actual hardware in use.
class TestJLed : public TJLed<HalMock, TestJLed> {
    using TJLed<HalMock, TestJLed>::TJLed;
};

// a group of JLed objects which can be controlled simultanously
class TestJLedSequence : public TJLedSequence<TestJLed> {
    using TJLedSequence<TestJLed>::TJLedSequence;
};

// instanciate for test coverage measurement
template class TJLed<HalMock, TestJLed>;
template class TJLedSequence<TestJLed>;

TEST_CASE("parallel sequence performs all updates", "[jled_sequence]") {
    TestJLed leds[] = {TestJLed(HalMock(1)).Blink(1, 1).Repeat(1),
                       TestJLed(HalMock(2)).Blink(1, 1).Repeat(1).LowActive()};
    TestJLedSequence seq(TestJLedSequence::eMode::PARALLEL, leds, 2);
    constexpr uint8_t expected1[] = {255, 0, 0};
    constexpr uint8_t expected2[] = {0, 255, 255};
    REQUIRE(sizeof(expected1) ==
            sizeof(expected2));  // otherwise test may be invalid

    uint32_t time = 0;
    for (size_t i = 0; i < 3; i++) {
        auto res = seq.Update();
        // Update() returns false on last Update, our example does 2 updates.
        REQUIRE(res == (i < 1));
        REQUIRE(expected1[i] == leds[0].Hal().Value());
        REQUIRE(expected2[i] == leds[1].Hal().Value());
        time++;
        leds[0].Hal().SetMillis(time);
        leds[1].Hal().SetMillis(time);
    }
}

TEST_CASE("sequence performs all updates", "[jled_sequence]") {
    TestJLed leds[] = {TestJLed(HalMock(1)).Blink(1, 1).Repeat(1),
                       TestJLed(HalMock(2)).Blink(1, 1).Repeat(1)};
    TestJLedSequence seq(TestJLedSequence::eMode::SEQUENCE, leds, 2);
    constexpr uint8_t expected1[] = {255,  // first led turns on ...
                                     0,    // and off
                                     0, 0, 0};
    constexpr uint8_t expected2[] = {0, 0,
                                     255,    // second led turns on ...
                                     0, 0};  // and off
    REQUIRE(sizeof(expected1) ==
            sizeof(expected2));  // otherwise test may be invalid

    uint32_t time = 0;
    auto constexpr kSteps = sizeof(expected1);
    for (size_t i = 0; i < kSteps; i++) {
        auto res = seq.Update();
        // Update() returns false on last Update, our example does 4 updates.
        REQUIRE(res == (i < kSteps - 1));
        REQUIRE(expected1[i] == leds[0].Hal().Value());
        REQUIRE(expected2[i] == leds[1].Hal().Value());
        time++;
        leds[0].Hal().SetMillis(time);
        leds[1].Hal().SetMillis(time);
    }
}
