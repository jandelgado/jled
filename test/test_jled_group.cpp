// JLed Unit tests for JLedGroup (run on host).
// Copyright 2017-2026 Jan Delgado jdelgado@gmx.net
#include <jled_base.h>        // NOLINT
#include <jled_group_base.h>  // NOLINT

#include "catch2/catch_amalgamated.hpp"
#include "hal_mock.h"              // NOLINT
#include "mock_brightness_eval.h"  // NOLINT

using namespace jled;  // NOLINT

class TestJLed : public TJLed<HalMock, TimeMock, uint8_t, TestJLed> {
    using TJLed<HalMock, TimeMock, uint8_t, TestJLed>::TJLed;
};

class TestJLedHD : public TJLed<HalMock, TimeMock, uint16_t, TestJLedHD> {
    using TJLed<HalMock, TimeMock, uint16_t, TestJLedHD>::TJLed;
};

namespace {
constexpr size_t kTestLedBufSize = sizeof(TestJLed) > sizeof(TestJLedHD)
                                       ? sizeof(TestJLed)     // NOLINT
                                       : sizeof(TestJLedHD);  // NOLINT
constexpr size_t kTestGroupBufSize = sizeof(TJLedGroup<TimeMock, char>);
constexpr size_t kTestAnyBufSize =
    kTestLedBufSize > kTestGroupBufSize ? kTestLedBufSize : kTestGroupBufSize;  // NOLINT
}  // namespace

using TestJLedAny = TJLedAny<kTestAnyBufSize>;
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
    auto mode = GENERATE(TestJLedGroupAny::eMode::SEQUENCE, TestJLedGroupAny::eMode::PARALLEL);

    auto eval = MockBrightnessEvaluator(std::vector<uint8_t>{255, 0});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval)};
    auto group = TestJLedGroupAny(mode, leds, 1).Repeat(2);

    constexpr uint8_t expected[] = {255, 0, 255, 0};
    for (auto i = 0u; i < sizeof(expected); i++) {
        TimeMock::set_millis(i);
        group.Update();
        INFO("mode=" << static_cast<int>(mode) << ", i=" << i);
        REQUIRE(HalMock::PinValue(1) == expected[i]);
    }
    REQUIRE(!group.Update());
}

TEST_CASE("Forever plays group indefinitely", "[jled_group]") {
    HalMock::Init();
    auto mode = GENERATE(TestJLedGroupAny::eMode::SEQUENCE, TestJLedGroupAny::eMode::PARALLEL);

    auto eval = MockBrightnessEvaluator(std::vector<uint8_t>{255, 0, 0});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval)};
    auto group = TestJLedGroupAny(mode, leds, 1).Forever();

    constexpr uint8_t expected[] = {255, 0, 0};
    constexpr auto num = sizeof(expected) / sizeof(expected[0]);

    for (uint32_t t = 0; t < 1000; t++) {
        TimeMock::set_millis(t);
        INFO("mode=" << static_cast<int>(mode) << ", t=" << t);
        REQUIRE(group.Update());
        REQUIRE(HalMock::PinValue(1) == expected[t % num]);
    }
}

TEST_CASE("IsForever is false initially, true after Forever()", "[jled_group]") {
    auto eval = MockBrightnessEvaluator(std::vector<uint8_t>{255});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval)};
    auto mode = GENERATE(TestJLedGroupAny::eMode::SEQUENCE, TestJLedGroupAny::eMode::PARALLEL);
    auto group = TestJLedGroupAny(mode, leds, 1);
    REQUIRE_FALSE(group.IsForever());
    REQUIRE(group.Forever().IsForever());

    // compile-time check: Repeat and Forever can be chained
    TestJLedGroupAny chained [[gnu::unused]] =
        TestJLedGroupAny(TestJLedGroupAny::eMode::PARALLEL, leds, 1).Repeat(1).Forever();
}

TEST_CASE("Reset restarts group from beginning", "[jled_group]") {
    HalMock::Init();
    auto mode = GENERATE(TestJLedGroupAny::eMode::SEQUENCE, TestJLedGroupAny::eMode::PARALLEL);

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
    auto mode = GENERATE(TestJLedGroupAny::eMode::SEQUENCE, TestJLedGroupAny::eMode::PARALLEL);

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

TEST_CASE("Stop(FULL_OFF) sets group LEDs to 0 regardless of MinBrightness", "[jled_group]") {
    HalMock::Init();
    auto eval = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval).Repeat(1).MinBrightness(50)};
    auto group = TestJLedGroupAny::Parallel(leds);

    TimeMock::set_millis(0);
    REQUIRE(group.Update());

    group.Stop(jled::eIdleMode::FULL_OFF);
    REQUIRE(HalMock::PinValue(1) == 0);
}

TEST_CASE("Stop(TO_MIN_BRIGHTNESS) sets group LEDs to minBrightness", "[jled_group]") {
    HalMock::Init();
    auto eval = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval).Repeat(1).MinBrightness(50)};
    auto group = TestJLedGroupAny::Parallel(leds);

    TimeMock::set_millis(0);
    REQUIRE(group.Update());

    group.Stop(jled::eIdleMode::TO_MIN_BRIGHTNESS);
    REQUIRE(HalMock::PinValue(1) == 50);
}

