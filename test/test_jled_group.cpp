// JLed Unit tests for JLedGroup and JLedAny (run on host).
// Copyright 2017-2026 Jan Delgado jdelgado@gmx.net
#include "catch2/catch_amalgamated.hpp"

#include <jled_base.h>  // NOLINT
#include "hal_mock.h"   // NOLINT

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

// instanciate for test coverage measurement
template class TJLedGroup<TimeMock, TestJLedAny>;
template class TJLedGroup<TimeMock, TJLedRef>;

TEST_CASE("parallel group updates all elements simultaneously", "[jled_group]") {
    HalMock::Init();
    // pin 1: normal, pin 2: low-active, same Blink(1,1)
    TestJLedAny leds[] = {TestJLed(HalMock(1)).Blink(1, 1).Repeat(1),
                          TestJLed(HalMock(2)).Blink(1, 1).Repeat(1).LowActive()};
    auto group = TestJLedGroupAny::Parallel(leds);

    constexpr uint8_t expected1[] = {255, 0, 0};
    constexpr uint8_t expected2[] = {0, 255, 255};

    for (auto i = 0u; i < 3u; i++) {
        TimeMock::set_millis(i);
        auto res = group.Update();
        INFO("i=" << i);
        REQUIRE(res == (i < 1));
        REQUIRE(HalMock::PinValue(1) == expected1[i]);
        REQUIRE(HalMock::PinValue(2) == expected2[i]);
    }
}

TEST_CASE("sequential group plays elements one at a time", "[jled_group]") {
    HalMock::Init();
    TestJLedAny leds[] = {TestJLed(HalMock(1)).Blink(1, 1).Repeat(1),
                          TestJLed(HalMock(2)).Blink(1, 1).Repeat(1)};
    auto group = TestJLedGroupAny::Sequential(leds);

    // pin1: on at t=0, off at t=1, done; pin2 starts at t=2
    constexpr uint8_t expected1[] = {255, 0, 0, 0, 0};
    constexpr uint8_t expected2[] = {0,   0, 255, 0, 0};

    constexpr auto kSteps = sizeof(expected1) / sizeof(expected1[0]);
    for (auto i = 0u; i < kSteps; i++) {
        TimeMock::set_millis(i);
        auto res = group.Update();
        INFO("i=" << i);
        REQUIRE(res == (i < 3));
        REQUIRE(HalMock::PinValue(1) == expected1[i]);
        REQUIRE(HalMock::PinValue(2) == expected2[i]);
    }
}

TEST_CASE("Repeat(n) plays the group n times", "[jled_group]") {
    HalMock::Init();
    auto mode = GENERATE(TestJLedGroupAny::eMode::SEQUENCE,
                         TestJLedGroupAny::eMode::PARALLEL);

    TestJLedAny leds[] = {TestJLed(HalMock(1)).Blink(1, 1)};
    auto group = TestJLedGroupAny(mode, leds, 1).Repeat(2);

    constexpr uint8_t expected[] = {255, 0, 255, 0, 0};
    for (auto i = 0u; i < sizeof(expected); i++) {
        TimeMock::set_millis(i);
        group.Update();
        INFO("mode=" << mode << ", i=" << i);
        REQUIRE(HalMock::PinValue(1) == expected[i]);
    }
    REQUIRE(!group.Update());
}

