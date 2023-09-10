// JLed Unit tests (runs on host)
// Copyright 2017-2022 Jan Delgado jdelgado@gmx.net
#include <jled_base.h>  // NOLINT
#include <iostream>
#include <limits>
#include <map>
#include <utility>
#include <vector>
#include "catch_amalgamated.hpp"
#include "hal_mock.h"  // NOLINT

using jled::BlinkBrightnessEvaluator;
using jled::BreatheBrightnessEvaluator;
using jled::BrightnessEvaluator;
using jled::CandleBrightnessEvaluator;
using jled::ConstantBrightnessEvaluator;
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
        CHECK(t < values_.size());
        count_++;
        return values_[t];
    }
};

// expected result when a JLed object is updated: return value
// of Update() and the current brightness
typedef std::pair<bool, uint8_t> UpdateResult;
typedef std::vector<UpdateResult> UpdateResults;

// helper to check if a led evaluates to given sequence
template <class T>
void check_led(T *led, const UpdateResults &expected) {
    uint32_t time = 0;
    for (const auto &current : expected) {
        led->Hal().SetMillis(time);
        const auto updated = led->Update();
        const auto val = led->Hal().Value();
        UNSCOPED_INFO("t=" << time << ", actual=("
                           << (updated ? "true" : "false") << ", " << (int)val
                           << "), expected=("
                           << (current.first ? "true" : "false") << ", "
                           << (int)current.second << ")");
        CHECK(current.first == updated);
        CHECK(current.second == val);
        time++;
    }
}

// helper to check that a JLed objects evaluates to the given values
#define CHECK_LED(led, expected)                                               \
    do {                                                                       \
        std::cout << "**********************" << std::endl;                    \
        uint32_t time = 0;                                                     \
        for (const auto &current : expected) {                                 \
            std::cout << "t=" << time << ",expected=(" << current.first << "," \
                      << current.second << ")" << std::endl;                   \
            const auto updated = jled.Update();                                \
            CHECK(current.first == jled.Hal().Value());                        \
            CHECK(current.second == updated);                                  \
            jled.Hal().SetMillis(++time);                                      \
        }                                                                      \
    } while (0)

TEST_CASE("jled without effect does nothing", "[jled]") {
    auto led = TestJLed(1);
    CHECK(!led.Update());
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
                CHECK(jled.brightness_eval_->Eval(0) == 255);
            }

            SECTION(
                "using Off() effect uses a BrightnessEval that turns the LED "
                "off") {
                TestableJLed jled(1);
                jled.Off();
                REQUIRE(dynamic_cast<ConstantBrightnessEvaluator *>(
                            jled.brightness_eval_) != nullptr);
                CHECK(jled.brightness_eval_->Eval(0) == 0);
            }

            SECTION("using Set() allows to set custom brightness level") {
                TestableJLed jled(1);
                jled.Set(123);
                REQUIRE(dynamic_cast<ConstantBrightnessEvaluator *>(
                            jled.brightness_eval_) != nullptr);
                CHECK(jled.brightness_eval_->Eval(0) == 123);
            }

            SECTION("using Set(0) allows to set custom turn LED off") {
                TestableJLed jled(1);
                jled.Set(0);
                REQUIRE(dynamic_cast<ConstantBrightnessEvaluator *>(
                            jled.brightness_eval_) != nullptr);
                CHECK(jled.brightness_eval_->Eval(0) == 0);
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
            jled.Breathe(100, 200, 300);
            REQUIRE(dynamic_cast<BreatheBrightnessEvaluator *>(
                        jled.brightness_eval_) != nullptr);
            auto eval = dynamic_cast<BreatheBrightnessEvaluator *>(
                jled.brightness_eval_);
            CHECK(100 == eval->DurationFadeOn());
            CHECK(200 == eval->DurationOn());
            CHECK(300 == eval->DurationFadeOff());
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
            SECTION("FadeOff() initializes with BreatheBrightnessEvaluator") {
                TestableJLed jled(1);
                jled.FadeOff(100);
                REQUIRE(dynamic_cast<BreatheBrightnessEvaluator *>(
                            jled.brightness_eval_) != nullptr);
                auto eval = dynamic_cast<BreatheBrightnessEvaluator *>(
                    jled.brightness_eval_);
                CHECK(0 == eval->DurationFadeOn());
                CHECK(0 == eval->DurationOn());
                CHECK(100 == eval->DurationFadeOff());
            }
            SECTION("FadeOn() initializes with BreatheBrightnessEvaluator") {
                TestableJLed jled(1);
                jled.FadeOn(100);
                REQUIRE(dynamic_cast<BreatheBrightnessEvaluator *>(
                            jled.brightness_eval_) != nullptr);
                auto eval = dynamic_cast<BreatheBrightnessEvaluator *>(
                    jled.brightness_eval_);
                CHECK(100 == eval->DurationFadeOn());
                CHECK(0 == eval->DurationOn());
                CHECK(0 == eval->DurationFadeOff());
            }
        }
    };
    TestableJLed::test();
}

