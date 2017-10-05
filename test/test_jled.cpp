// JLed Unit tests  (run on host).
// Copyright 2017 Jan Delgado jdelgado@gmx.net
#include <iostream>
#include <vector>
#include <map>

#define CATCH_CONFIG_MAIN
// #define NOMINMAX 1
#include "catch.hpp"
#include <jled.h>     // NOLINT

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

TEST_CASE("turn led on and off", "[jled]") {
    constexpr auto kTestPin = 10;
    arduinoMockInit();
    JLed jled(kTestPin);

    SECTION("turn led on and off") {
        jled.On().Update();
        REQUIRE(arduinoMockGetPinState(kTestPin) == 255);
        jled.Off().Update();
        REQUIRE(arduinoMockGetPinState(kTestPin) == 0);
    }
    SECTION("turn led on with delay") {
        constexpr auto kDelayTime = 5;
        jled.On().DelayBefore(kDelayTime);
        // expect that led turns on after kDelayTime
        arduinoMockSetMillis(0);
        jled.Update();
        REQUIRE(arduinoMockGetPinState(kTestPin) == 0);
        arduinoMockSetMillis(kDelayTime-1);
        jled.Update();
        REQUIRE(arduinoMockGetPinState(kTestPin) == 0);
        arduinoMockSetMillis(kDelayTime);
        jled.Update();
        REQUIRE(arduinoMockGetPinState(kTestPin) == 255);
    }
}

TEST_CASE("turn led on and off with Set()", "[jled]") {
    constexpr auto kTestPin = 10;
    arduinoMockInit();
    JLed jled(kTestPin);

    jled.Set(true).Update();
    REQUIRE(arduinoMockGetPinState(kTestPin) == 255);
    jled.Set(false).Update();
    REQUIRE(arduinoMockGetPinState(kTestPin) == 0);
}

TEST_CASE("set and test forever", "[jled]") {
    constexpr auto kTestPin = 10;
    arduinoMockInit();
    JLed jled(kTestPin);
    REQUIRE_FALSE(jled.IsForever());
    jled.Forever();
    REQUIRE(jled.IsForever());
}

TEST_CASE("stop effect", "[jled]") {
    constexpr auto kTestPin = 10;
    constexpr auto kDuration = 100;
    arduinoMockInit();

    // we test that an effect that normally has high ouput for a longer
    // time (e.g. FadeOff()) stays off after Stop() was called
    JLed jled = JLed(kTestPin).FadeOff(kDuration);

    REQUIRE(jled.IsRunning());
    jled.Update();
    REQUIRE(arduinoMockGetPinState(kTestPin) > 0);
    jled.Stop();
    REQUIRE(arduinoMockGetPinState(kTestPin) == 0);
    REQUIRE_FALSE(jled.IsRunning());
}

TEST_CASE("inverted output", "[jled]") {
    constexpr auto kTestPin = 10;
    arduinoMockInit();
    JLed jled = JLed(kTestPin).Invert();

    SECTION("turn inverted led on") {
        jled.On().Update();
        REQUIRE(arduinoMockGetPinState(kTestPin) == 0);
    }
    SECTION("turn inverted led off") {
        jled.Off().Update();
        REQUIRE(arduinoMockGetPinState(kTestPin) == 255);
    }
}

TEST_CASE("blink led twice", "[jled]") {
    constexpr auto kTestPin = 10;
    arduinoMockInit();
    JLed jled(kTestPin);

    // 1 ms on, 2 ms off
    jled.Blink(1, 2).Repeat(2);
    std::vector<uint8_t> expected =
       { /* 1ms on */ 255, /* 2ms off */ 0, 0,
         /* dito */ 255, 0, 0,
         /* finally stay off */ 0 };
    uint32_t time = 0;
    for (auto val : expected) {
        jled.Update();
        REQUIRE(arduinoMockGetPinState(kTestPin) == val);
        arduinoMockSetMillis(++time);
    }
    REQUIRE_FALSE(jled.IsRunning());
}