TEST_CASE("Stop(KEEP_CURRENT) leaves group LEDs at current brightness", "[jled_group]") {
    HalMock::Init();
    auto eval = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval).Repeat(1)};
    auto group = TestJLedGroupAny::Parallel(leds);

    TimeMock::set_millis(0);
    REQUIRE(group.Update());
    REQUIRE(HalMock::PinValue(1) == 200);

    group.Stop(jled::eIdleMode::KEEP_CURRENT);
    REQUIRE(HalMock::PinValue(1) == 200);
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
    TestJLedAny leds[] = {TestJLedHD(HalMock(1)).UserFunc(&eval).Repeat(1).MaxBrightness(0x8000u)};
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
        TestJLedAny arr[] = {src};
        TimeMock::set_millis(0);
        TestJLedGroupAny::Parallel(arr).Update(0);
        REQUIRE(HalMock::PinValue(1) == 255);
    }

    SECTION("copy of JLedGroup") {
        TestJLedAny led_arr[] = {TestJLed(HalMock(1)).UserFunc(&eval).Repeat(1)};
        TestJLedAny src(TestJLedGroupAny::Parallel(led_arr));
        TestJLedAny arr[] = {src};
        TimeMock::set_millis(0);
        TestJLedGroupAny::Parallel(arr).Update(0);
        REQUIRE(HalMock::PinValue(1) == 255);
    }
}

TEST_CASE("As<T>() recovers the concrete stored/referenced type", "[jled_group]") {
    HalMock::Init();
    auto eval = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});

    SECTION("TJLedAny holding a JLed") {
        TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval).Repeat(1)};

        auto* led = leds[0].As<TestJLed>();
        REQUIRE(led != nullptr);
        led->MaxBrightness(123);
        REQUIRE(led->MaxBrightness() == 123);

        REQUIRE(leds[0].As<TestJLedHD>() == nullptr);
    }

    SECTION("TJLedAny holding a nested group") {
        TestJLedAny inner_leds[] = {TestJLed(HalMock(1)).UserFunc(&eval).Repeat(1)};
        TestJLedAny outer[] = {TestJLedGroupAny::Parallel(inner_leds).Forever()};

        auto* group = outer[0].As<TestJLedGroupAny>();
        REQUIRE(group != nullptr);
        REQUIRE(group->IsForever());

        REQUIRE(outer[0].As<TestJLed>() == nullptr);
    }

    SECTION("TJLedRef referencing a JLed") {
        TestJLed led1 = TestJLed(HalMock(1)).UserFunc(&eval).Repeat(1);

        TJLedRef ref(&led1);
        REQUIRE(ref.As<TestJLed>() == &led1);
        REQUIRE(ref.As<TestJLedHD>() == nullptr);
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

    SECTION("referenced LED stays stopped after group's natural completion") {
        TJLedRef refs[] = {&led1};
        auto group = TestJLedRefGroup::Parallel(refs);

        TimeMock::set_millis(0);
        REQUIRE(group.Update());
        TimeMock::set_millis(1);
        auto r = group.Update();
        REQUIRE(!r);
        REQUIRE(r.IsDone());

        // the group is done for good; the referenced LED must not have been
        // silently rearmed for a repetition that will never come.
        REQUIRE_FALSE(led1.IsRunning());
    }
}

TEST_CASE("Pause() default mode is TO_MIN_BRIGHTNESS", "[jled_group]") {
    HalMock::Init();
    auto eval = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval).Repeat(1).MinBrightness(50)};
    auto group = TestJLedGroupAny::Parallel(leds);

    TimeMock::set_millis(0);
    REQUIRE(group.Update());

    TimeMock::set_millis(1);
    group.Pause();
    REQUIRE(HalMock::PinValue(1) == 50);
}

TEST_CASE("Pause() propagates to all children in parallel group", "[jled_group]") {
    HalMock::Init();
    auto eval1 = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100, 50});
    auto eval2 = MockBrightnessEvaluator(std::vector<uint8_t>{150, 75, 25});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval1).Repeat(1),
                          TestJLed(HalMock(2)).UserFunc(&eval2).Repeat(1)};
    auto group = TestJLedGroupAny::Parallel(leds, 2);

    TimeMock::set_millis(0);
    REQUIRE(group.Update());  // t=0: brightness 200, 150

    TimeMock::set_millis(1);
    group.Pause();

    // Updates while paused must return true and not change brightness
    TimeMock::set_millis(1);
    REQUIRE(group.Update());
    REQUIRE(HalMock::PinValue(1) == 0);
    REQUIRE(HalMock::PinValue(2) == 0);

    TimeMock::set_millis(2);
    REQUIRE(group.Update());
    REQUIRE(HalMock::PinValue(1) == 0);
    REQUIRE(HalMock::PinValue(2) == 0);
}

TEST_CASE("Resume() continues parallel group from freeze point", "[jled_group]") {
    HalMock::Init();
    auto eval1 = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    auto eval2 = MockBrightnessEvaluator(std::vector<uint8_t>{150, 50});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval1).Repeat(1),
                          TestJLed(HalMock(2)).UserFunc(&eval2).Repeat(1)};
    auto group = TestJLedGroupAny::Parallel(leds, 2);

    // Advance to t=0 (elapsed=0, first values output)
    TimeMock::set_millis(0);
    REQUIRE(group.Update());

    // Pause at t=0, resume at t=50 — children should see elapsed=1 at t=51
    TimeMock::set_millis(0);
    group.Pause();
    TimeMock::set_millis(50);
    group.Resume();

    TimeMock::set_millis(51);
    REQUIRE(!group.Update());  // elapsed=1, final tick, group done
    REQUIRE(HalMock::PinValue(1) == 100);
    REQUIRE(HalMock::PinValue(2) == 50);
}

