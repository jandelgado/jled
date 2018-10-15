// JLed Unit tests (run on host), using the ArduinoAnalogWriter and
// an Arduino mock for testing.
// Copyright 2017 Jan Delgado jdelgado@gmx.net
#include <map>

#include "catch.hpp"
#include <jled.h>  // NOLINT

using namespace jled;

TEST_CASE("jled ctor set pin mode to OUTPUT", "[jled]") {
    constexpr auto kTestPin = 10;
    REQUIRE(arduinoMockGetPinMode(kTestPin) == 0);
    JLed jled(kTestPin);
    REQUIRE(arduinoMockGetPinMode(kTestPin) == OUTPUT);
}

// TEST_CASE("On/Off function configuration", "[jled]") {
//     class TestableJLed : public JLed {
//      public:
//         using JLed::JLed;
//         static void test() {
//             SECTION("On()") {
//                 TestableJLed jled(1);
//                 jled.On();
//                 REQUIRE(jled.brightness_func_ == &JLed::OnFunc);
//             }

//             SECTION("Off()") {
//                 TestableJLed jled(1);
//                 jled.Off();
//                 REQUIRE(jled.brightness_func_ == &JLed::OffFunc);
//             }

//             SECTION("Set(true)") {
//                 TestableJLed jled(1);
//                 jled.Set(true);
//                 REQUIRE(jled.brightness_func_ == &JLed::OnFunc);
//             }

//             SECTION("Set(false)") {
//                 TestableJLed jled(1);
//                 jled.Set(false);
//                 REQUIRE(jled.brightness_func_ == &JLed::OffFunc);
//             }
//         }
//     };
//     TestableJLed::test();
// }

// TEST_CASE("Breathe() function configuration", "[jled]") {
//     class TestableJLed : public JLed {
//      public:
//         using JLed::JLed;
//         static void test() {
//             TestableJLed jled(1);
//             jled.Breathe(0);
//             REQUIRE(jled.brightness_func_ == &JLed::BreatheFunc);
//         }
//     };
//     TestableJLed::test();
// }

// TEST_CASE("FadeOn()/FadeOff() function configuration", "[jled]") {
//     class TestableJLed : public JLed {
//      public:
//         using JLed::JLed;
//         static void testFadeOff() {
//             TestableJLed jled(1);
//             jled.FadeOff(0);
//             REQUIRE(jled.brightness_func_ == &JLed::FadeOffFunc);
//         }
//         static void testFadeOn() {
//             TestableJLed jled(1);
//             jled.FadeOn(0);
//             REQUIRE(jled.brightness_func_ == &JLed::FadeOnFunc);
//         }
//     };
//     TestableJLed::testFadeOn();
//     TestableJLed::testFadeOff();
// }

TEST_CASE("ConstantBrightnessEvaluator calculates correct values", "[jled]") {
    auto cbZero = ConstantBrightnessEvaluator(0);
    REQUIRE(1 == cbZero.Period());
    REQUIRE(0 == cbZero.Eval(0));
    REQUIRE(0 == cbZero.Eval(1000));

    auto cbFull = ConstantBrightnessEvaluator(255);
    REQUIRE(1 == cbFull.Period());
    REQUIRE(255 == cbFull.Eval(0));
    REQUIRE(255 == cbFull.Eval(1000));
}

TEST_CASE("BlinkBrightnessEvaluator calculates correct values", "[jled]") {
    auto eval = BlinkBrightnessEvaluator(10, 5);
    REQUIRE(10+5 == eval.Period());
    REQUIRE(255 == eval.Eval(0));
    REQUIRE(255 == eval.Eval(9));
    REQUIRE(0 == eval.Eval(10));
    REQUIRE(0 == eval.Eval(14));
}

