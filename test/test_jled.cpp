// JLed Unit tests  (run on host).
// Copyright 2017 Jan Delgado jdelgado@gmx.net
#include <map>
#include <vector>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <jled.h>  // NOLINT

TEST_CASE("arduino mock init", "[mock]") {
  arduinoMockInit();
  for (auto i = 0; i < ARDUINO_PINS; i++) {
    REQUIRE(arduinoMockGetPinMode(i) == 0);
    REQUIRE(arduinoMockGetPinState(i) == 0);
  }
  REQUIRE(millis() == 0);
}

TEST_CASE("arduino mock set time", "[mock]") {
  arduinoMockInit();
  REQUIRE(millis() == 0);
  arduinoMockSetMillis(6502);
  REQUIRE(millis() == 6502);
}

TEST_CASE("arduino mock analog write", "[mock]") {
  constexpr auto kTestPin = 10;
  arduinoMockInit();
  analogWrite(kTestPin, 99);
  REQUIRE(arduinoMockGetPinState(kTestPin) == 99);
}

TEST_CASE("jled ctor set pin mode to OUTPUT", "[jled]") {
  constexpr auto kTestPin = 10;
  REQUIRE(arduinoMockGetPinMode(kTestPin) == 0);
  JLed jled(kTestPin);
  REQUIRE(arduinoMockGetPinMode(kTestPin) == OUTPUT);
}

TEST_CASE("properly initialize brightness_func", "[jled]") {
  class TestableJLed : public JLed {
   public:
    using JLed::JLed;
    static void test() {
      TestableJLed jled = TestableJLed(1);
      REQUIRE(jled.brightness_func_ == nullptr);
    }
  };
  TestableJLed::test();
}

TEST_CASE("On/Off function configuration", "[jled]") {
  class TestableJLed : public JLed {
   public:
    using JLed::JLed;
    static void test() {
      SECTION("On()") {
        TestableJLed jled(1);
        jled.On();
        REQUIRE(jled.brightness_func_ == &JLed::OnFunc);
      }

      SECTION("Off()") {
        TestableJLed jled(1);
        jled.Off();
        REQUIRE(jled.brightness_func_ == &JLed::OffFunc);
      }

      SECTION("Set(true)") {
        TestableJLed jled(1);
        jled.Set(true);
        REQUIRE(jled.brightness_func_ == &JLed::OnFunc);
      }

      SECTION("Set(false)") {
        TestableJLed jled(1);
        jled.Set(false);
        REQUIRE(jled.brightness_func_ == &JLed::OffFunc);
      }
    }
  };
  TestableJLed::test();
}

TEST_CASE("Breathe() function configuration", "[jled]") {
  class TestableJLed : public JLed {
   public:
    using JLed::JLed;
    static void test() {
      TestableJLed jled(1);
      jled.Breathe(0);
      REQUIRE(jled.brightness_func_ == &JLed::BreatheFunc);
    }
  };
  TestableJLed::test();
}

TEST_CASE("FadeOn()/FadeOff() function configuration", "[jled]") {
  class TestableJLed : public JLed {
   public:
    using JLed::JLed;
    static void testFadeOff() {
      TestableJLed jled(1);
      jled.FadeOff(0);
      REQUIRE(jled.brightness_func_ == &JLed::FadeOffFunc);
    }
    static void testFadeOn() {
      TestableJLed jled(1);
      jled.FadeOn(0);
      REQUIRE(jled.brightness_func_ == &JLed::FadeOnFunc);
    }
  };
  TestableJLed::testFadeOn();
  TestableJLed::testFadeOff();
}

TEST_CASE("brightness functions calculate correct values", "[jled]") {
  class TestableJLed : JLed {
   public:
    static void test() {
      SECTION("OffFunc()") {
        // OffFunc returns always 0
        REQUIRE(JLed::OffFunc(0, 0, 0) == 0);
        REQUIRE(JLed::OffFunc((uint32_t)(-1), 0, 0) == 0);
      }

      SECTION("OnFunc()") {
        // OnFunc returns always 255
        REQUIRE(JLed::OnFunc(0, 0, 0) == 255);
        REQUIRE(JLed::OnFunc((uint32_t)(-1), 0, 0) == 255);
      }

      SECTION("BreatheFunc()") {
        constexpr auto kPeriod = 2000;
        const std::map<uint32_t, uint8_t> test_values = {
            {0, 0}, {500, 68}, {1000, 255}, {1500, 68}, {1999, 0}, {2000, 0}};
        for (auto &x : test_values) {
          auto val = JLed::BreatheFunc(x.first, kPeriod, 0);
          REQUIRE((int)x.second == (int)val);
        }
      }

      SECTION("BlinkFunc()") {
        constexpr auto kPeriod = 2000;
        constexpr auto kDurationOn = 500;
        REQUIRE(JLed::BlinkFunc(0, kPeriod, kDurationOn) == 255);
        REQUIRE(JLed::BlinkFunc(kDurationOn - 1, kPeriod, kDurationOn) == 255);
        REQUIRE(JLed::BlinkFunc(kDurationOn, kPeriod, kDurationOn) == 0);
        REQUIRE(JLed::BlinkFunc((uint32_t)(-1), kPeriod, kDurationOn) == 0);
      }

      SECTION("FadeOnFunc() and FadeOffFunc()") {
        // since FadeOffFunc is just mirrored FadeOffFunc inverted, we
        // test both together.
        constexpr auto kPeriod = 2000;
        const std::map<uint32_t, uint8_t> test_values = {
            {0, 0},      {500, 13},   {1000, 68},  {1500, 179},
            {1999, 255}, {2000, 255}, {10000, 255}};
        for (auto &x : test_values) {
          auto valFadeOn = JLed::FadeOnFunc(x.first, kPeriod, 0);
          auto valFadeOff = JLed::FadeOffFunc(kPeriod - x.first, kPeriod, 0);
          REQUIRE(x.second == valFadeOn);
          REQUIRE(x.second == valFadeOff);
        }
      }
    }
  };
  TestableJLed::test();
}