TEST_CASE("Pause() freezes sequential group on current LED", "[jled_group]") {
    HalMock::Init();
    auto eval1 = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    auto eval2 = MockBrightnessEvaluator(std::vector<uint8_t>{50, 25});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval1).Repeat(1),
                          TestJLed(HalMock(2)).UserFunc(&eval2).Repeat(1)};
    auto group = TestJLedGroupAny::Sequential(leds);

    TimeMock::set_millis(0);
    REQUIRE(group.Update());  // LED1 at t_cycle=0 -> 200

    TimeMock::set_millis(1);
    group.Pause();

    // Updates while paused: group returns true, LED1 is off (FULL_OFF default), LED2 not started
    TimeMock::set_millis(1);
    REQUIRE(group.Update());
    REQUIRE(HalMock::PinValue(1) == 0);
    REQUIRE(HalMock::PinValue(2) == 0);

    TimeMock::set_millis(5);
    REQUIRE(group.Update());
    REQUIRE(HalMock::PinValue(1) == 0);

    // Resume: LED1 continues from t_cycle=0 -- next tick it finishes (t_cycle=1 -> 100)
    TimeMock::set_millis(10);
    group.Resume();
    TimeMock::set_millis(11);
    REQUIRE(group.Update());  // LED1 finishes, LED2 starts
    REQUIRE(HalMock::PinValue(1) == 100);

    TimeMock::set_millis(12);
    REQUIRE(group.Update());  // LED2 running
    REQUIRE(HalMock::PinValue(2) == 50);

    TimeMock::set_millis(13);
    REQUIRE(!group.Update());  // LED2 finishes, group done
    REQUIRE(HalMock::PinValue(2) == 25);
}

TEST_CASE("Pause()/Resume() propagate through TJLedRef", "[jled_group]") {
    HalMock::Init();
    auto eval1 = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    auto eval2 = MockBrightnessEvaluator(std::vector<uint8_t>{150, 50});
    TestJLed led1 = TestJLed(HalMock(1)).UserFunc(&eval1).Repeat(1);
    TestJLed led2 = TestJLed(HalMock(2)).UserFunc(&eval2).Repeat(1);
    TJLedRef refs[] = {&led1, &led2};
    auto group = TestJLedRefGroup::Parallel(refs);

    TimeMock::set_millis(0);
    REQUIRE(group.Update());  // elapsed=0: 200, 150
    REQUIRE(HalMock::PinValue(1) == 200);
    REQUIRE(HalMock::PinValue(2) == 150);

    // Pause freezes both referenced LEDs (FULL_OFF default)
    TimeMock::set_millis(0);
    group.Pause();
    TimeMock::set_millis(1);
    REQUIRE(group.Update());
    REQUIRE(HalMock::PinValue(1) == 0);
    REQUIRE(HalMock::PinValue(2) == 0);

    // Resume continues from the freeze point: at t=51 elapsed=1 -> final tick
    TimeMock::set_millis(50);
    group.Resume();
    TimeMock::set_millis(51);
    REQUIRE(!group.Update());  // group done
    REQUIRE(HalMock::PinValue(1) == 100);
    REQUIRE(HalMock::PinValue(2) == 50);
}

// --- Lifecycle events (GroupUpdateResult) ---

TEST_CASE("group kStart fires once, on the first Update() call", "[jled_group]") {
    HalMock::Init();
    auto eval = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval)};
    auto group = TestJLedGroupAny::Parallel(leds, 1);

    TimeMock::set_millis(0);
    auto r0 = group.Update();
    CHECK(r0.IsStarted());
    CHECK(r0.IsRunning());

    TimeMock::set_millis(1);
    auto r1 = group.Update();
    CHECK_FALSE(r1.IsStarted());
}

TEST_CASE("group kStart re-arms after Reset()", "[jled_group]") {
    HalMock::Init();
    auto eval = MockBrightnessEvaluator(std::vector<uint8_t>{200});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval)};
    auto group = TestJLedGroupAny::Parallel(leds, 1);

    TimeMock::set_millis(0);
    auto r0 = group.Update();
    CHECK(r0.IsStarted());
    CHECK(r0.IsDone());  // single-tick element -> group also done in one tick

    group.Reset();

    TimeMock::set_millis(1);
    auto r1 = group.Update();
    CHECK(r1.IsStarted());
}

TEST_CASE("group kDone fires exactly once, on the terminal tick (natural completion)",
          "[jled_group]") {
    HalMock::Init();
    auto eval = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval)};
    auto group = TestJLedGroupAny::Parallel(leds, 1);

    int doneCount = 0;
    for (uint32_t t = 0; t < 5; t++) {
        TimeMock::set_millis(t);
        auto r = group.Update();
        if (r.IsDone()) doneCount++;
        if (!r.IsRunning()) break;
    }
    CHECK(doneCount == 1);
}

TEST_CASE("group kDone fires on Update() after Stop(), exactly once", "[jled_group]") {
    HalMock::Init();
    auto eval = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100, 50, 25});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval)};
    auto group = TestJLedGroupAny::Parallel(leds, 1);

    TimeMock::set_millis(0);
    auto r0 = group.Update();
    CHECK(r0.IsRunning());
    CHECK_FALSE(r0.IsDone());

    group.Stop();

    TimeMock::set_millis(1);
    auto r1 = group.Update();
    CHECK_FALSE(r1.IsRunning());
    CHECK(r1.IsDone());

    TimeMock::set_millis(2);
    auto r2 = group.Update();
    CHECK_FALSE(r2.IsDone());
}

