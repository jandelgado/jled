// JLed Unit tests (runs on host)
// Copyright 2017-2022 Jan Delgado jdelgado@gmx.net
#include <jled_base.h>  // NOLINT
#include <iostream>
#include <limits>
#include <map>
#include <utility>
#include <vector>
#include "catch.hpp"
#include "hal_mock.h"  // NOLINT

using jled::BlinkBrightnessEvaluator;
using jled::BreatheBrightnessEvaluator;
using jled::BrightnessEvaluator;
using jled::CandleBrightnessEvaluator;
using jled::ConstantBrightnessEvaluator;
using jled::FadeOffBrightnessEvaluator;
using jled::FadeOnBrightnessEvaluator;
using jled::TJLed;

// TestJLed is a JLed class using the HalMock for tests. This allows to
// test the code abstracted from the actual hardware in use.
class TestJLed : public TJLed<HalMock, TestJLed> {
    using TJLed<HalMock, TestJLed>::TJLed;
};
// instanciate for test coverage measurement
template class TJLed<HalMock, TestJLed>;

using ByteVec = std::vector<uint8_t>;

class MockBrightnessEvaluator : public BrightnessEvaluator {
    ByteVec values_;
    mutable uint16_t count_ = 0;

 public:
    explicit MockBrightnessEvaluator(ByteVec values) : values_(values) {}
    uint16_t Count() const { return count_; }
    uint16_t Period() const { return values_.size(); }
    uint8_t Eval(uint32_t t) const {
        REQUIRE(t < values_.size());
        count_++;
        return values_[t];
    }
};

// helper to check that a JLed objects evaluates to the given values
#define CHECK_LED(led, all_expected)               \
    do {                                           \
        uint32_t time = 0;                         \
        for (const auto expected : all_expected) { \
            INFO("t=" << time);                    \
            jled.Update();                         \
            CHECK(expected == jled.Hal().Value()); \
            jled.Hal().SetMillis(++time);          \
        }                                          \
    } while (0)

TEST_CASE("jled without effect does nothing", "[jled]") {
    auto led = TestJLed(1);
    REQUIRE(!led.Update());
}

TEST_CASE("On/Off function configuration", "[jled]") {
    // class used to access proteced fields during test
    class TestableJLed : public TestJLed {
     public:
        using TestJLed::TestJLed;
        static void test() {
            SECTION(
                "using On() effect uses a BrightnessEval that turns the LED "
                "on") {
                TestableJLed jled(1);
                jled.On();
                REQUIRE(dynamic_cast<ConstantBrightnessEvaluator *>(
                            jled.brightness_eval_) != nullptr);
                REQUIRE(jled.brightness_eval_->Eval(0) == 255);
            }

            SECTION(
                "using Off() effect uses a BrightnessEval that turns the LED "
                "off") {
                TestableJLed jled(1);
                jled.Off();
                REQUIRE(dynamic_cast<ConstantBrightnessEvaluator *>(
                            jled.brightness_eval_) != nullptr);
                REQUIRE(jled.brightness_eval_->Eval(0) == 0);
            }

            SECTION("using Set() allows to set custom brightness level") {
                TestableJLed jled(1);
                jled.Set(123);
                REQUIRE(dynamic_cast<ConstantBrightnessEvaluator *>(
                            jled.brightness_eval_) != nullptr);
                REQUIRE(jled.brightness_eval_->Eval(0) == 123);
            }

            SECTION("using Set(0) allows to set custom turn LED off") {
                TestableJLed jled(1);
                jled.Set(0);
                REQUIRE(dynamic_cast<ConstantBrightnessEvaluator *>(
                            jled.brightness_eval_) != nullptr);
                REQUIRE(jled.brightness_eval_->Eval(0) == 0);
            }
        }
    };
    TestableJLed::test();
}

TEST_CASE("using Breathe() configures BreatheBrightnessEvaluator", "[jled]") {
    class TestableJLed : public TestJLed {
     public:
        using TestJLed::TestJLed;
        static void test() {
            TestableJLed jled(1);
            jled.Breathe(0);
            REQUIRE(dynamic_cast<BreatheBrightnessEvaluator *>(
                        jled.brightness_eval_) != nullptr);
        }
    };
    TestableJLed::test();
}

