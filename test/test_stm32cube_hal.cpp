// JLed unit tests for the STM32Cube HAL (runs on host).
// Copyright 2026 Jan Delgado jdelgado@gmx.net
// clang-format off
#include "stm32cube_hal_mock.h"  // must precede stm32cube_hal.h (include-order contract)

#include <stm32cube_hal.h>  // NOLINT
// clang-format on

#include "catch2/catch_amalgamated.hpp"

using jled::Stm32CubeClock;
using jled::Stm32CubeHal;
using jled::Stm32PwmChannel;

struct Stm32CubeMockFixture {
    Stm32CubeMockState mock{};
    TIM_HandleTypeDef htim{};
    Stm32CubeMockFixture() { stm32MockSetInstance(&mock); }
    ~Stm32CubeMockFixture() { stm32MockSetInstance(nullptr); }
};

TEST_CASE_METHOD(Stm32CubeMockFixture, "constructor starts PWM once and caches period",
                 "[stm32cube_hal]") {
    htim.Init.Period = 999;

    SECTION("starts PWM once on the given channel") {
        Stm32CubeHal({&htim, TIM_CHANNEL_2});  // constructed for its side effect
        REQUIRE(mock.pwm_start_count == 1);
        REQUIRE(mock.last_start_channel == TIM_CHANNEL_2);
    }

    SECTION("a copy touches no hardware and keeps the cached period/channel") {
        // TJLed copies its Hal on copy/assign, that copy must not re-start PWM.
        auto hal = Stm32CubeHal({&htim, TIM_CHANNEL_2});
        auto hal_copy = hal;
        REQUIRE(mock.pwm_start_count == 1);
        // full brightness at 999 -> 1000 proves the copy kept period_/channel_
        hal_copy.analogWrite<uint8_t>(255);
        REQUIRE(mock.compare[stm32MockChanIndex(TIM_CHANNEL_2)] == 1000);
    }
}

TEST_CASE_METHOD(Stm32CubeMockFixture, "analogWrite scales brightness to timer period",
                 "[stm32cube_hal]") {
    const int idx = stm32MockChanIndex(TIM_CHANNEL_1);

    SECTION("8-bit brightness, non-power-of-two period") {
        struct Case {
            uint8_t brightness;
            uint32_t expected;
            const char* what;
        };
        auto tc = GENERATE(values<Case>({
            {0, 0, "zero -> 0"},
            {128, 501, "scaling: 128 * 999 / 255"},
            {255, 1000, "full brightness: 999 + 1 (period below full width)"},
        }));
        CAPTURE(tc.what);

        htim.Init.Period = 999;
        auto hal = Stm32CubeHal({&htim, TIM_CHANNEL_1});
        hal.analogWrite<uint8_t>(tc.brightness);
        REQUIRE(mock.compare[idx] == tc.expected);
    }

    SECTION("8-bit brightness, period equals full brightness (identity fast path)") {
        // GIVEN
        htim.Init.Period = 255;  // period_ == kFull -> duty = val, no mul/div
        auto hal = Stm32CubeHal({&htim, TIM_CHANNEL_1});

        // WHEN
        hal.analogWrite<uint8_t>(200);  // identity: CCR == val
        // THEN
        REQUIRE(mock.compare[idx] == 200);
    }

    SECTION("16-bit brightness, large period") {
        // GIVEN
        htim.Init.Period = 65535;
        auto hal = Stm32CubeHal({&htim, TIM_CHANNEL_1});

        // WHEN
        hal.analogWrite<uint16_t>(32768);  // 32768 * 65535 / 65535 == 32768
        // THEN
        REQUIRE(mock.compare[idx] == 32768);

        // At full-width period the code caps CCR at ARR (no +1) to avoid
        // overflowing the register.
        hal.analogWrite<uint16_t>(65535);  // full: CCR capped at ARR
        REQUIRE(mock.compare[idx] == 65535);
    }
}

TEST_CASE_METHOD(Stm32CubeMockFixture, "analogWrite(val, invert) ignores invert",
                 "[stm32cube_hal]") {
    htim.Init.Period = 1000;
    auto hal = Stm32CubeHal({&htim, TIM_CHANNEL_1});
    const int idx = stm32MockChanIndex(TIM_CHANNEL_1);
    const auto invert = GENERATE(false, true);
    CAPTURE(invert);

    // Hardware owns inversion: duty is identical whatever the flag
    hal.analogWrite<uint8_t>(200, invert);
    REQUIRE(mock.compare[idx] == 784);  // 200 * 1000 / 255
}

TEST_CASE_METHOD(Stm32CubeMockFixture, "SetLowActive flips polarity, preserves duty",
                 "[stm32cube_hal]") {
    htim.Init.Period = 1000;
    auto hal = Stm32CubeHal({&htim, TIM_CHANNEL_3});
    hal.analogWrite<uint16_t>(12345);  // establish an in-flight compare value
    const uint32_t duty = mock.compare[stm32MockChanIndex(TIM_CHANNEL_3)];

    struct Case {
        bool low_active;
        uint32_t expected_polarity;
    };
    auto tc = GENERATE(values<Case>({
        {true, TIM_OCPOLARITY_LOW},
        {false, TIM_OCPOLARITY_HIGH},
    }));
    CAPTURE(tc.low_active);

    hal.SetLowActive(tc.low_active);
    REQUIRE(mock.last_config_channel == TIM_CHANNEL_3);
    REQUIRE(mock.last_oc_config.OCMode == TIM_OCMODE_PWM1);
    REQUIRE(mock.last_oc_config.OCPolarity == tc.expected_polarity);
    REQUIRE(mock.last_oc_config.Pulse == duty);  // duty preserved
}

TEST_CASE_METHOD(Stm32CubeMockFixture, "Stm32CubeClock::millis forwards to HAL_GetTick",
                 "[stm32cube_hal]") {
    mock.tick = 4242;
    REQUIRE(Stm32CubeClock::millis() == 4242);
}