TEST_CASE("empty group fires kStart and kDone together on its one Update() call", "[jled_group]") {
    auto group = TestJLedGroupAny::Parallel(nullptr, 0);

    TimeMock::set_millis(0);
    auto r0 = group.Update();
    CHECK(r0.IsStarted());
    CHECK(r0.IsDone());
    CHECK_FALSE(r0.IsRunning());

    TimeMock::set_millis(1);
    auto r1 = group.Update();
    CHECK_FALSE(r1.IsStarted());
    CHECK_FALSE(r1.IsDone());
}

TEST_CASE("group kDone re-arms after Reset() for a second full run", "[jled_group]") {
    HalMock::Init();
    auto eval = MockBrightnessEvaluator(std::vector<uint8_t>{200});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval)};
    auto group = TestJLedGroupAny::Parallel(leds, 1);

    TimeMock::set_millis(0);
    auto r0 = group.Update();
    CHECK(r0.IsDone());

    group.Reset();

    TimeMock::set_millis(1);
    auto r1 = group.Update();
    CHECK(r1.IsDone());
}

TEST_CASE("group OnStart/OnDone chaining invokes callbacks", "[jled_group]") {
    HalMock::Init();
    auto eval = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval)};
    auto group = TestJLedGroupAny::Parallel(leds, 1);

    int startCount = 0, doneCount = 0;
    TimeMock::set_millis(0);
    group.Update().OnStart([&](TestJLedGroupAny*) { startCount++; }).OnDone([&](TestJLedGroupAny*) {
        doneCount++;
    });
    CHECK(startCount == 1);
    CHECK(doneCount == 0);

    TimeMock::set_millis(1);
    group.Update().OnStart([&](TestJLedGroupAny*) { startCount++; }).OnDone([&](TestJLedGroupAny*) {
        doneCount++;
    });
    CHECK(startCount == 1);
    CHECK(doneCount == 1);
}

TEST_CASE("GroupUpdateResult::GetEvents() returns the raw bitmask", "[jled_group]") {
    auto group = TestJLedGroupAny::Parallel(nullptr, 0);

    TimeMock::set_millis(0);
    auto r = group.Update();
    CHECK(r.GetEvents() == (jled::Event::kStart | jled::Event::kRepeatStart | jled::Event::kDone));
}

TEST_CASE("GroupUpdateResult stays usable as a plain bool (backwards compatibility)",
          "[jled_group]") {
    HalMock::Init();
    auto eval1 = MockBrightnessEvaluator(std::vector<uint8_t>{200});
    TestJLedAny leds1[] = {TestJLed(HalMock(1)).UserFunc(&eval1)};
    auto group1 = TestJLedGroupAny::Parallel(leds1, 1);

    TimeMock::set_millis(0);
    bool x = group1.Update();
    CHECK_FALSE(x);  // single-tick element finishes on first tick

    auto eval2 = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    TestJLedAny leds2[] = {TestJLed(HalMock(2)).UserFunc(&eval2)};
    auto group2 = TestJLedGroupAny::Parallel(leds2, 1);

    TimeMock::set_millis(0);
    if (group2.Update()) {
        SUCCEED("group still running after first tick");
    } else {
        FAIL("expected group to still be running");
    }
}

TEST_CASE("group lifecycle events work through JLedRefGroup", "[jled_group]") {
    HalMock::Init();
    auto eval = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100, 50, 25});
    TestJLed led1 = TestJLed(HalMock(1)).UserFunc(&eval).Repeat(1);
    TJLedRef refs[] = {&led1};
    auto group = TestJLedRefGroup::Parallel(refs);

    TimeMock::set_millis(0);
    auto r0 = group.Update();
    CHECK(r0.IsStarted());
    CHECK_FALSE(r0.IsDone());

    group.Stop();

    TimeMock::set_millis(1);
    auto r1 = group.Update();
    CHECK(r1.IsDone());
    CHECK_FALSE(r1.IsRunning());

    TimeMock::set_millis(2);
    auto r2 = group.Update();
    CHECK_FALSE(r2.IsDone());
}

// --- Lifecycle events: kElementChanged (SEQUENCE mode only) ---

TEST_CASE("kElementChanged fires when the sequence advances to the next element", "[jled_group]") {
    HalMock::Init();
    auto eval1 = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    auto eval2 = MockBrightnessEvaluator(std::vector<uint8_t>{50, 25});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval1).Repeat(1),
                          TestJLed(HalMock(2)).UserFunc(&eval2).Repeat(1)};
    auto group = TestJLedGroupAny::Sequential(leds);

    TimeMock::set_millis(0);
    auto r0 = group.Update();  // led1 tick 0
    CHECK_FALSE(r0.IsElementChanged());

    TimeMock::set_millis(1);
    auto r1 = group.Update();  // led1 finishes, cur_ advances to led2
    CHECK(r1.IsElementChanged());

    TimeMock::set_millis(2);
    auto r2 = group.Update();  // led2 tick 0
    CHECK_FALSE(r2.IsElementChanged());

    TimeMock::set_millis(3);
    auto r3 = group.Update();  // led2 finishes, group done (no "next" element)
    CHECK(r3.IsDone());
    CHECK_FALSE(r3.IsElementChanged());
}

TEST_CASE("kElementChanged never fires in PARALLEL mode", "[jled_group]") {
    HalMock::Init();
    auto eval1 = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    auto eval2 = MockBrightnessEvaluator(std::vector<uint8_t>{50, 25});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval1).Repeat(1),
                          TestJLed(HalMock(2)).UserFunc(&eval2).Repeat(1)};
    auto group = TestJLedGroupAny::Parallel(leds, 2);

    for (uint32_t t = 0; t < 2; t++) {
        TimeMock::set_millis(t);
        auto r = group.Update();
        CHECK_FALSE(r.IsElementChanged());
    }
}

