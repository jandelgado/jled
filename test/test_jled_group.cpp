// JLed Unit tests for JLedGroup (run on host).
// Copyright 2017-2026 Jan Delgado jdelgado@gmx.net
#include "catch2/catch_amalgamated.hpp"

#include <jled_base.h>  // NOLINT
#include "hal_mock.h"           // NOLINT
#include "mock_brightness_eval.h"  // NOLINT

using namespace jled;  // NOLINT

class TestJLed : public TJLed<HalMock, TimeMock, uint8_t, TestJLed> {
    using TJLed<HalMock, TimeMock, uint8_t, TestJLed>::TJLed;
};

class TestJLedHD : public TJLed<HalMock, TimeMock, uint16_t, TestJLedHD> {
    using TJLed<HalMock, TimeMock, uint16_t, TestJLedHD>::TJLed;
};

namespace {
constexpr size_t kTestLedBufSize =
    sizeof(TestJLed) > sizeof(TestJLedHD) ? sizeof(TestJLed) : sizeof(TestJLedHD);  // NOLINT
constexpr size_t kTestGroupBufSize = sizeof(TJLedGroup<TimeMock, char>);
constexpr size_t kTestAnyBufSize =
    kTestLedBufSize > kTestGroupBufSize ? kTestLedBufSize : kTestGroupBufSize;  // NOLINT
}  // namespace

using TestJLedAny      = TJLedAny<kTestAnyBufSize>;
using TestJLedGroupAny = TJLedGroup<TimeMock, TestJLedAny>;
using TestJLedRefGroup = TJLedGroup<TimeMock, TJLedRef>;

// instantiate for test coverage measurement
template class TJLedGroup<TimeMock, TestJLedAny>;
template class TJLedGroup<TimeMock, TJLedRef>;

// Parallel uses the pointer overload; Sequential uses the array overload.
// Both static factory overloads are exercised across the test suite.

TEST_CASE("parallel group updates all elements simultaneously", "[jled_group]") {
    HalMock::Init();
    auto eval1 = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    auto eval2 = MockBrightnessEvaluator(std::vector<uint8_t>{150, 50});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval1).Repeat(1),
                          TestJLed(HalMock(2)).UserFunc(&eval2).Repeat(1)};
    // Use pointer overload to cover Parallel(AnyType*, size_t)
    auto group = TestJLedGroupAny::Parallel(leds, 2);

    TimeMock::set_millis(0);
    REQUIRE(group.Update());
    REQUIRE(HalMock::PinValue(1) == 200);
    REQUIRE(HalMock::PinValue(2) == 150);

    TimeMock::set_millis(1);
    REQUIRE(!group.Update());
    REQUIRE(HalMock::PinValue(1) == 100);
    REQUIRE(HalMock::PinValue(2) == 50);
}

TEST_CASE("sequential group plays elements one at a time", "[jled_group]") {
    HalMock::Init();
    auto eval1 = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    auto eval2 = MockBrightnessEvaluator(std::vector<uint8_t>{50, 25});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval1).Repeat(1),
                          TestJLed(HalMock(2)).UserFunc(&eval2).Repeat(1)};
    // Use array overload to cover Sequential(AnyType (&)[N])
    auto group = TestJLedGroupAny::Sequential(leds);

    // t=0: only LED1 active
    TimeMock::set_millis(0);
    REQUIRE(group.Update());
    REQUIRE(HalMock::PinValue(1) == 200);
    REQUIRE(HalMock::PinValue(2) == 0);

    // t=1: LED1 finishes (writes Eval(1)=100); LED2 not yet started
    TimeMock::set_millis(1);
    REQUIRE(group.Update());
    REQUIRE(HalMock::PinValue(1) == 100);
    REQUIRE(HalMock::PinValue(2) == 0);

    // t=2: LED2 starts (time_start_=2, Eval(0)=50)
    TimeMock::set_millis(2);
    REQUIRE(group.Update());
    REQUIRE(HalMock::PinValue(2) == 50);

    // t=3: LED2 finishes (Eval(1)=25), group done
    TimeMock::set_millis(3);
    REQUIRE(!group.Update());
    REQUIRE(HalMock::PinValue(2) == 25);
}

TEST_CASE("Repeat(n) plays the group n times", "[jled_group]") {
    HalMock::Init();
    auto mode = GENERATE(TestJLedGroupAny::eMode::SEQUENCE,
                         TestJLedGroupAny::eMode::PARALLEL);

    auto eval = MockBrightnessEvaluator(std::vector<uint8_t>{255, 0});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval)};
    auto group = TestJLedGroupAny(mode, leds, 1).Repeat(2);

    constexpr uint8_t expected[] = {255, 0, 255, 0};
    for (auto i = 0u; i < sizeof(expected); i++) {
        TimeMock::set_millis(i);
        group.Update();
        INFO("mode=" << mode << ", i=" << i);
        REQUIRE(HalMock::PinValue(1) == expected[i]);
    }
    REQUIRE(!group.Update());
}