TEST_CASE("using Candle() configures CandleBrightnessEvaluator", "[jled]") {
    class TestableJLed : public TestJLed {
     public:
        using TestJLed::TestJLed;
        static void test() {
            TestableJLed jled(1);
            jled.Candle(1, 2, 3);
            REQUIRE(dynamic_cast<CandleBrightnessEvaluator *>(
                        jled.brightness_eval_) != nullptr);
        }
    };
    TestableJLed::test();
}

TEST_CASE("using Fadeon(), FadeOff() configures Fade-BrightnessEvaluators",
          "[jled]") {
    class TestableJLed : public TestJLed {
     public:
        using TestJLed::TestJLed;
        static void test() {
            SECTION("FadeOff() initializes with FadeOffBrightnessEvaluator") {
                TestableJLed jled(1);
                jled.FadeOff(0);
                REQUIRE(dynamic_cast<FadeOffBrightnessEvaluator *>(
                            jled.brightness_eval_) != nullptr);
            }
            SECTION("FadeOn() initializes with FadeOnBrightnessEvaluator") {
                TestableJLed jled(1);
                jled.FadeOn(0);
                REQUIRE(dynamic_cast<FadeOnBrightnessEvaluator *>(
                            jled.brightness_eval_) != nullptr);
            }
        }
    };
    TestableJLed::test();
}

TEST_CASE("UserFunc() allows to use a custom brightness evaluator", "[jled]") {
    class TestableJLed : public TestJLed {
     public:
        using TestJLed::TestJLed;
        static void test() {
            TestableJLed jled(1);
            auto cust = MockBrightnessEvaluator(ByteVec{});
            jled.UserFunc(&cust);
            REQUIRE(dynamic_cast<MockBrightnessEvaluator *>(
                        jled.brightness_eval_) != nullptr);
        }
    };
    TestableJLed::test();
}

TEST_CASE("ConstantBrightnessEvaluator returns constant provided value",
          "[jled]") {
    auto cbZero = ConstantBrightnessEvaluator(0);
    REQUIRE(1 == cbZero.Period());
    REQUIRE(0 == cbZero.Eval(0));
    REQUIRE(0 == cbZero.Eval(1000));

    auto cbFull = ConstantBrightnessEvaluator(255);
    REQUIRE(1 == cbFull.Period());
    REQUIRE(255 == cbFull.Eval(0));
    REQUIRE(255 == cbFull.Eval(1000));
}

TEST_CASE(
    "BlinkBrightnessEvaluator calculates switches between on and off in given "
    "time frames",
    "[jled]") {
    auto eval = BlinkBrightnessEvaluator(10, 5);
    REQUIRE(10 + 5 == eval.Period());
    REQUIRE(255 == eval.Eval(0));
    REQUIRE(255 == eval.Eval(9));
    REQUIRE(0 == eval.Eval(10));
    REQUIRE(0 == eval.Eval(14));
}

TEST_CASE("CandleBrightnessEvaluator simulated candle flickering", "[jled]") {
    auto eval = CandleBrightnessEvaluator(7, 15, 1000);
    REQUIRE(1000 == eval.Period());
    // TODO(jd) do further and better tests
    REQUIRE(eval.Eval(0) > 0);
    REQUIRE(eval.Eval(999) > 0);
}

TEST_CASE("FadeOnEvaluator evaluates to expected brightness curve", "[jled]") {
    constexpr auto kPeriod = 2000;

    auto evalOn = FadeOnBrightnessEvaluator(kPeriod);

    REQUIRE(kPeriod == evalOn.Period());

    const std::map<uint32_t, uint8_t> test_values = {
        {0, 0},      {500, 13},   {1000, 68},  {1500, 179},
        {1999, 255}, {2000, 255}, {10000, 255}};

    for (const auto &x : test_values) {
        REQUIRE(x.second == evalOn.Eval(x.first));
    }
}

TEST_CASE("FadeOffEvaluator evaluates to expected brightness curve", "[jled]") {
    constexpr auto kPeriod = 2000;

    // note: FadeOff is invervted FadeOn
    auto evalOff = FadeOffBrightnessEvaluator(kPeriod);

    REQUIRE(kPeriod == evalOff.Period());
    const std::map<uint32_t, uint8_t> test_values = {
        {0, 0},      {500, 13},   {1000, 68},  {1500, 179},
        {1999, 255}, {2000, 255}, {10000, 255}};

    for (const auto &x : test_values) {
        REQUIRE(x.second == evalOff.Eval(kPeriod - x.first));
    }
}