TEST_CASE("Forever plays group forever", "[jled_group]") {
    HalMock::Init();
    auto mode = GENERATE(TestJLedGroupAny::eMode::SEQUENCE,
                         TestJLedGroupAny::eMode::PARALLEL);

    TestJLedAny leds[] = {TestJLed(HalMock(1)).Blink(1, 2)};
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

TEST_CASE("IsForever is false initially", "[jled_group]") {
    TestJLedAny leds[] = {TestJLed(HalMock(1)).Blink(1, 1)};
    auto mode = GENERATE(TestJLedGroupAny::eMode::SEQUENCE,
                         TestJLedGroupAny::eMode::PARALLEL);
    REQUIRE_FALSE(TestJLedGroupAny(mode, leds, 1).IsForever());
}

TEST_CASE("IsForever is true after calling Forever()", "[jled_group]") {
    TestJLedAny leds[] = {TestJLed(HalMock(1)).Blink(1, 1)};
    auto mode = GENERATE(TestJLedGroupAny::eMode::SEQUENCE,
                         TestJLedGroupAny::eMode::PARALLEL);
    REQUIRE(TestJLedGroupAny(mode, leds, 1).Forever().IsForever());
}

TEST_CASE("Repeat and Forever can be chained", "[jled_group]") {
    // compile-time check
    TestJLedAny leds[] = {TestJLed(HalMock(1)).Blink(1, 1)};
    TestJLedGroupAny group[[gnu::unused]] =
        TestJLedGroupAny(TestJLedGroupAny::eMode::PARALLEL, leds, 1).Repeat(1).Forever();
}

TEST_CASE("Reset restarts group from beginning", "[jled_group]") {
    HalMock::Init();
    auto mode = GENERATE(TestJLedGroupAny::eMode::SEQUENCE,
                         TestJLedGroupAny::eMode::PARALLEL);

    TestJLedAny leds[] = {TestJLed(HalMock(1)).Blink(1, 1)};
    auto group = TestJLedGroupAny(mode, leds, 1);

    constexpr uint8_t expected[] = {255, 0, 255, 0, 0};
    for (auto i = 0u; i < sizeof(expected); i++) {
        TimeMock::set_millis(i);
        group.Update();
        REQUIRE(HalMock::PinValue(1) == expected[i]);
        if (i == 1) {
            REQUIRE(!group.Update());
            group.Reset();
        }
    }
}

TEST_CASE("Stop halts group execution", "[jled_group]") {
    HalMock::Init();
    auto mode = GENERATE(TestJLedGroupAny::eMode::SEQUENCE,
                         TestJLedGroupAny::eMode::PARALLEL);

    TestJLedAny leds[] = {TestJLed(HalMock(1)).Blink(100, 100)};
    auto group = TestJLedGroupAny(mode, leds, 1);

    TimeMock::set_millis(0);
    REQUIRE(group.Update());
    REQUIRE(HalMock::PinValue(1) == 255);

    group.Stop();
    REQUIRE(HalMock::PinValue(1) == 0);
    TimeMock::set_millis(1);
    REQUIRE(!group.Update());
}

TEST_CASE("TestJLedHD stored in TestJLedAny", "[jled_group]") {
    HalMock::Init();
    TestJLedAny leds[] = {TestJLedHD(HalMock(1)).Blink(1, 1).Repeat(1)};
    auto group = TestJLedGroupAny::Parallel(leds);

    TimeMock::set_millis(0);
    REQUIRE(group.Update());
    REQUIRE(HalMock::PinValue(1) > 0);

    TimeMock::set_millis(1);
    REQUIRE(!group.Update());
    REQUIRE(HalMock::PinValue(1) == 0);
}

TEST_CASE("nested JLedGroup within JLedGroup", "[jled_group]") {
    HalMock::Init();

    TestJLedAny inner_leds[] = {TestJLed(HalMock(2)).Blink(1, 1).LowActive().Repeat(1)};
    TestJLedAny outer_leds[] = {TestJLed(HalMock(1)).Blink(1, 1).Repeat(1),
                                 TestJLedGroupAny::Parallel(inner_leds)};
    auto group = TestJLedGroupAny::Parallel(outer_leds);

    TimeMock::set_millis(0);
    REQUIRE(group.Update());
    REQUIRE(HalMock::PinValue(1) == 255);
    REQUIRE(HalMock::PinValue(2) == 0);

    TimeMock::set_millis(1);
    REQUIRE(!group.Update());
    REQUIRE(HalMock::PinValue(1) == 0);
    REQUIRE(HalMock::PinValue(2) == 255);
}

TEST_CASE("JLedGroup copy constructor copies all state", "[jled_group]") {
    HalMock::Init();
    TestJLedAny leds[] = {TestJLed(HalMock(1)).Blink(1, 1).Repeat(1)};
    TestJLedAny src(TestJLedGroupAny::Parallel(leds));
    TestJLedAny dst(src);  // copy

    TimeMock::set_millis(0);
    dst.Update(0);
    REQUIRE(HalMock::PinValue(1) == 255);
}

TEST_CASE("JLedAny copy of JLed copies state", "[jled_group]") {
    HalMock::Init();
    TestJLedAny src(TestJLed(HalMock(1)).Blink(1, 1).Repeat(1));
    TestJLedAny dst(src);  // copy exercises copy_fn_ for T=TestJLed

    TimeMock::set_millis(0);
    dst.Update(0);
    REQUIRE(HalMock::PinValue(1) == 255);
}

TEST_CASE("Parallel/Sequential pointer overloads work", "[jled_group]") {
    HalMock::Init();
    TestJLedAny leds[] = {TestJLed(HalMock(1)).Blink(1, 1).Repeat(1)};

    SECTION("Parallel pointer overload") {
        auto group = TestJLedGroupAny::Parallel(leds, 1);
        TimeMock::set_millis(0);
        REQUIRE(group.Update());
        REQUIRE(HalMock::PinValue(1) == 255);
    }
    SECTION("Sequential pointer overload") {
        auto group = TestJLedGroupAny::Sequential(leds, 1);
        TimeMock::set_millis(0);
        REQUIRE(group.Update());
        REQUIRE(HalMock::PinValue(1) == 255);
    }
}

TEST_CASE("JLedHD with MaxBrightness exercises scale<uint16_t>", "[jled_group]") {
    HalMock::Init();
    // Non-default MaxBrightness prevents the early return in lerp<uint16_t>,
    // so scale<uint16_t> (line 705 in jled_base.h) is executed.
    TestJLedAny leds[] = {
        TestJLedHD(HalMock(1)).Blink(2, 1).Repeat(1).MaxBrightness(0x8000u)};
    auto group = TestJLedGroupAny::Parallel(leds);
    TimeMock::set_millis(0);
    REQUIRE(group.Update());
    REQUIRE(HalMock::PinValue(1) > 0);
}

TEST_CASE("JLedHD Candle exercises 16-bit dim-brightness path", "[jled_group]") {
    HalMock::Init();
    // jitter=255 guarantees rnd < jitter, so the 16-bit candle dim-value
    // branch (lines 206-207 in jled_base.h) is taken.
    TestJLedAny leds[] = {TestJLedHD(HalMock(1)).Candle(0, 255, 100)};
    auto group = TestJLedGroupAny::Parallel(leds);
    TimeMock::set_millis(0);
    group.Update();  // t=0: returns cached initial value
    rand_seed(0);    // first rand8() returns 0x59=89 < 255 -> dim path
    TimeMock::set_millis(1);
    REQUIRE(group.Update());
    REQUIRE(HalMock::PinValue(1) > 0);
}

TEST_CASE("JLedRefGroup references externally managed LEDs", "[jled_group]") {
    HalMock::Init();
    TestJLed   led1 = TestJLed(HalMock(1)).Blink(1, 1).Repeat(1);
    TestJLedHD led2 = TestJLedHD(HalMock(2)).Blink(1, 1).Repeat(1);
    TJLedRef refs[] = {&led1, &led2};
    auto group = TestJLedRefGroup::Parallel(refs);

    TimeMock::set_millis(0);
    REQUIRE(group.Update());
    REQUIRE(led1.GetHal().Value() > 0);
    REQUIRE(led2.GetHal().Value() > 0);

    TimeMock::set_millis(1);
    REQUIRE(!group.Update());
}

TEST_CASE("JLedRef can wrap a JLedGroup", "[jled_group]") {
    HalMock::Init();
    TestJLed    led1 = TestJLed(HalMock(1)).Blink(1, 1).Repeat(1);
    TestJLedAny inner[] = {TestJLed(HalMock(2)).Blink(1, 1).Repeat(1)};
    TestJLedGroupAny inner_group = TestJLedGroupAny::Parallel(inner);

    TJLedRef refs[] = {&led1, &inner_group};
    auto group = TestJLedRefGroup::Parallel(refs);

    TimeMock::set_millis(0);
    REQUIRE(group.Update());
    REQUIRE(led1.GetHal().Value() > 0);
    REQUIRE(HalMock::PinValue(2) > 0);
}

TEST_CASE("nested JLedRefGroup within JLedRefGroup", "[jled_group]") {
    HalMock::Init();

    TestJLed   inner0 = TestJLed(HalMock(2)).Blink(1, 1).LowActive().Repeat(1);
    TJLedRef   inner_refs[] = {&inner0};
    auto innerGroup = TestJLedRefGroup::Parallel(inner_refs);

    TestJLed   led1 = TestJLed(HalMock(1)).Blink(1, 1).Repeat(1);
    TJLedRef   leds[] = {&led1, &innerGroup};
    auto group = TestJLedRefGroup::Parallel(leds);

    TimeMock::set_millis(0);
    REQUIRE(group.Update());
    REQUIRE(HalMock::PinValue(1) == 255);
    REQUIRE(HalMock::PinValue(2) == 0);  // low-active: on=0

    TimeMock::set_millis(1);
    REQUIRE(!group.Update());
    REQUIRE(HalMock::PinValue(1) == 0);
    REQUIRE(HalMock::PinValue(2) == 255);
}

TEST_CASE("Stop on group with nested JLedGroup stops inner group", "[jled_group]") {
    HalMock::Init();

    TestJLedAny inner_leds[] = {TestJLed(HalMock(2)).Blink(100, 100)};
    // outer_leds[1] is a TestJLedAny holding a TestJLedGroupAny
    TestJLedAny outer_leds[] = {TestJLed(HalMock(1)).Blink(100, 100),
                                 TestJLedGroupAny::Parallel(inner_leds)};
    auto group = TestJLedGroupAny::Parallel(outer_leds);

    TimeMock::set_millis(0);
    REQUIRE(group.Update());

    group.Stop();
    TimeMock::set_millis(1);
    REQUIRE(!group.Update());
}