TEST_CASE("kElementChanged does not fire for a single-element sequence", "[jled_group]") {
    HalMock::Init();
    auto eval = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval)};
    auto group = TestJLedGroupAny::Sequential(leds).Repeat(3);

    for (uint32_t t = 0; t < 6; t++) {
        TimeMock::set_millis(t);
        auto r = group.Update();
        CHECK_FALSE(r.IsElementChanged());
    }
}

TEST_CASE(
    "kElementChanged fires when the sequence wraps to the first element "
    "for a new repetition",
    "[jled_group]") {
    HalMock::Init();
    auto eval1 = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    auto eval2 = MockBrightnessEvaluator(std::vector<uint8_t>{50, 25});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval1).Repeat(1),
                          TestJLed(HalMock(2)).UserFunc(&eval2).Repeat(1)};
    auto group = TestJLedGroupAny::Sequential(leds).Repeat(2);

    TimeMock::set_millis(0);
    group.Update();  // led1 tick 0
    TimeMock::set_millis(1);
    group.Update();  // led1 finishes -> led2
    TimeMock::set_millis(2);
    group.Update();  // led2 tick 0

    TimeMock::set_millis(3);
    auto r3 = group.Update();  // led2 finishes -> wraps to led1 for repetition 2
    CHECK(r3.IsRunning());
    CHECK_FALSE(r3.IsDone());
    CHECK(r3.IsElementChanged());
}

TEST_CASE("kElementChanged fires on each advance across a 3-element sequence", "[jled_group]") {
    HalMock::Init();
    auto eval1 = MockBrightnessEvaluator(std::vector<uint8_t>{200});
    auto eval2 = MockBrightnessEvaluator(std::vector<uint8_t>{100});
    auto eval3 = MockBrightnessEvaluator(std::vector<uint8_t>{50, 25});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval1),
                          TestJLed(HalMock(2)).UserFunc(&eval2),
                          TestJLed(HalMock(3)).UserFunc(&eval3)};
    auto group = TestJLedGroupAny::Sequential(leds);

    // led1 and led2 are single-tick, so both advances land on t=0 (led1->led2)
    // and t=1 (led2->led3): two independent kElementChanged fires in one run.
    TimeMock::set_millis(0);
    auto r0 = group.Update();  // led1 finishes instantly -> led2
    CHECK(r0.IsElementChanged());

    TimeMock::set_millis(1);
    auto r1 = group.Update();  // led2 finishes instantly -> led3
    CHECK(r1.IsElementChanged());

    TimeMock::set_millis(2);
    auto r2 = group.Update();  // led3 tick 0
    CHECK_FALSE(r2.IsElementChanged());

    TimeMock::set_millis(3);
    auto r3 = group.Update();  // led3 finishes, group done
    CHECK(r3.IsDone());
    CHECK_FALSE(r3.IsElementChanged());
}

TEST_CASE("kElementChanged fires on wraparound with Forever()", "[jled_group]") {
    HalMock::Init();
    auto eval1 = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    auto eval2 = MockBrightnessEvaluator(std::vector<uint8_t>{50, 25});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval1).Repeat(1),
                          TestJLed(HalMock(2)).UserFunc(&eval2).Repeat(1)};
    auto group = TestJLedGroupAny::Sequential(leds).Forever();

    TimeMock::set_millis(0);
    group.Update();  // led1 tick 0
    TimeMock::set_millis(1);
    group.Update();  // led1 finishes -> led2
    TimeMock::set_millis(2);
    group.Update();  // led2 tick 0

    TimeMock::set_millis(3);
    auto r3 = group.Update();  // led2 finishes -> wraps to led1, group keeps running forever
    CHECK(r3.IsRunning());
    CHECK_FALSE(r3.IsDone());
    CHECK(r3.IsElementChanged());
}

// --- Lifecycle events: kRepeatStart (group-level, mode-independent) ---

TEST_CASE("group kRepeatStart fires on the first Update() call, coincident with kStart",
          "[jled_group]") {
    HalMock::Init();
    auto mode = GENERATE(TestJLedGroupAny::eMode::SEQUENCE, TestJLedGroupAny::eMode::PARALLEL);
    auto eval = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval)};
    auto group = TestJLedGroupAny(mode, leds, 1);

    TimeMock::set_millis(0);
    auto r0 = group.Update();
    INFO("mode=" << static_cast<int>(mode));
    CHECK(r0.IsStarted());
    CHECK(r0.IsRepeatStarted());

    TimeMock::set_millis(1);
    auto r1 = group.Update();
    CHECK_FALSE(r1.IsStarted());
    CHECK_FALSE(r1.IsRepeatStarted());
}

TEST_CASE(
    "group kRepeatStart fires once per repetition, including the first, "
    "independent of mode",
    "[jled_group]") {
    HalMock::Init();
    auto mode = GENERATE(TestJLedGroupAny::eMode::SEQUENCE, TestJLedGroupAny::eMode::PARALLEL);
    auto eval = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval)};
    auto group = TestJLedGroupAny(mode, leds, 1).Repeat(3);

    int repeatStartCount = 0;
    for (uint32_t t = 0; t < 6; t++) {
        TimeMock::set_millis(t);
        auto r = group.Update();
        if (r.IsRepeatStarted()) repeatStartCount++;
    }
    INFO("mode=" << static_cast<int>(mode));
    CHECK(repeatStartCount == 3);
}