TEST_CASE(
    "BreatheEvaluator evaluates to bell curve distributed brightness curve",
    "[jled]") {
    auto eval = BreatheBrightnessEvaluator(100, 200, 300);
    REQUIRE(100 + 200 + 300 == eval.Period());

    const std::map<uint32_t, uint8_t> test_values = {
        {0, 0},     {50, 68},   {80, 198},  {99, 255}, {100, 255},
        {299, 255}, {300, 255}, {399, 138}, {499, 26}, {599, 0}};

    for (const auto &x : test_values) {
        INFO("t=" << x.first);
        REQUIRE((int)x.second == (int)eval.Eval(x.first));
    }
}

TEST_CASE("Forever flag is initially set to false", "[jled]") {
    TestJLed jled(1);
    REQUIRE_FALSE(jled.IsForever());
}

TEST_CASE("Forever flag is set by call to Forever()", "[jled]") {
    TestJLed jled(1);
    jled.Forever();
    REQUIRE(jled.IsForever());
}

TEST_CASE("dont evalute twice during one time tick", "[jled]") {
    auto eval = MockBrightnessEvaluator(ByteVec{0, 1, 2});
    TestJLed jled = TestJLed(1).UserFunc(&eval);

    jled.Hal().SetMillis(0);
    jled.Update();
    REQUIRE(eval.Count() == 1);
    jled.Update();
    REQUIRE(eval.Count() == 1);

    jled.Hal().SetMillis(1);
    jled.Update();

    REQUIRE(eval.Count() == 2);
}

TEST_CASE("Handles millis overflow during effect", "[jled]") {
    TestJLed jled = TestJLed(10);
    // Set time close to overflow
    auto time = std::numeric_limits<uint32_t>::max() - 25;
    jled.Hal().SetMillis(time);
    REQUIRE_FALSE(jled.Update());
    // Start fade off
    jled.FadeOff(100);
    REQUIRE(jled.Update());
    REQUIRE(jled.IsRunning());
    REQUIRE(jled.Hal().Value() > 0);
    // Set time after overflow, before effect ends
    jled.Hal().SetMillis(time + 50);
    REQUIRE(jled.Update());
    REQUIRE(jled.IsRunning());
    REQUIRE(jled.Hal().Value() > 0);
    // Set time after effect ends
    jled.Hal().SetMillis(time + 150);
    REQUIRE_FALSE(jled.Update());
    REQUIRE_FALSE(jled.IsRunning());
    REQUIRE(0 == jled.Hal().Value());
}

TEST_CASE("Stop() stops the effect", "[jled]") {
    auto eval = MockBrightnessEvaluator(ByteVec{255, 255, 255, 0});
    TestJLed jled = TestJLed(10).UserFunc(&eval);

    REQUIRE(jled.IsRunning());
    jled.Update();
    REQUIRE(jled.Hal().Value() == 255);
    jled.Stop();

    REQUIRE(!jled.IsRunning());
    REQUIRE_FALSE(jled.Update());
    REQUIRE(0 == jled.Hal().Value());

    // update must not change anything
    REQUIRE_FALSE(jled.Update());
    REQUIRE(0 == jled.Hal().Value());
}

TEST_CASE("LowActive() inverts signal", "[jled]") {
    auto eval = MockBrightnessEvaluator(ByteVec{255});
    TestJLed jled = TestJLed(10).UserFunc(&eval).LowActive();

    REQUIRE(jled.IsLowActive());

    jled.Update();
    REQUIRE(0 == jled.Hal().Value());

    jled.Stop();
    REQUIRE(255 == jled.Hal().Value());
}

TEST_CASE("effect with repeat 2 runs twice as long", "[jled]") {
    auto eval = MockBrightnessEvaluator(ByteVec{10, 20});
    TestJLed jled = TestJLed(10).UserFunc(&eval).Repeat(2);

    auto expected = ByteVec{10, 20, 10, 20, 20, 20};

    CHECK_LED(jled, expected);
}

