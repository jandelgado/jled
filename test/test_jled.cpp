// JLed Unit tests (runs on host)
// Copyright 2017 Jan Delgado jdelgado@gmx.net
#include <jled_base.h>  // NOLINT
#include <map>
#include <utility>
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
    class CustomBrightnessEvaluator : public BrightnessEvaluator {
     public:
        uint16_t Period() const { return 0; }
        uint8_t Eval(uint32_t) const { return 0; }
    };

    class TestableJLed : public TestJLed {
     public:
        using TestJLed::TestJLed;
        static void test() {
            TestableJLed jled(1);
            auto cust = CustomBrightnessEvaluator();
            jled.UserFunc(&cust);
            REQUIRE(dynamic_cast<CustomBrightnessEvaluator *>(
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
    "BlinkBrightnessEvaluator calculates switches betwen on and off in given "
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
    constexpr auto kPeriod = 2000;
    auto eval = BreatheBrightnessEvaluator(kPeriod);
    REQUIRE(kPeriod == eval.Period());
    const std::map<uint32_t, uint8_t> test_values = {
        {0, 0}, {500, 68}, {1000, 255}, {1500, 68}, {1999, 0}, {2000, 0}};

    for (const auto &x : test_values) {
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
    class CountingCustomBrightnessEvaluator : public BrightnessEvaluator {
        mutable uint16_t count_ = 0;

     public:
        BrightnessEvaluator *clone(void *p) const {
            return new (p) CountingCustomBrightnessEvaluator(*this);
        }
        uint16_t Period() const { return 1000; }
        uint16_t Count() const { return count_; }
        uint8_t Eval(uint32_t) const {
            count_++;
            return 0;
        }
    };

    auto eval = CountingCustomBrightnessEvaluator();
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

TEST_CASE("Stop() stops the effect", "[jled]") {
    constexpr auto kDuration = 100;

    // we test that an effect that normally has high ouput for a longer
    // time (e.g. FadeOff()) stays off after Stop() was called.
    TestJLed jled = TestJLed(10).FadeOff(kDuration);
    REQUIRE(jled.IsRunning());
    jled.Update();
    REQUIRE(jled.Hal().Value() > 0);
    jled.Stop();
    REQUIRE(!jled.IsRunning());
    REQUIRE_FALSE(jled.Update());
    REQUIRE(0 == jled.Hal().Value());
    // update should not change anything
    REQUIRE_FALSE(jled.Update());
    REQUIRE(0 == jled.Hal().Value());
}

TEST_CASE("LowActive() inverts signal", "[jled]") {
    TestJLed jled = TestJLed(10).On().LowActive();

    REQUIRE(jled.IsLowActive());
    REQUIRE(0 == jled.Hal().Value());
    jled.Update();
    REQUIRE(0 == jled.Hal().Value());
    jled.Stop();
    REQUIRE(255 == jled.Hal().Value());
}

TEST_CASE("blink led twice with delay and repeat", "[jled]") {
    TestJLed jled(10);

    // 1 ms on, 2 ms off + 2 ms delay = 3ms off in total per iteration
    jled.DelayBefore(5).Blink(1, 2).DelayAfter(2).Repeat(2);
    constexpr uint8_t expected[]{
        /* delay before 5ms */ 0, 0, 0, 0, 0,
        /* 1ms on */ 255,
        /* 2ms off */ 0,          0,
        /* 2ms delay */ 0,        0,
        /* repeat */ 255,         0, 0, 0, 0,
        /* finally stay off */ 0, 0};
    uint32_t time = 0;
    for (const auto val : expected) {
        jled.Update();
        REQUIRE(val == jled.Hal().Value());
        jled.Hal().SetMillis(++time);
    }
}

TEST_CASE("After calling Forever() the effect is repeated over and over again ",
          "[jled]") {
    constexpr auto kOnDuration = 5;
    constexpr auto kOffDuration = 10;
    constexpr auto kPeriod = kOnDuration + kOffDuration;
    constexpr auto kRepetitions = 50;  // test this number of times

    TestJLed jled(10);
    jled.Blink(kOnDuration, kOffDuration).Forever();

    uint32_t time = 0;
    for (auto i = 0; i < kRepetitions; i++) {
        jled.Update();
        const bool is_on = ((time % kPeriod) < kOnDuration);
        const auto expected = (is_on ? 255 : 0);
        REQUIRE(expected == jled.Hal().Value());
        // time++;
        // if (time >= kOnDuration + kOffDuration) {
        //     time = 0;
        // }
        jled.Hal().SetMillis(++time);
    }
}

TEST_CASE("The Hal object provided in the ctor is used during update",
          "[jled]") {
    TestJLed jled = TestJLed(HalMock(10)).Blink(1, 1);

    // test with a simple on-off sequence
    uint32_t time = 0;

    jled.Hal().SetMillis(time);
    REQUIRE(jled.Update());
    REQUIRE(255 == jled.Hal().Value());

    jled.Hal().SetMillis(++time);
    REQUIRE(!jled.Update());
    REQUIRE(0 == jled.Hal().Value());
}

TEST_CASE("Update returns true while updating, else false", "[jled]") {
    TestJLed jled = TestJLed(10).Blink(2, 3);
    constexpr auto expectedTime = 2 + 3;

    uint32_t time = 0;
    for (auto i = 0; i < expectedTime - 1; i++) {
        // returns FALSE on last step and beyond, else TRUE
        jled.Hal().SetMillis(time++);
        REQUIRE(jled.Update());
    }
    // when effect is done, we expect still false to be returned
    jled.Hal().SetMillis(time++);
    REQUIRE_FALSE(jled.Update());
}

TEST_CASE("After Reset() the effect can be restarted", "[jled]") {
    TestJLed jled(10);
    uint32_t time = 0;
    typedef std::pair<bool, uint8_t> p;

    // 1 ms on, 2 ms off + 2 ms delay = 3ms off in total per iteration
    jled.Blink(1, 2);
    constexpr p expected[]{p{true, 255}, p{true, 0}, p{false, 0}, p{false, 0}};

    for (const auto x : expected) {
        jled.Hal().SetMillis(time++);
        REQUIRE(x.first == jled.Update());
        REQUIRE(x.second == jled.Hal().Value());
    }

    // after Reset() effect starts over
    jled.Reset();
    for (const auto x : expected) {
        jled.Hal().SetMillis(time++);
        REQUIRE(x.first == jled.Update());
        REQUIRE(x.second == jled.Hal().Value());
    }
}

TEST_CASE("Changing the effect resets object and starts over", "[jled]") {
    TestJLed jled(10);
    uint32_t time = 0;
    typedef std::pair<bool, uint8_t> p;

    // 1 ms on, 2 ms off + 2 ms delay = 3ms off in total per iteration
    jled.Blink(1, 2);
    constexpr p expected_blink[]{p{true, 255}, p{true, 0}, p{false, 0},
                                 p{false, 0}};

    for (const auto x : expected_blink) {
        jled.Hal().SetMillis(time++);
        REQUIRE(x.first == jled.Update());
        REQUIRE(x.second == jled.Hal().Value());
    }

    // expect to start over after changing effect.
    jled.FadeOff(1000);
    REQUIRE(jled.Update());
    REQUIRE(0 < jled.Hal().Value());
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