TEST_CASE("using Fade() configures BreatheBrightnessEvaluator", "[jled]") {
    class TestableJLed : public TestJLed {
     public:
        using TestJLed::TestJLed;
        static void test() {
            SECTION("fade with from < to") {
                TestableJLed jled(1);
                jled.Fade(100, 200, 300);
                REQUIRE(dynamic_cast<BreatheBrightnessEvaluator *>(
                            jled.brightness_eval_) != nullptr);
                auto eval = dynamic_cast<BreatheBrightnessEvaluator *>(
                    jled.brightness_eval_);
                CHECK(300 == eval->DurationFadeOn());
                CHECK(0 == eval->DurationOn());
                CHECK(0 == eval->DurationFadeOff());
                CHECK(100 == jled.MinBrightness());
                CHECK(200 == jled.MaxBrightness());
            }
            SECTION("fade with from >= to") {
                TestableJLed jled(1);
                jled.Fade(200, 100, 300);
                REQUIRE(dynamic_cast<BreatheBrightnessEvaluator *>(
                            jled.brightness_eval_) != nullptr);
                auto eval = dynamic_cast<BreatheBrightnessEvaluator *>(
                    jled.brightness_eval_);
                CHECK(0 == eval->DurationFadeOn());
                CHECK(0 == eval->DurationOn());
                CHECK(300 == eval->DurationFadeOff());
                CHECK(100 == jled.MinBrightness());
                CHECK(200 == jled.MaxBrightness());
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
    CHECK(1 == cbZero.Period());
    CHECK(0 == cbZero.Eval(0));
    CHECK(0 == cbZero.Eval(1000));

    auto cbFull = ConstantBrightnessEvaluator(255);
    CHECK(1 == cbFull.Period());
    CHECK(255 == cbFull.Eval(0));
    CHECK(255 == cbFull.Eval(1000));
}

TEST_CASE(
    "BlinkBrightnessEvaluator calculates switches between on and off in given "
    "time frames",
    "[jled]") {
    auto eval = BlinkBrightnessEvaluator(10, 5);
    CHECK(10 + 5 == eval.Period());
    CHECK(255 == eval.Eval(0));
    CHECK(255 == eval.Eval(9));
    CHECK(0 == eval.Eval(10));
    CHECK(0 == eval.Eval(14));
}

TEST_CASE("CandleBrightnessEvaluator simulated candle flickering", "[jled]") {
    auto eval = CandleBrightnessEvaluator(7, 15, 1000);
    CHECK(1000 == eval.Period());
    // TODO(jd) do further and better tests
    CHECK(eval.Eval(0) > 0);
    CHECK(eval.Eval(999) > 0);
}

TEST_CASE(
    "BreatheEvaluator evaluates to bell curve distributed brightness curve",
    "[jled]") {
    auto eval = BreatheBrightnessEvaluator(100, 200, 300);
    CHECK(100 + 200 + 300 == eval.Period());

    const std::map<uint32_t, uint8_t> test_values = {
        {0, 0},     {50, 68},   {80, 198},  {99, 255}, {100, 255},
        {299, 255}, {300, 255}, {399, 138}, {499, 26}, {599, 0}};

    for (const auto &x : test_values) {
        INFO("t=" << x.first);
        CHECK((int)x.second == (int)eval.Eval(x.first));
    }
}

TEST_CASE("Forever flag is initially set to false", "[jled]") {
    TestJLed jled(1);
    CHECK_FALSE(jled.IsForever());
}

TEST_CASE("Forever flag is set by call to Forever()", "[jled]") {
    TestJLed jled(1);
    jled.Forever();
    CHECK(jled.IsForever());
}

TEST_CASE("dont evalute twice during one time tick", "[jled]") {
    auto eval = MockBrightnessEvaluator(ByteVec{0, 1, 2});
    TestJLed jled = TestJLed(1).UserFunc(&eval);

    jled.Hal().SetMillis(0);
    jled.Update();
    CHECK(eval.Count() == 1);
    jled.Update();
    CHECK(eval.Count() == 1);

    jled.Hal().SetMillis(1);
    jled.Update();

    CHECK(eval.Count() == 2);
}

TEST_CASE("Handles millis overflow during effect", "[jled]") {
    TestJLed jled = TestJLed(10);
    // Set time close to overflow
    auto time = std::numeric_limits<uint32_t>::max() - 25;
    jled.Hal().SetMillis(time);
    CHECK_FALSE(jled.Update());
    // Start fade off
    jled.FadeOff(100);
    CHECK(jled.Update());
    CHECK(jled.IsRunning());
    CHECK(jled.Hal().Value() > 0);
    // Set time after overflow, before effect ends
    jled.Hal().SetMillis(time + 50);
    CHECK(jled.Update());
    CHECK(jled.IsRunning());
    CHECK(jled.Hal().Value() > 0);
    // Set time after effect ends
    jled.Hal().SetMillis(time + 150);
    CHECK_FALSE(jled.Update());
    CHECK_FALSE(jled.IsRunning());
    CHECK(0 == jled.Hal().Value());
}

TEST_CASE("Stop() stops the effect", "[jled]") {
    auto eval = MockBrightnessEvaluator(ByteVec{255, 255, 255, 0});
    TestJLed jled = TestJLed(10).UserFunc(&eval);

    REQUIRE(jled.IsRunning());
    jled.Update();
    jled.Stop();

    CHECK(!jled.IsRunning());
}

TEST_CASE("default Stop() sets the brightness to minBrightness", "[jled]") {
    auto eval = MockBrightnessEvaluator(ByteVec{100, 0});
    TestJLed jled = TestJLed(10).UserFunc(&eval).MinBrightness(50);

    jled.Update();
    REQUIRE(130 == jled.Hal().Value());  // 100 scaled to [50,255]
    jled.Stop();

    CHECK(50 == jled.Hal().Value());
}

TEST_CASE("Stop(FULL_OFF) sets the brightness to 0", "[jled]") {
    auto eval = MockBrightnessEvaluator(ByteVec{100, 0});
    TestJLed jled = TestJLed(10).UserFunc(&eval).MinBrightness(50);

    jled.Update();
    REQUIRE(130 == jled.Hal().Value());  // 100 scaled to [50,255]
    jled.Stop(TestJLed::eStopMode::FULL_OFF);

    CHECK(0 == jled.Hal().Value());
}

TEST_CASE("Stop(KEEP_CURRENT) keeps the last brightness level", "[jled]") {
    auto eval = MockBrightnessEvaluator(ByteVec{100, 101});
    TestJLed jled = TestJLed(10).UserFunc(&eval).MinBrightness(50);

    jled.Update();
    REQUIRE(130 == jled.Hal().Value());  // 100 scaled to [50,255]
    jled.Stop(TestJLed::eStopMode::KEEP_CURRENT);

    CHECK(130 == jled.Hal().Value());
}

TEST_CASE("LowActive() inverts signal", "[jled]") {
    auto eval = MockBrightnessEvaluator(ByteVec{0, 255});
    TestJLed jled = TestJLed(1).UserFunc(&eval).LowActive();

    CHECK(jled.IsLowActive());

    jled.Update();
    CHECK(255 == jled.Hal().Value());

    jled.Hal().SetMillis(1);
    jled.Update();
    CHECK(0 == jled.Hal().Value());
}

TEST_CASE("effect with repeat 2 repeats sequence once", "[jled]") {
    auto eval = MockBrightnessEvaluator(ByteVec{10, 20});
    TestJLed jled = TestJLed(10).UserFunc(&eval).Repeat(2);

    typedef UpdateResult u;
    const UpdateResults expected = {u{true, 10},  u{true, 20},  u{true, 10},
                                    u{false, 20}, u{false, 20}, u{false, 20}};

    check_led(&jled, expected);
}

TEST_CASE("effect with delay after delays start of next iteration", "[jled]") {
    auto eval = MockBrightnessEvaluator(ByteVec{10, 20});
    TestJLed jled = TestJLed(10).UserFunc(&eval).Repeat(2).DelayAfter(2);

    typedef UpdateResult u;
    const UpdateResults expected = {
        u{true, 10}, u{true, 20}, u{true, 20},  u{true, 20},  u{true, 10},
        u{true, 20}, u{true, 20}, u{false, 20}, u{false, 20}, u{false, 20}};

    check_led(&jled, expected);
}

TEST_CASE("effect with delay before has delayed start ", "[jled]") {
    auto eval = MockBrightnessEvaluator(ByteVec{10, 20});
    TestJLed jled = TestJLed(10).UserFunc(&eval).DelayBefore(2);

    typedef UpdateResult u;
    const UpdateResults expected = {u{true, 0},   u{true, 0},   u{true, 10},
                                    u{false, 20}, u{false, 20}, u{false, 20}};

    check_led(&jled, expected);
}

TEST_CASE("After calling Forever() the effect is repeated over and over again ",
          "[jled]") {
    auto eval = MockBrightnessEvaluator(ByteVec{10, 20});
    TestJLed jled = TestJLed(10).UserFunc(&eval).Forever();

    typedef UpdateResult u;
    const UpdateResults expected = {u{true, 10}, u{true, 20}, u{true, 10},
                                    u{true, 20}, u{true, 10}, u{true, 20}};

    check_led(&jled, expected);
}

TEST_CASE("The Hal object provided in the ctor is used during update",
          "[jled]") {
    auto eval = MockBrightnessEvaluator(ByteVec{10, 20});
    TestJLed jled = TestJLed(HalMock(123)).UserFunc(&eval);

    CHECK(jled.Hal().Pin() == 123);
}

TEST_CASE("Update returns true while updating, else false", "[jled]") {
    auto eval = MockBrightnessEvaluator(ByteVec{10, 20});
    TestJLed jled = TestJLed(10).UserFunc(&eval);

    // Update returns FALSE on last step and beyond, else TRUE
    auto time = 0;
    jled.Hal().SetMillis(time++);
    CHECK(jled.Update());

    // when effect is done, we expect still false to be returned
    jled.Hal().SetMillis(time++);
    CHECK_FALSE(jled.Update());
    jled.Hal().SetMillis(time++);
    CHECK_FALSE(jled.Update());
}

TEST_CASE("After Reset() the effect can be restarted", "[jled]") {
    auto eval = MockBrightnessEvaluator(ByteVec{10, 20});
    TestJLed jled = TestJLed(10).UserFunc(&eval);

    typedef UpdateResult u;

    const UpdateResults expected = {u{true, 10}, u{false, 20}, u{false, 20},
                                    u{false, 20}};

    check_led(&jled, expected);

    // after Reset() effect starts over
    jled.Reset();

    check_led(&jled, expected);
}

TEST_CASE("Changing the effect resets object and starts over", "[jled]") {
    auto eval = MockBrightnessEvaluator(ByteVec{10, 20});
    TestJLed jled = TestJLed(10).UserFunc(&eval);

    typedef UpdateResult u;
    const UpdateResults expected = {u{true, 10}, u{false, 20}, u{false, 20}};

    check_led(&jled, expected);

    // expect to start over after changing effect.
    jled.UserFunc(&eval);
    check_led(&jled, expected);
}

TEST_CASE("Max brightness level is initialized to 255", "[jled]") {
    TestJLed jled(10);
    CHECK(255 == jled.MaxBrightness());
}

TEST_CASE("Previously max brightness level can be read back", "[jled]") {
    TestJLed jled(10);
    jled.MaxBrightness(100);
    CHECK(100 == jled.MaxBrightness());
}

TEST_CASE("Min brightness level is initialized to 0", "[jled]") {
    TestJLed jled(10);
    CHECK(0 == jled.MinBrightness());
}

TEST_CASE("Previously set min brightness level can be read back", "[jled]") {
    TestJLed jled(10);

    jled.MinBrightness(100);
    CHECK(100 == jled.MinBrightness());
}

TEST_CASE(
    "Setting min and max brightness levels scales evaluated effect values",
    "[jled]") {
    class TestableJLed : public TestJLed {
     public:
        using TestJLed::TestJLed;
        static void test() {
            TestableJLed jled(1);

            auto eval = MockBrightnessEvaluator(ByteVec{0, 128, 255});
            jled.UserFunc(&eval).MinBrightness(100).MaxBrightness(200);

            CHECK(100 == jled.Eval(0));
            CHECK(150 == jled.Eval(1));
            CHECK(200 == jled.Eval(2));
        }
    };
    TestableJLed::test();
};

TEST_CASE("timeChangeSinceLastUpdate detects time changes", "[jled]") {
    class TestableJLed : public TestJLed {
     public:
        using TestJLed::TestJLed;
        static void test() {
            TestableJLed jled(1);

            jled.trackLastUpdateTime(1000);
            CHECK_FALSE(jled.timeChangedSinceLastUpdate(1000));
            CHECK(jled.timeChangedSinceLastUpdate(1001));
        }
    };
    TestableJLed::test();
}

TEST_CASE("random generator delivers pseudo random numbers", "[rand]") {
    jled::rand_seed(0);
    CHECK(0x59 == jled::rand8());
    CHECK(0x159 >> 1 == jled::rand8());
}

TEST_CASE("scaling a value with factor 0 scales it to 0", "[scale8]") {
    CHECK(0 == jled::scale8(0, 0));
    CHECK(0 == jled::scale8(255, 0));
}

TEST_CASE("scaling a value with factor 127 halfes the value", "[scale8]") {
    CHECK(0 == jled::scale8(0, 128));
    CHECK(50 == jled::scale8(100, 128));
    CHECK(128 == jled::scale8(255, 128));
}

TEST_CASE("scaling a value with factor 255 returns original value",
          "[scale8]") {
    CHECK(0 == jled::scale8(0, 255));
    CHECK(127 == jled::scale8(127, 255));
    CHECK(255 == jled::scale8(255, 255));
}

TEST_CASE("lerp8by8 interpolates a byte into the given interval",
          "[lerp8by8]") {
    CHECK(0 == (int)(jled::lerp8by8(0, 0, 255)));
    CHECK(0 == (int)(jled::lerp8by8(255, 0, 0)));
    CHECK(255 == (int)(jled::lerp8by8(255, 0, 255)));

    CHECK(100 == (int)(jled::lerp8by8(0, 100, 255)));
    CHECK(100 == (int)(jled::lerp8by8(0, 100, 110)));

    CHECK(255 == (int)(jled::lerp8by8(255, 100, 255)));
    CHECK(200 == (int)(jled::lerp8by8(255, 100, 200)));
}