TEST_CASE("effect with delay after delays start of next iteration", "[jled]") {
    auto eval = MockBrightnessEvaluator(ByteVec{10, 20});
    TestJLed jled = TestJLed(10).UserFunc(&eval).Repeat(2).DelayAfter(2);

    auto expected = ByteVec{// Eval  Delay
                            10, 20, 20, 20, 10, 20, 20, 20,
                            // Final
                            20, 20};

    CHECK_LED(jled, expected);
}

TEST_CASE("effect with delay before has delayed start ", "[jled]") {
    auto eval = MockBrightnessEvaluator(ByteVec{10, 20});
    TestJLed jled = TestJLed(10).UserFunc(&eval).DelayBefore(2);

    auto expected = ByteVec{0, 0, 10, 20, 20, 20, 20};

    CHECK_LED(jled, expected);
}

TEST_CASE("After calling Forever() the effect is repeated over and over again ",
          "[jled]") {
    auto eval = MockBrightnessEvaluator(ByteVec{10, 20});
    TestJLed jled = TestJLed(10).UserFunc(&eval).Forever();

    auto expected = ByteVec{10, 20, 10, 20, 10, 20};

    CHECK_LED(jled, expected);
}

TEST_CASE("The Hal object provided in the ctor is used during update",
          "[jled]") {
    auto eval = MockBrightnessEvaluator(ByteVec{10, 20});
    TestJLed jled = TestJLed(HalMock(123)).UserFunc(&eval);

    REQUIRE(jled.Hal().Pin() == 123);
}

TEST_CASE("Update returns true while updating, else false", "[jled]") {
    auto eval = MockBrightnessEvaluator(ByteVec{10, 20});
    TestJLed jled = TestJLed(10).UserFunc(&eval);

    // Update returns FALSE on last step and beyond, else TRUE
    auto time = 0;
    jled.Hal().SetMillis(time++);
    REQUIRE(jled.Update());

    // when effect is done, we expect still false to be returned
    jled.Hal().SetMillis(time++);
    REQUIRE_FALSE(jled.Update());
    jled.Hal().SetMillis(time++);
    REQUIRE_FALSE(jled.Update());
}

TEST_CASE("After Reset() the effect can be restarted", "[jled]") {
    auto eval = MockBrightnessEvaluator(ByteVec{10, 20});
    TestJLed jled = TestJLed(10).UserFunc(&eval);

    uint32_t time = 0;
    typedef std::pair<bool, uint8_t> p;

    constexpr p expected[]{p{true, 10}, p{false, 20}, p{false, 20},
                           p{false, 20}};

    for (const auto &x : expected) {
        jled.Hal().SetMillis(time++);
        REQUIRE(x.first == jled.Update());
        REQUIRE(x.second == jled.Hal().Value());
    }

    // after Reset() effect starts over
    jled.Reset();
    for (const auto &x : expected) {
        jled.Hal().SetMillis(time++);
        REQUIRE(x.first == jled.Update());
        REQUIRE(x.second == jled.Hal().Value());
    }
}

TEST_CASE("Changing the effect resets object and starts over", "[jled]") {
    auto eval = MockBrightnessEvaluator(ByteVec{10, 20});
    TestJLed jled = TestJLed(10).UserFunc(&eval);
    uint32_t time = 0;
    typedef std::pair<bool, uint8_t> p;

    constexpr p expected[]{p{true, 10}, p{false, 20}, p{false, 20}};

    for (const auto &x : expected) {
        jled.Hal().SetMillis(time++);
        REQUIRE(x.first == jled.Update());
        REQUIRE(x.second == jled.Hal().Value());
    }

    // expect to start over after changing effect.
    jled.UserFunc(&eval);
    REQUIRE(jled.Update());
    REQUIRE(0 < jled.Hal().Value());
}

TEST_CASE("Pause stops further update of effect", "[jled]") {
    auto eval = MockBrightnessEvaluator(ByteVec{10, 20, 30, 40});
    TestJLed jled = TestJLed(10).UserFunc(&eval);

    jled.Hal().SetMillis(0);
    REQUIRE(true == jled.Update());

    jled.Pause();
    jled.Hal().SetMillis(1);
    REQUIRE(false == jled.Update());
}