TEST_CASE("group kRepeatStart does not fire on the terminal tick when no repetitions remain",
          "[jled_group]") {
    HalMock::Init();
    auto mode = GENERATE(TestJLedGroupAny::eMode::SEQUENCE, TestJLedGroupAny::eMode::PARALLEL);
    auto eval = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval)};
    auto group = TestJLedGroupAny(mode, leds, 1).Repeat(2);

    TimeMock::set_millis(0);
    group.Update();
    TimeMock::set_millis(1);
    auto r1 = group.Update();  // lap 1 done, lap 2 begins
    CHECK(r1.IsRepeatStarted());

    TimeMock::set_millis(2);
    group.Update();
    TimeMock::set_millis(3);
    auto r3 = group.Update();  // lap 2 done, group finished
    INFO("mode=" << static_cast<int>(mode));
    CHECK(r3.IsDone());
    CHECK_FALSE(r3.IsRepeatStarted());
}

TEST_CASE(
    "kRepeatStart and kElementChanged coincide when a SEQUENCE group wraps "
    "for a new repetition",
    "[jled_group]") {
    HalMock::Init();
    auto eval1 = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    auto eval2 = MockBrightnessEvaluator(std::vector<uint8_t>{50, 25});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval1).Repeat(1),
                          TestJLed(HalMock(2)).UserFunc(&eval2).Repeat(1)};
    auto group = TestJLedGroupAny::Sequential(leds).Repeat(2);

    TimeMock::set_millis(0);
    group.Update();
    TimeMock::set_millis(1);
    group.Update();
    TimeMock::set_millis(2);
    group.Update();

    TimeMock::set_millis(3);
    auto r3 = group.Update();  // led2 finishes -> wraps to led1 for repetition 2
    CHECK(r3.IsElementChanged());
    CHECK(r3.IsRepeatStarted());
}

TEST_CASE("kRepeatStart fires in PARALLEL mode even though kElementChanged never does",
          "[jled_group]") {
    HalMock::Init();
    auto eval = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval)};
    auto group = TestJLedGroupAny::Parallel(leds, 1).Repeat(2);

    TimeMock::set_millis(0);
    group.Update();
    TimeMock::set_millis(1);
    auto r1 = group.Update();  // lap 1 done, lap 2 begins
    CHECK(r1.IsRepeatStarted());
    CHECK_FALSE(r1.IsElementChanged());
}

TEST_CASE("empty group also fires kRepeatStart together with kStart and kDone", "[jled_group]") {
    auto group = TestJLedGroupAny::Parallel(nullptr, 0);

    TimeMock::set_millis(0);
    auto r0 = group.Update();
    CHECK(r0.IsStarted());
    CHECK(r0.IsRepeatStarted());
    CHECK(r0.IsDone());
}

TEST_CASE("group kRepeatStart re-arms after Reset()", "[jled_group]") {
    HalMock::Init();
    auto eval = MockBrightnessEvaluator(std::vector<uint8_t>{200});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval)};
    auto group = TestJLedGroupAny::Sequential(leds);

    TimeMock::set_millis(0);
    auto r0 = group.Update();
    CHECK(r0.IsRepeatStarted());

    group.Reset();

    TimeMock::set_millis(1);
    auto r1 = group.Update();
    CHECK(r1.IsRepeatStarted());
}

TEST_CASE("group OnRepeatStart callback invokes on each repetition boundary", "[jled_group]") {
    HalMock::Init();
    auto eval = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval)};
    auto group = TestJLedGroupAny::Sequential(leds).Repeat(2);

    int repeatStartCount = 0;
    TimeMock::set_millis(0);
    group.Update().OnRepeatStart([&](TestJLedGroupAny*) { repeatStartCount++; });
    CHECK(repeatStartCount == 1);

    TimeMock::set_millis(1);
    group.Update().OnRepeatStart([&](TestJLedGroupAny*) { repeatStartCount++; });
    CHECK(repeatStartCount == 2);

    TimeMock::set_millis(2);
    group.Update().OnRepeatStart([&](TestJLedGroupAny*) { repeatStartCount++; });
    CHECK(repeatStartCount == 2);
}

TEST_CASE(
    "kElementChanged and kStart coincide when the first element finishes "
    "on the very first tick",
    "[jled_group]") {
    HalMock::Init();
    auto eval1 = MockBrightnessEvaluator(std::vector<uint8_t>{200});
    auto eval2 = MockBrightnessEvaluator(std::vector<uint8_t>{50, 25});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval1),
                          TestJLed(HalMock(2)).UserFunc(&eval2)};
    auto group = TestJLedGroupAny::Sequential(leds);

    TimeMock::set_millis(0);
    auto r0 = group.Update();
    CHECK(r0.IsStarted());
    CHECK(r0.IsElementChanged());
    CHECK(r0.IsRunning());
}

TEST_CASE("OnElementChanged callback invokes on the tick the active element changes",
          "[jled_group]") {
    HalMock::Init();
    auto eval1 = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    auto eval2 = MockBrightnessEvaluator(std::vector<uint8_t>{50, 25});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval1).Repeat(1),
                          TestJLed(HalMock(2)).UserFunc(&eval2).Repeat(1)};
    auto group = TestJLedGroupAny::Sequential(leds);

    int changedCount = 0;
    TimeMock::set_millis(0);
    group.Update().OnElementChanged([&](TestJLedGroupAny*) { changedCount++; });
    CHECK(changedCount == 0);

    TimeMock::set_millis(1);
    group.Update().OnElementChanged([&](TestJLedGroupAny*) { changedCount++; });
    CHECK(changedCount == 1);

    TimeMock::set_millis(2);
    group.Update().OnElementChanged([&](TestJLedGroupAny*) { changedCount++; });
    CHECK(changedCount == 1);
}