TEST_CASE("Forever plays group indefinitely", "[jled_group]") {
    HalMock::Init();
    auto mode = GENERATE(TestJLedGroupAny::eMode::SEQUENCE,
                         TestJLedGroupAny::eMode::PARALLEL);

    auto eval = MockBrightnessEvaluator(std::vector<uint8_t>{255, 0, 0});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval)};
    auto group = TestJLedGroupAny(mode, leds, 1).Forever();

    constexpr uint8_t expected[] = {255, 0, 0};
    constexpr auto num = sizeof(expected) / sizeof(expected[0]);

    for (uint32_t t = 0; t < 1000; t++) {
        TimeMock::set_millis(t);
        INFO("mode=" << mode << ", t=" << t);
        REQUIRE(group.Update());
        REQUIRE(HalMock::PinValue(1) == expected[t % num]);
    }
}

TEST_CASE("IsForever is false initially, true after Forever()", "[jled_group]") {
    auto eval = MockBrightnessEvaluator(std::vector<uint8_t>{255});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval)};
    auto mode = GENERATE(TestJLedGroupAny::eMode::SEQUENCE,
                         TestJLedGroupAny::eMode::PARALLEL);
    auto group = TestJLedGroupAny(mode, leds, 1);
    REQUIRE_FALSE(group.IsForever());
    REQUIRE(group.Forever().IsForever());

    // compile-time check: Repeat and Forever can be chained
    TestJLedGroupAny chained [[gnu::unused]] =
        TestJLedGroupAny(TestJLedGroupAny::eMode::PARALLEL, leds, 1).Repeat(1).Forever();
}

TEST_CASE("Reset restarts group from beginning", "[jled_group]") {
    HalMock::Init();
    auto mode = GENERATE(TestJLedGroupAny::eMode::SEQUENCE,
                         TestJLedGroupAny::eMode::PARALLEL);

    auto eval = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval).Repeat(1)};
    auto group = TestJLedGroupAny(mode, leds, 1);

    TimeMock::set_millis(0);
    REQUIRE(group.Update());
    REQUIRE(HalMock::PinValue(1) == 200);

    TimeMock::set_millis(1);
    REQUIRE(!group.Update());

    group.Reset();

    TimeMock::set_millis(2);
    REQUIRE(group.Update());
    REQUIRE(HalMock::PinValue(1) == 200);

    TimeMock::set_millis(3);
    REQUIRE(!group.Update());
}

TEST_CASE("Stop halts group execution and turns LEDs off", "[jled_group]") {
    HalMock::Init();
    auto mode = GENERATE(TestJLedGroupAny::eMode::SEQUENCE,
                         TestJLedGroupAny::eMode::PARALLEL);

    auto eval = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval).Repeat(1)};
    auto group = TestJLedGroupAny(mode, leds, 1);

    TimeMock::set_millis(0);
    REQUIRE(group.Update());
    REQUIRE(HalMock::PinValue(1) == 200);

    group.Stop();
    REQUIRE(HalMock::PinValue(1) == 0);

    // Further Update() calls return false and keep the LED off (issue #115)
    TimeMock::set_millis(1);
    REQUIRE(!group.Update());
    REQUIRE(!group.Update());
}

TEST_CASE("nested JLedGroup within JLedGroup", "[jled_group]") {
    HalMock::Init();
    auto outer_eval = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    auto inner_eval = MockBrightnessEvaluator(std::vector<uint8_t>{50, 25});
    TestJLedAny inner_leds[] = {TestJLed(HalMock(2)).UserFunc(&inner_eval).Repeat(1)};
    TestJLedAny outer_leds[] = {TestJLed(HalMock(1)).UserFunc(&outer_eval).Repeat(1),
                                 TestJLedGroupAny::Parallel(inner_leds)};
    auto group = TestJLedGroupAny::Parallel(outer_leds);

    TimeMock::set_millis(0);
    REQUIRE(group.Update());
    REQUIRE(HalMock::PinValue(1) == 200);
    REQUIRE(HalMock::PinValue(2) == 50);

    TimeMock::set_millis(1);
    REQUIRE(!group.Update());
    REQUIRE(HalMock::PinValue(1) == 100);
    REQUIRE(HalMock::PinValue(2) == 25);
}

TEST_CASE("Stop propagates to nested group and inner LEDs", "[jled_group]") {
    HalMock::Init();
    auto outer_eval = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    auto inner_eval = MockBrightnessEvaluator(std::vector<uint8_t>{50, 25});
    TestJLedAny inner_leds[] = {TestJLed(HalMock(2)).UserFunc(&inner_eval).Repeat(1)};
    TestJLedAny outer_leds[] = {TestJLed(HalMock(1)).UserFunc(&outer_eval).Repeat(1),
                                 TestJLedGroupAny::Parallel(inner_leds)};
    auto group = TestJLedGroupAny::Parallel(outer_leds);

    TimeMock::set_millis(0);
    REQUIRE(group.Update());

    group.Stop();
    REQUIRE(HalMock::PinValue(1) == 0);
    REQUIRE(HalMock::PinValue(2) == 0);

    TimeMock::set_millis(1);
    REQUIRE(!group.Update());
}