TEST_CASE("EvalBrightness()", "[jled]") {
  class TestableJLed : public JLed {
    using JLed::JLed;

   public:
    static void test() {
      constexpr auto kTimeProbe = 100;
      constexpr auto kPeriod = 1000;
      constexpr auto kUserParam = 1337;
      constexpr auto kBrightness = 100;
      constexpr auto kBrightnessLast = 10;

      // we test that our custom brightness Function is called properly.
      auto func = [](uint32_t t, uint16_t period,
                     uintptr_t userParam) -> uint8_t {
        REQUIRE(t == kTimeProbe);
        REQUIRE(period == kPeriod);
        REQUIRE(userParam == kUserParam);
        return t + 1 == period ? kBrightnessLast : kBrightness;
      };

      SECTION("standard evaluation") {
        TestableJLed jled(1);
        jled.UserFunc(func, kPeriod, kUserParam);
        REQUIRE(jled.EvalBrightness(kTimeProbe) == kBrightness);
      }

      SECTION("inverted evaluation") {
        TestableJLed jled(1);
        jled.UserFunc(func, kPeriod, kUserParam).Invert();
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
  static auto num_times_called = 0;

  auto func = [](uint32_t, uint16_t, uintptr_t) -> uint8_t {
    num_times_called++;
    return 0;
  };

  JLed jled(1);
  jled.UserFunc(func, 1000, 0);
  num_times_called = 0;
  arduinoMockSetMillis(0);

  jled.Update();
  REQUIRE(num_times_called == 1);
  jled.Update();
  REQUIRE(num_times_called == 1);

  arduinoMockSetMillis(1);
  jled.Update();
  REQUIRE(num_times_called == 2);
}

TEST_CASE("stop effect", "[jled]") {
  constexpr auto kTestPin = 10;
  constexpr auto kDuration = 100;
  arduinoMockInit();

  // we test that an effect that normally has high ouput for a longer
  // time (e.g. FadeOff()) stays off after Stop() was called
  JLed jled = JLed(kTestPin).FadeOff(kDuration);

  jled.Update();
  REQUIRE(arduinoMockGetPinState(kTestPin) > 0);
  jled.Stop();
  REQUIRE(arduinoMockGetPinState(kTestPin) == 0);
  // update should not change anything
  jled.Update();
  REQUIRE(arduinoMockGetPinState(kTestPin) == 0);
}

TEST_CASE("LowActive() inverts signal", "[jled]") {
  constexpr auto kTestPin = 10;
  arduinoMockInit();

  // we test that an effect that normally has high ouput for a longer
  // time (e.g. FadeOff()) stays off after Stop() was called
  JLed jled = JLed(kTestPin).On().LowActive();
  REQUIRE(arduinoMockGetPinState(kTestPin) == 0);
  jled.Update();
  REQUIRE(arduinoMockGetPinState(kTestPin) == 0);
  jled.Off();
  jled.Update();
  REQUIRE(arduinoMockGetPinState(kTestPin) == 255);
}

TEST_CASE("blink led twice with delay and repeat", "[jled]") {
  constexpr auto kTestPin = 10;
  arduinoMockInit();
  JLed jled(kTestPin);

  // 1 ms on, 2 ms off + 1 ms delay = 3ms off.
  jled.Blink(1, 2).DelayAfter(1).Repeat(2).DelayBefore(5);
  const std::vector<uint8_t> expected = {
      /* delay before 5ms */ 0, 0, 0, 0, 0,
      /* 1ms on */ 255,
      /* 2ms off */ 0,          0,
      /* 1ms delay */ 0,
      /* repeat */ 255,         0, 0, 0,
      /* finally stay off */ 0};
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

    jled.Blink(kOnDuration, kOffDuration);
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

TEST_CASE("user provided brightness function", "[jled]") {
  constexpr auto kTestPin = 10;
  constexpr auto kDuration = 5;
  constexpr auto kBrightness = 99;

  arduinoMockInit();

  // user func returns sequence (0, 1, 2, 3, 99) for t >= 0
  auto user_func = [](uint32_t t, uint16_t period, uintptr_t param) -> uint8_t {
    return t < kDuration - 1 ? static_cast<uint8_t>(t) : kBrightness;
  };

  JLed jled = JLed(kTestPin).UserFunc(user_func, kDuration, 0);

  const std::vector<uint8_t> expected = {0, 1, 2, 3, kBrightness};
  auto time = 0;
  for (const auto val : expected) {
    jled.Update();
    REQUIRE(arduinoMockGetPinState(kTestPin) == val);
    arduinoMockSetMillis(++time);
  }
  jled.Update();
}