// --- Lifecycle events: OnEnter/OnLeave (derived from kStart/kElementChanged/kDone) ---

TEST_CASE("OnEnter/OnLeave report correct indices across a 2-element sequence", "[jled_group]") {
    HalMock::Init();
    auto eval1 = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    auto eval2 = MockBrightnessEvaluator(std::vector<uint8_t>{50, 25});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval1).Repeat(1),
                          TestJLed(HalMock(2)).UserFunc(&eval2).Repeat(1)};
    auto group = TestJLedGroupAny::Sequential(leds);

    TimeMock::set_millis(0);
    auto r0 = group.Update();  // led1 (index 0) enters
    uint8_t enter0 = 255;
    CHECK(r0.IsEnter());
    r0.OnEnter([&](TestJLedGroupAny*, uint8_t idx) { enter0 = idx; });
    CHECK(enter0 == 0);
    CHECK_FALSE(r0.IsLeave());

    TimeMock::set_millis(1);
    auto r1 = group.Update();  // led1 leaves (0), led2 enters (1)
    uint8_t enter1 = 255, leave1 = 255;
    r1.OnEnter([&](TestJLedGroupAny*, uint8_t idx) {
          enter1 = idx;
      }).OnLeave([&](TestJLedGroupAny*, uint8_t idx) { leave1 = idx; });
    CHECK(enter1 == 1);
    CHECK(leave1 == 0);

    TimeMock::set_millis(2);
    auto r2 = group.Update();  // led2 tick 0
    CHECK_FALSE(r2.IsEnter());
    CHECK_FALSE(r2.IsLeave());

    TimeMock::set_millis(3);
    auto r3 = group.Update();  // led2 (last element, index 1) leaves, group done
    uint8_t leave3 = 255;
    CHECK(r3.IsDone());
    r3.OnLeave([&](TestJLedGroupAny*, uint8_t idx) { leave3 = idx; });
    CHECK(leave3 == 1);
    CHECK_FALSE(r3.IsEnter());
}

TEST_CASE("OnEnter/OnLeave indices are correct across a Repeat(2) wrap", "[jled_group]") {
    HalMock::Init();
    auto eval1 = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    auto eval2 = MockBrightnessEvaluator(std::vector<uint8_t>{50, 25});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval1).Repeat(1),
                          TestJLed(HalMock(2)).UserFunc(&eval2).Repeat(1)};
    auto group = TestJLedGroupAny::Sequential(leds).Repeat(2);

    TimeMock::set_millis(0);
    group.Update();  // led1 (0) enters, lap 1
    TimeMock::set_millis(1);
    group.Update();  // led1 leaves (0), led2 (1) enters
    TimeMock::set_millis(2);
    group.Update();  // led2 tick 0

    TimeMock::set_millis(3);
    auto r3 = group.Update();  // led2 leaves (1), wraps: led1 (0) enters for lap 2
    uint8_t enter3 = 255, leave3 = 255;
    CHECK(r3.IsRepeatStarted());
    r3.OnEnter([&](TestJLedGroupAny*, uint8_t idx) {
          enter3 = idx;
      }).OnLeave([&](TestJLedGroupAny*, uint8_t idx) { leave3 = idx; });
    CHECK(enter3 == 0);
    CHECK(leave3 == 1);

    TimeMock::set_millis(4);
    group.Update();  // led1 tick 0, lap 2

    TimeMock::set_millis(5);
    auto r5 = group.Update();  // led1 leaves (0), led2 (1) enters, lap 2
    uint8_t enter5 = 255, leave5 = 255;
    r5.OnEnter([&](TestJLedGroupAny*, uint8_t idx) {
          enter5 = idx;
      }).OnLeave([&](TestJLedGroupAny*, uint8_t idx) { leave5 = idx; });
    CHECK(enter5 == 1);
    CHECK(leave5 == 0);

    TimeMock::set_millis(6);
    group.Update();  // led2 tick 0, lap 2

    TimeMock::set_millis(7);
    auto r7 = group.Update();  // led2 leaves (1), no more repetitions, group done
    uint8_t leave7 = 255;
    CHECK(r7.IsDone());
    r7.OnLeave([&](TestJLedGroupAny*, uint8_t idx) { leave7 = idx; });
    CHECK(leave7 == 1);
    CHECK_FALSE(r7.IsEnter());
}

TEST_CASE("OnEnter/OnLeave alias OnStart/OnDone with index 0 in PARALLEL mode", "[jled_group]") {
    HalMock::Init();
    auto eval1 = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    auto eval2 = MockBrightnessEvaluator(std::vector<uint8_t>{50, 25});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval1).Repeat(1),
                          TestJLed(HalMock(2)).UserFunc(&eval2).Repeat(1)};
    auto group = TestJLedGroupAny::Parallel(leds, 2);

    TimeMock::set_millis(0);
    auto r0 = group.Update();
    uint8_t enter0 = 255;
    CHECK(r0.IsEnter() == r0.IsStarted());
    r0.OnEnter([&](TestJLedGroupAny*, uint8_t idx) { enter0 = idx; });
    CHECK(enter0 == 0);
    CHECK_FALSE(r0.IsLeave());

    TimeMock::set_millis(1);
    auto r1 = group.Update();  // both elements finish simultaneously, group done
    uint8_t leave1 = 255;
    CHECK(r1.IsLeave() == r1.IsDone());
    r1.OnLeave([&](TestJLedGroupAny*, uint8_t idx) { leave1 = idx; });
    CHECK(leave1 == 0);
    CHECK_FALSE(r1.IsEnter());
}