TEST_CASE("blink led twice with delay", "[jled]") {
    constexpr auto kTestPin = 10;
    arduinoMockInit();
    JLed jled(kTestPin);

    // 1 ms on, 2 ms off + 1 ms delay = 3ms off.
    jled.Blink(1, 2).DelayAfter(1).Repeat(2);
    std::vector<uint8_t> expected =
       { /* 1ms on */ 255, /* 2ms off */ 0, 0, /*1ms delay*/ 0,
         /* dito */ 255, 0, 0, 0,
         /* finally stay off */ 0 };
    uint32_t time = 0;
    for (auto val : expected) {
        jled.Update();
        REQUIRE(arduinoMockGetPinState(kTestPin) == val);
        arduinoMockSetMillis(++time);
    }
    REQUIRE_FALSE(jled.IsRunning());
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
            REQUIRE(arduinoMockGetPinState(kTestPin) == (state?255:0));
            timer++;
            if (timer >= kOnDuration+kOffDuration) {
                timer = 0;
            }
            arduinoMockSetMillis(++time);
        }
    }
}

TEST_CASE("fade led on with delay before", "[jled]") {
    constexpr auto kTestPin = 10;
    constexpr auto kDuration = 2000;
    constexpr auto kDelayBefore = 500;

    arduinoMockInit();
    JLed jled = JLed(kTestPin).FadeOn(kDuration).DelayBefore(kDelayBefore);

    // test some values we expect the fade-on function should produce
    std::map<uint32_t, uint8_t> test_values = {
        {0, 0},
        {kDelayBefore, 0},
        {kDelayBefore+500, 13},
        {kDelayBefore+1000, 68},
        {kDelayBefore+1500, 179},
        {kDelayBefore+kDuration-1, 255},
        {kDelayBefore+kDuration, 255}};
    for (auto &x : test_values) {
        arduinoMockSetMillis(x.first);
        jled.Update();
        auto val = arduinoMockGetPinState(kTestPin);
        REQUIRE(x.second == val);
    }
}

TEST_CASE("fade led off with delay before", "[jled]") {
    constexpr auto kTestPin = 10;
    constexpr auto kDuration = 2000;
    constexpr auto kDelayBefore = 500;

    arduinoMockInit();
    JLed jled = JLed(kTestPin).FadeOff(kDuration).DelayBefore(kDelayBefore);
    // the fade-off function is an inverse of the fade-on function
    std::map<uint32_t, uint8_t> test_values = {
        {0, 0},
        {kDelayBefore-1, 0},
        {kDelayBefore, 255},
        {kDelayBefore+500, 255-13},
        {kDelayBefore+1000, 255-68},
        {kDelayBefore+1500, 255-179},
        {kDelayBefore+kDuration-1, 0},
        {kDelayBefore+kDuration, 0}};
    for (auto &x : test_values) {
        arduinoMockSetMillis(x.first);
        jled.Update();
        auto val = arduinoMockGetPinState(kTestPin);
        REQUIRE(x.second == val);
    }
}

TEST_CASE("user provided brightness function", "[jled]") {
    constexpr auto kTestPin = 10;
    constexpr auto kDuration = 5;

    arduinoMockInit();

    // user func returns sequence (0, 1, 2, 3, 99) for t >= 0
    auto user_func =
        [](uint32_t t, uint16_t period, uint32_t param) -> uint8_t {
          return t < kDuration-1 ? static_cast<uint8_t>(t) : 99; };

    JLed jled = JLed(kTestPin).UserFunc(user_func, kDuration);

    std::vector<uint8_t> expected = { 0, 1, 2, 3, 99 };
    auto time = 0;
    for (auto val : expected) {
        jled.Update();
        REQUIRE(arduinoMockGetPinState(kTestPin) == val);
        arduinoMockSetMillis(++time);
    }
    jled.Update();
    REQUIRE_FALSE(jled.IsRunning());
}