TEST_CASE("FadeOnOffEvaluator calculates correct values", "[jled]") {
    constexpr auto kPeriod = 2000;
    // since FadeOffFunc is just mirrored FadeOffFunc inverted
    // we test both together.
    auto evalOn = FadeOnBrightnessEvaluator(kPeriod);
    auto evalOff = FadeOffBrightnessEvaluator(kPeriod);

    REQUIRE(kPeriod == evalOn.Period());
    REQUIRE(kPeriod == evalOff.Period());
    const std::map<uint32_t, uint8_t> test_values = {
        {0, 0},      {500, 13},   {1000, 68},  {1500, 179},
        {1999, 255}, {2000, 255}, {10000, 255}};

    for (auto &x : test_values) {
        REQUIRE(x.second == evalOn.Eval(x.first));
        REQUIRE(x.second == evalOff.Eval(kPeriod - x.first));
    }
}

TEST_CASE("BreatheEvaluator calculates correct values", "[jled]") {
    constexpr auto kPeriod = 2000;
    auto eval = BreatheBrightnessEvaluator(kPeriod);
    REQUIRE(kPeriod == eval.Period());
    const std::map<uint32_t, uint8_t> test_values = {
        {0, 0}, {500, 68}, {1000, 255}, {1500, 68}, {1999, 0}, {2000, 0}};

    for (const auto &x : test_values) {
        REQUIRE((int)x.second == (int)eval.Eval(x.first));
    }
}

TEST_CASE("EvalBrightness()", "[jled]") {
    static constexpr auto kTimeProbe = 100;
    static constexpr auto kPeriod = 1000;
    static constexpr auto kBrightness = 100;
    static constexpr auto kBrightnessLast = 10;

    class CustomBrightnessEvaluator : public BrightnessEvaluator {
        uint16_t period_;

     public:
        CustomBrightnessEvaluator(uint16_t period) : period_(period) {}
        uint16_t Period() const { return period_; }
        uint8_t Eval(uint32_t t) {
            // we test that our custom brightness evaluator is called
            // properly and return the expected values.
            REQUIRE(t == kTimeProbe);
            return t + 1 == period_ ? kBrightnessLast : kBrightness;
        }
    };

    class TestableJLed : public JLed {
        using JLed::JLed;

     public:
        static void test() {
            SECTION("standard evaluation") {
                TestableJLed jled(1);
                auto eval = CustomBrightnessEvaluator(kPeriod);
                jled.UserFunc(&eval);
                REQUIRE(jled.EvalBrightness(kTimeProbe) == kBrightness);
            }

            SECTION("inverted evaluation") {
                TestableJLed jled(1);
                auto eval = CustomBrightnessEvaluator(kPeriod);
                jled.UserFunc(&eval).Invert();
                REQUIRE(jled.EvalBrightness(kTimeProbe) == 255 - kBrightness);
            }
        }
    };
    TestableJLed::test();
}

TEST_CASE("set and test forever", "[jled]") {
    JLed jled(1);
    REQUIRE_FALSE(jled.IsForever());
    jled.Forever();
    REQUIRE(jled.IsForever());
}

TEST_CASE("dont evalute twice during one time tick", "[jled]") {
    class CountingCustomBrightnessEvaluator : public BrightnessEvaluator {
        uint16_t count_ = 0;

     public:
        uint16_t Period() const { return 1000; }
        uint16_t Count() const { return count_; }
        uint8_t Eval(uint32_t t) {
            count_++;
            return 0;
        }
    };

    uint16_t num_times_called = 0;
    auto eval = CountingCustomBrightnessEvaluator();
    JLed jled = JLed(1).UserFunc(&eval);
    arduinoMockSetMillis(0);

    jled.Update();
    REQUIRE(eval.Count() == 1);
    jled.Update();
    REQUIRE(eval.Count() == 1);

    arduinoMockSetMillis(1);
    jled.Update();
    REQUIRE(eval.Count() == 2);
}

TEST_CASE("stop effect stops the effect", "[jled]") {
    constexpr auto kTestPin = 10;
    constexpr auto kDuration = 100;
    arduinoMockInit();

    // we test that an effect that normally has high ouput for a longer
    // time (e.g. FadeOff()) stays off after Stop() was called.
    JLed jled = JLed(kTestPin).FadeOff(kDuration);
    REQUIRE(!jled.IsStopped());
    jled.Update();
    REQUIRE(arduinoMockGetPinState(kTestPin) > 0);
    jled.Stop();
    REQUIRE(jled.IsStopped());
    REQUIRE_FALSE(jled.Update());
    REQUIRE(arduinoMockGetPinState(kTestPin) == 0);
    // update should not change anything
    REQUIRE_FALSE(jled.Update());
    REQUIRE(arduinoMockGetPinState(kTestPin) == 0);
}