TEST_CASE("JLedAny stores TestJLedHD and exercises 16-bit scale path", "[jled_group]") {
    HalMock::Init();
    // MaxBrightness(0x8000u) means lerp<uint16_t>(val, 0, 0x8000) calls scale<uint16_t>
    auto eval = MockBrightnessEvaluatorT<uint16_t>(std::vector<uint16_t>{32768u, 16384u});
    TestJLedAny leds[] = {
        TestJLedHD(HalMock(1)).UserFunc(&eval).Repeat(1).MaxBrightness(0x8000u)};
    auto group = TestJLedGroupAny::Parallel(leds);

    TimeMock::set_millis(0);
    REQUIRE(group.Update());
    REQUIRE(HalMock::PinValue(1) > 0);

    TimeMock::set_millis(1);
    REQUIRE(!group.Update());
}

TEST_CASE("JLedAny copy constructor copies all state", "[jled_group]") {
    HalMock::Init();
    auto eval = MockBrightnessEvaluator(std::vector<uint8_t>{255, 0});

    SECTION("copy of JLed") {
        TestJLedAny src(TestJLed(HalMock(1)).UserFunc(&eval).Repeat(1));
        TestJLedAny dst(src);
        TimeMock::set_millis(0);
        dst.Update(0);
        REQUIRE(HalMock::PinValue(1) == 255);
    }

    SECTION("copy of JLedGroup") {
        TestJLedAny led_arr[] = {TestJLed(HalMock(1)).UserFunc(&eval).Repeat(1)};
        TestJLedAny src(TestJLedGroupAny::Parallel(led_arr));
        TestJLedAny dst(src);
        TimeMock::set_millis(0);
        dst.Update(0);
        REQUIRE(HalMock::PinValue(1) == 255);
    }
}

TEST_CASE("JLedRefGroup references externally managed LEDs", "[jled_group]") {
    HalMock::Init();
    auto eval1 = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    auto eval2 = MockBrightnessEvaluator(std::vector<uint8_t>{50, 25});
    TestJLed led1 = TestJLed(HalMock(1)).UserFunc(&eval1).Repeat(1);
    TestJLed led2 = TestJLed(HalMock(2)).UserFunc(&eval2).Repeat(1);

    SECTION("references to two LEDs") {
        TJLedRef refs[] = {&led1, &led2};
        auto group = TestJLedRefGroup::Parallel(refs);

        TimeMock::set_millis(0);
        REQUIRE(group.Update());
        REQUIRE(HalMock::PinValue(1) == 200);
        REQUIRE(HalMock::PinValue(2) == 50);

        TimeMock::set_millis(1);
        REQUIRE(!group.Update());
        REQUIRE(HalMock::PinValue(1) == 100);
        REQUIRE(HalMock::PinValue(2) == 25);
    }

    SECTION("TJLedRef wraps a JLedGroup") {
        auto eval3 = MockBrightnessEvaluator(std::vector<uint8_t>{100, 0});
        TestJLedAny inner_leds[] = {TestJLed(HalMock(3)).UserFunc(&eval3).Repeat(1)};
        TestJLedGroupAny inner_group = TestJLedGroupAny::Parallel(inner_leds);

        TJLedRef refs[] = {&led1, &inner_group};
        auto group = TestJLedRefGroup::Parallel(refs);

        TimeMock::set_millis(0);
        REQUIRE(group.Update());
        REQUIRE(HalMock::PinValue(1) == 200);
        REQUIRE(HalMock::PinValue(3) == 100);
    }

    SECTION("nested JLedRefGroup within JLedRefGroup") {
        TJLedRef inner_refs[] = {&led2};
        auto inner_group = TestJLedRefGroup::Parallel(inner_refs);

        TJLedRef refs[] = {&led1, &inner_group};
        auto group = TestJLedRefGroup::Parallel(refs);

        TimeMock::set_millis(0);
        REQUIRE(group.Update());
        REQUIRE(HalMock::PinValue(1) == 200);
        REQUIRE(HalMock::PinValue(2) == 50);

        TimeMock::set_millis(1);
        REQUIRE(!group.Update());
    }

    SECTION("Stop propagates through TJLedRef") {
        TJLedRef refs[] = {&led1};
        auto group = TestJLedRefGroup::Parallel(refs);

        TimeMock::set_millis(0);
        REQUIRE(group.Update());
        REQUIRE(HalMock::PinValue(1) == 200);

        group.Stop();
        REQUIRE(HalMock::PinValue(1) == 0);
        TimeMock::set_millis(1);
        REQUIRE(!group.Update());
    }
}