TEST_CASE("Resume after Pause continues later where we left off", "[jled]") {
    auto eval = MockBrightnessEvaluator(ByteVec{10, 20, 30});
    TestJLed jled = TestJLed(10).UserFunc(&eval);

    // start at t=1000
    jled.Hal().SetMillis(1000);
    REQUIRE(true == jled.Update());

    // pause at t=1001
    jled.Hal().SetMillis(1001);
    auto state = jled.Pause();

    REQUIRE_FALSE(jled.IsRunning());

    // resume at t=2000, we expect to continue where we left off
    // at t=1001
    jled.Hal().SetMillis(2000);
    jled.Resume(state);

    REQUIRE(true == jled.Update());
    REQUIRE(20 == jled.Hal().Value());

    jled.Hal().SetMillis(2001);
    REQUIRE(false == jled.Update());
    REQUIRE(30 == jled.Hal().Value());
}

TEST_CASE("Max brightness level is initialized to 255 within accuracy",
          "[jled]") {
    // maximum brightness is only stored with kBitsBrightness. Lower bits
    // will always be 0 when the current value is read
    constexpr uint8_t mask = (1 << (8 - TestJLed::kBitsBrightness)) - 1;

    TestJLed jled(10);

    REQUIRE(jled.MaxBrightness() == (255 & ~mask));
}

TEST_CASE("Setting max brightness level can be read back within accuracy",
          "[jled]") {
    constexpr uint8_t mask = (1 << (8 - TestJLed::kBitsBrightness)) - 1;

    TestJLed jled(10);

    jled.MaxBrightness(0);
    REQUIRE(jled.MaxBrightness() == 0);

    jled.MaxBrightness(100);
    REQUIRE(jled.MaxBrightness() == (100 & ~mask));

    jled.MaxBrightness(255);
    REQUIRE(jled.MaxBrightness() == (255 & ~mask));
}

TEST_CASE("Setting max brightness level limits brightness value written to HAL",
          "[jled]") {
    class TestableJLed : public TestJLed {
     public:
        using TestJLed::TestJLed;
        static void test() {
            SECTION(
                "After setting max brightness to 0, always 0 is written to the "
                "HAL",
                "max level is 0") {
                TestableJLed jled(1);

                jled.MaxBrightness(0);

                for (auto b = 0; b <= 255; b++) {
                    jled.Write(b);
                    REQUIRE(0 == jled.Hal().Value());
                }
            }

            SECTION(
                "After setting max brightness to 255, the original value is "
                "written to the HAL",
                "max level is 255") {
                TestableJLed jled(1);

                jled.MaxBrightness(255);

                for (auto b = 0; b <= 255; b++) {
                    jled.Write(b);
                    REQUIRE(b == jled.Hal().Value());
                }
            }

            SECTION(
                "After setting max brightness to 128, the original value is "
                "scaled by 50% when written to the HAL",
                "max level is 128") {
                TestableJLed jled(1);

                jled.MaxBrightness(128);

                for (auto b = 0; b <= 255; b++) {
                    jled.Write(b);
                    REQUIRE(b >> 1 == jled.Hal().Value());
                }
            }
        }
    };
    TestableJLed::test();
}

TEST_CASE("random generator delivers pseudo random numbers", "[rand]") {
    jled::rand_seed(0);
    REQUIRE(0x59 == jled::rand8());
    REQUIRE(0x159 >> 1 == jled::rand8());
}

TEST_CASE("scaling a value with factor 0 scales it to 0", "[scale5]") {
    REQUIRE(0 == jled::scale5(0, 0));
    REQUIRE(0 == jled::scale5(255, 0));
}

TEST_CASE("scaling a value with factor 16 halfes the value", "[scale5]") {
    REQUIRE(0 == jled::scale5(0, 16));
    REQUIRE(50 == jled::scale5(100, 16));
    REQUIRE(127 == jled::scale5(255, 16));
}

TEST_CASE("scaling a value with factor 31 returns original value", "[scale5]") {
    REQUIRE(0 == jled::scale5(0, 31));
    REQUIRE(127 == jled::scale5(127, 31));
    REQUIRE(255 == jled::scale5(255, 31));
}