TEST_CASE("OnEnter/OnLeave do not fire on an empty group's one and only tick", "[jled_group]") {
    auto group = TestJLedGroupAny::Parallel(nullptr, 0);

    TimeMock::set_millis(0);
    auto r0 = group.Update();
    CHECK(r0.IsStarted());
    CHECK(r0.IsDone());

    // there is no element to enter or leave, so neither fires, even though
    // IsStarted()/IsDone() (and the kStart/kDone bits IsEnter()/IsLeave()
    // would otherwise alias) are both true on this tick.
    CHECK_FALSE(r0.IsEnter());
    CHECK_FALSE(r0.IsLeave());

    uint8_t enter0 = 255, leave0 = 255;
    r0.OnEnter([&](TestJLedGroupAny*, uint8_t idx) {
          enter0 = idx;
      }).OnLeave([&](TestJLedGroupAny*, uint8_t idx) { leave0 = idx; });
    CHECK(enter0 == 255);
    CHECK(leave0 == 255);
}

TEST_CASE(
    "OnEnter/OnLeave alias OnStart/OnDone for a single-element sequence, "
    "silent on mid-run repeat boundaries",
    "[jled_group]") {
    HalMock::Init();
    auto eval = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval)};
    auto group = TestJLedGroupAny::Sequential(leds).Repeat(2);

    TimeMock::set_millis(0);
    auto r0 = group.Update();  // lap 1 tick 0
    uint8_t enter0 = 255;
    CHECK(r0.IsEnter());
    r0.OnEnter([&](TestJLedGroupAny*, uint8_t idx) { enter0 = idx; });
    CHECK(enter0 == 0);

    TimeMock::set_millis(1);
    auto r1 = group.Update();
    // Lap 1 finishes and wraps to lap 2 in this same tick (IsRepeatStarted()
    // fires), but cur_ stays 0 the whole time on a single-element sequence,
    // so kElementChanged never fires here (documented, existing behavior).
    // OnEnter/OnLeave derive only from kStart/kElementChanged/kDone, so they
    // stay silent on this boundary even though a repeat just happened.
    CHECK(r1.IsRepeatStarted());
    CHECK_FALSE(r1.IsEnter());
    CHECK_FALSE(r1.IsLeave());

    TimeMock::set_millis(2);
    auto r2 = group.Update();  // lap 2 tick 0
    CHECK_FALSE(r2.IsEnter());
    CHECK_FALSE(r2.IsLeave());

    TimeMock::set_millis(3);
    auto r3 = group.Update();  // lap 2 finishes, no more repetitions, group done
    uint8_t leave3 = 255;
    CHECK(r3.IsDone());
    r3.OnLeave([&](TestJLedGroupAny*, uint8_t idx) { leave3 = idx; });
    CHECK(leave3 == 0);
}

TEST_CASE("OnEnter/OnLeave report correct indices regardless of chain order", "[jled_group]") {
    HalMock::Init();
    auto eval1 = MockBrightnessEvaluator(std::vector<uint8_t>{200, 100});
    auto eval2 = MockBrightnessEvaluator(std::vector<uint8_t>{50, 25});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval1).Repeat(1),
                          TestJLed(HalMock(2)).UserFunc(&eval2).Repeat(1)};
    auto group = TestJLedGroupAny::Sequential(leds);

    TimeMock::set_millis(0);
    group.Update();  // led1 tick 0

    TimeMock::set_millis(1);
    auto r1 = group.Update();  // led1 leaves (0), led2 enters (1)
    uint8_t enter_idx = 255, leave_idx = 255;
    // OnEnter() chained before OnLeave() here, the opposite order used in
    // the tests above: the result must be identical either way, since both
    // indices are resolved once at construction time, not read live off
    // shared, mutable state.
    r1.OnEnter([&](TestJLedGroupAny*, uint8_t idx) {
          enter_idx = idx;
      }).OnLeave([&](TestJLedGroupAny*, uint8_t idx) { leave_idx = idx; });
    CHECK(enter_idx == 1);
    CHECK(leave_idx == 0);
}

// --- Duplicate-tick protection (calling Update() more than once for the same t) ---

TEST_CASE(
    "a duplicate Update() call at the same t does not advance the group "
    "past a repetition boundary",
    "[jled_group]") {
    HalMock::Init();
    auto mode = GENERATE(TestJLedGroupAny::eMode::SEQUENCE, TestJLedGroupAny::eMode::PARALLEL);
    // single-tick element: every Update() call is a repetition boundary
    auto eval = MockBrightnessEvaluator(std::vector<uint8_t>{200});
    TestJLedAny leds[] = {TestJLed(HalMock(1)).UserFunc(&eval)};
    auto group = TestJLedGroupAny(mode, leds, 1).Repeat(3);

    TimeMock::set_millis(0);
    auto r0 = group.Update();  // lap 1 begins and completes on this single tick
    INFO("mode=" << static_cast<int>(mode));
    CHECK(r0.IsRepeatStarted());
    CHECK(r0.IsRunning());

    // calling Update() again at the SAME t must not silently start lap 2
    auto r0dup = group.Update();
    CHECK_FALSE(r0dup.IsRepeatStarted());
    CHECK_FALSE(r0dup.IsDone());
    CHECK(r0dup.IsRunning());

    // advancing time now correctly starts lap 2
    TimeMock::set_millis(1);
    auto r1 = group.Update();
    CHECK(r1.IsRepeatStarted());
}
