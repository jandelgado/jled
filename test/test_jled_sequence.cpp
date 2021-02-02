// JLed Unit tests  (run on host).
// Copyright 2017-2021 Jan Delgado jdelgado@gmx.net
#include "catch.hpp"

#include <jled_base.h>  // NOLINT
#include "hal_mock.h"   // NOLINT

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
    constexpr uint8_t expected1[] = {255, 0, 0};
    constexpr uint8_t expected2[] = {0, 255, 255};
    REQUIRE(sizeof(expected1) == sizeof(expected2));  // enter criteria for test

    TestJLed leds[] = {TestJLed(HalMock(1)).Blink(1, 1).Repeat(1),
                       TestJLed(HalMock(2)).Blink(1, 1).Repeat(1).LowActive()};
    TestJLedSequence seq(TestJLedSequence::eMode::PARALLEL, leds);
    uint32_t time = 0;
    for (auto i = 0u; i < 3; i++) {
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
    constexpr uint8_t expected1[] = {255,  // first led turns on ...
                                     0,    // and off
                                     0, 0, 0};
    constexpr uint8_t expected2[] = {0, 0,
                                     255,             // second led turns on ...
                                     0, 0};           // and off
    REQUIRE(sizeof(expected1) == sizeof(expected2));  // enter criteria for test

    TestJLed leds[] = {TestJLed(HalMock(1)).Blink(1, 1).Repeat(1),
                       TestJLed(HalMock(2)).Blink(1, 1).Repeat(1)};
    TestJLedSequence seq(TestJLedSequence::eMode::SEQUENCE, leds);

    uint32_t time = 0;
    auto constexpr kSteps = sizeof(expected1) / sizeof(expected1[0]);
    for (auto i = 0u; i < kSteps; i++) {
        auto res = seq.Update();
        INFO("time = " << time << ", i = " << i << ",res=" << res);
        // Update() returns false on last Update. This example does only
        // 4 updates in total, returning false starting from update 3
        REQUIRE(res == (i < 3));
        REQUIRE(expected1[i] == leds[0].Hal().Value());
        REQUIRE(expected2[i] == leds[1].Hal().Value());
        time++;
        leds[0].Hal().SetMillis(time);
        leds[1].Hal().SetMillis(time);
    }
}

TEST_CASE("stop on sequence stops all JLeds and turns them off",
          "[jled_sequence]") {
    auto mode = GENERATE(TestJLedSequence::eMode::SEQUENCE,
                         TestJLedSequence::eMode::PARALLEL);
    SECTION("all leds are stopped and turned off") {
        INFO("mode = " << mode);
        TestJLed leds[] = {TestJLed(HalMock(1)).Blink(100, 100)};
        TestJLedSequence seq(mode, leds);

        seq.Update();
        REQUIRE(255 == leds[0].Hal().Value());
        seq.Stop();
        REQUIRE(0 == leds[0].Hal().Value());
        REQUIRE(!leds[0].IsRunning());
    }
}

TEST_CASE("repeat plays the sequence N times", "[jled_sequence]") {
    constexpr uint8_t expected[]{255, 0, 255, 0, 0};

    // TODO(JD): generate also over N
    auto mode = GENERATE(TestJLedSequence::eMode::SEQUENCE,
                         TestJLedSequence::eMode::PARALLEL);
    SECTION("repeat plays the sequence N times") {
        // 1 ms on, 1 ms off = 2ms off in total per iteration
        TestJLed leds[] = {TestJLed(HalMock(1)).Blink(1, 1)};
        auto seq = TestJLedSequence(mode, leds).Repeat(2);
        uint32_t time = 0;
        for (const auto val : expected) {
            INFO("mode = " << mode << ", time = " << time);
            seq.Update();
            REQUIRE(val == leds[0].Hal().Value());
            leds[0].Hal().SetMillis(++time);
        }
        REQUIRE(!seq.Update());
    }
}

TEST_CASE("Forever seems to play the sequence forever", "[jled_sequence]") {
    constexpr uint8_t expected[]{255, 0, 0};
    constexpr auto num = sizeof(expected) / sizeof(expected[0]);

    auto mode = GENERATE(TestJLedSequence::eMode::SEQUENCE,
                         TestJLedSequence::eMode::PARALLEL);
    SECTION("forever plays sequence forever") {
        INFO("mode = " << mode);
        // 1 ms on, 2 ms off in total per iteration
        TestJLed leds[] = {TestJLed(HalMock(1)).Blink(1, 2)};
        auto seq = TestJLedSequence(mode, leds).Forever();

        for (uint32_t time = 0; time < 1000; time++) {
            INFO("mode = " << mode << ", time = " << time << ",num=" << num
                           << ", mo=" << (time % num));
            leds[0].Hal().SetMillis(time);
            REQUIRE(seq.Update());
            CHECK(expected[time % num] == leds[0].Hal().Value());
        }
    }
}

TEST_CASE("Forever flag is initially set to false", "[jled_sequence]") {
    auto mode = GENERATE(TestJLedSequence::eMode::SEQUENCE,
                         TestJLedSequence::eMode::PARALLEL);
    SECTION("forever flag is initially off") {
        INFO("mode = " << mode);
        TestJLed leds[] = {TestJLed(HalMock(1)).Blink(1, 1)};
        auto seq = TestJLedSequence(mode, leds);
        REQUIRE_FALSE(seq.IsForever());
    }
}

TEST_CASE("Forever flag is set by call to Forever()", "[jled_sequence]") {
    auto mode = GENERATE(TestJLedSequence::eMode::SEQUENCE,
                         TestJLedSequence::eMode::PARALLEL);
    SECTION("forever flag is set") {
        INFO("mode = " << mode);
        TestJLed leds[] = {TestJLed(HalMock(1)).Blink(1, 1)};
        auto seq = TestJLedSequence(mode, leds).Forever();
        REQUIRE(seq.IsForever());
    }
}

TEST_CASE("reset on sequence resets all JLeds", "[jled_sequence]") {
    constexpr uint8_t expected[]{/* 1ms on */ 255,
                                 /* 1ms off */ 0, 255, 0,
                                 /* finally off */ 0};

    auto mode = GENERATE(TestJLedSequence::eMode::SEQUENCE,
                         TestJLedSequence::eMode::PARALLEL);
    SECTION("reset all leds in sequence") {
        INFO("mode = " << mode);
        TestJLed leds[] = {TestJLed(HalMock(1)).Blink(1, 1)};
        TestJLedSequence seq(mode, leds);

        uint32_t time = 0;
        for (const auto val : expected) {
            seq.Update();

            REQUIRE(val == leds[0].Hal().Value());

            ++time;
            leds[0].Hal().SetMillis(time);

            if (time == 2) {
                // expect sequence to stop after 2ms
                REQUIRE(!seq.Update());
                seq.Reset();
            }
        }
    }
}