TEST_CASE("LowActive() inverts signal", "[jled]") {
    constexpr auto kTestPin = 10;
    arduinoMockInit();

    JLed jled = JLed(kTestPin).On().LowActive();
    REQUIRE(arduinoMockGetPinState(kTestPin) == 0);
    jled.Update();
    REQUIRE(arduinoMockGetPinState(kTestPin) == 0);
    jled.Stop();
    REQUIRE(arduinoMockGetPinState(kTestPin) == 255);
}

TEST_CASE("blink led twice with delay and repeat", "[jled]") {
    constexpr auto kTestPin = 10;
    arduinoMockInit();
    JLed jled(kTestPin);

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
        REQUIRE(arduinoMockGetPinState(kTestPin) == val);
        arduinoMockSetMillis(++time);
    }
}

TEST_CASE("blink led forever", "[jled]") {
    constexpr auto kTestPin = 10;
    arduinoMockInit();
    JLed jled(kTestPin);

    SECTION("blink led forever") {
        constexpr auto kOnDuration = 5;
        constexpr auto kOffDuration = 10;
        constexpr auto kRepetitions = 5;  // test this number of times
        uint32_t time = 0;

        REQUIRE_FALSE(jled.IsForever());
        jled.Blink(kOnDuration, kOffDuration).Forever();
        REQUIRE(jled.IsForever());

        auto timer = 0;
        for (auto i = 0; i < kRepetitions; i++) {
            jled.Update();
            auto state = (timer < kOnDuration);
            REQUIRE(arduinoMockGetPinState(kTestPin) == (state ? 255 : 0));
            timer++;
            if (timer >= kOnDuration + kOffDuration) {
                timer = 0;
            }
            arduinoMockSetMillis(++time);
        }
    }
}

TEST_CASE("construct Jled object with custom ctor", "[jled]") {
    arduinoMockInit();

    constexpr auto kTestPin = 1;
    JLed jled = JLed(ArduinoAnalogWriter(kTestPin)).Blink(1, 1);

    // test with a simple on-off sequence
    uint32_t time = 0;
    REQUIRE(jled.Update());
    REQUIRE(arduinoMockGetPinState(kTestPin) == 255);
    arduinoMockSetMillis(++time);
    REQUIRE(!jled.Update());
    REQUIRE(arduinoMockGetPinState(kTestPin) == 0);
    arduinoMockSetMillis(++time);
}

TEST_CASE("Update returns true while updating, else false", "[jled]") {
    arduinoMockInit();
    JLed jled = JLed(10).Blink(2, 3);
    constexpr auto expectedTime = 2 + 3;

    uint32_t time = 0;
    for (auto i = 0; i < expectedTime - 1; i++) {
        // returns FALSE on last step and beyond, else TRUE
        arduinoMockSetMillis(time++);
        auto res = jled.Update();
        REQUIRE(res);
    }
    // when effect is done, we expect still false to be returned
    arduinoMockSetMillis(time++);
    REQUIRE_FALSE(jled.Update());
}

// TEST_CASE("user provided brightness function", "[jled]") {
//     constexpr auto kTestPin = 10;
//     constexpr auto kDuration = 5;
//     constexpr auto kBrightness = 99;

//     arduinoMockInit();

//     // user func returns sequence (0, 1, 2, 3, 99) for t >= 0
//     auto user_func = [](uint32_t t, uint16_t period,
//                         uintptr_t param) -> uint8_t {
//         return t < kDuration - 1 ? static_cast<uint8_t>(t) : kBrightness;
//     };

//     JLed jled = JLed(kTestPin).UserFunc(user_func, kDuration, 0);

//     constexpr uint8_t expected[]{0, 1, 2, 3, kBrightness};
//     auto time = 0;
//     for (const auto val : expected) {
//         jled.Update();
//         REQUIRE(arduinoMockGetPinState(kTestPin) == val);
//         arduinoMockSetMillis(++time);
//     }
//     jled.Update();
// }
