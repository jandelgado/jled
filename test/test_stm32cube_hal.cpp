// JLed unit tests for the STM32Cube HAL (runs on host).
// Copyright 2026 Jan Delgado jdelgado@gmx.net
#include "stm32cube_hal_mock.h"  // must precede stm32cube_hal.h (include-order contract)

#include <stm32cube_hal.h>  // NOLINT

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

    auto hal = Stm32CubeHal({&htim, TIM_CHANNEL_2});

    REQUIRE(mock.pwm_start_count == 1);
    REQUIRE(mock.last_start_channel == TIM_CHANNEL_2);

    // A copy must not touch hardware (TJLed copies its Hal on copy/assign).
    auto hal_copy = hal;
    REQUIRE(mock.pwm_start_count == 1);
    // full at 999 -> 1000, proves the copy kept period_/channel_
    hal_copy.analogWrite<uint8_t>(255);
    REQUIRE(mock.compare[stm32MockChanIndex(TIM_CHANNEL_2)] == 1000);
}

TEST_CASE_METHOD(Stm32CubeMockFixture, "analogWrite scales brightness to timer period",
                 "[stm32cube_hal]") {
    const int idx = stm32MockChanIndex(TIM_CHANNEL_1);

    SECTION("8-bit brightness, non-power-of-two period") {
        htim.Init.Period = 999;  // proves no hidden bit-width assumption
        auto hal = Stm32CubeHal({&htim, TIM_CHANNEL_1});

        hal.analogWrite<uint8_t>(0);
        REQUIRE(mock.compare[idx] == 0);

        hal.analogWrite<uint8_t>(128);  // 128 * 999 / 255 == 501
        REQUIRE(mock.compare[idx] == 501);

        // 999 is below full width, so the +1 applies at full brightness.
        hal.analogWrite<uint8_t>(255);  // full: CCR = period + 1
        REQUIRE(mock.compare[idx] == 1000);
    }

    SECTION("16-bit brightness, large period") {
        htim.Init.Period = 65535;
        auto hal = Stm32CubeHal({&htim, TIM_CHANNEL_1});

        hal.analogWrite<uint16_t>(32768);  // 32768 * 65535 / 65535 == 32768
        REQUIRE(mock.compare[idx] == 32768);

        // At full-width period the code caps CCR at ARR (no +1) to avoid
        // overflowing the register.
        hal.analogWrite<uint16_t>(65535);  // full: CCR capped at ARR
        REQUIRE(mock.compare[idx] == 65535);
    }
}

TEST_CASE_METHOD(Stm32CubeMockFixture, "full brightness does not overflow a 16-bit CCR",
                 "[stm32cube_hal]") {
    htim.Init.Period = 0xFFFF;  // full-width 16-bit timer period
    auto hal = Stm32CubeHal({&htim, TIM_CHANNEL_1});
    hal.analogWrite<uint16_t>(65535);  // full: must cap at ARR, never period+1
    REQUIRE(mock.compare[stm32MockChanIndex(TIM_CHANNEL_1)] == 0xFFFF);
}

TEST_CASE_METHOD(Stm32CubeMockFixture, "period is cached at construction, not re-read",
                 "[stm32cube_hal]") {
    htim.Init.Period = 1000;
    auto hal = Stm32CubeHal({&htim, TIM_CHANNEL_1});
    htim.Init.Period = 500;             // change after construction, must be ignored
    hal.analogWrite<uint8_t>(128);      // 128 * 1000 / 255 == 501, using cached 1000
    REQUIRE(mock.compare[stm32MockChanIndex(TIM_CHANNEL_1)] == 501);
}

TEST_CASE_METHOD(Stm32CubeMockFixture, "analogWrite(val, invert) ignores invert",
                 "[stm32cube_hal]") {
    htim.Init.Period = 1000;
    auto hal = Stm32CubeHal({&htim, TIM_CHANNEL_1});
    const int idx = stm32MockChanIndex(TIM_CHANNEL_1);

    hal.analogWrite<uint8_t>(128, false);
    const uint32_t duty_false = mock.compare[idx];
    REQUIRE(duty_false == 501);  // 128 * 1000 / 255, a concrete expected value
    hal.analogWrite<uint8_t>(128, true);
    REQUIRE(mock.compare[idx] == duty_false);  // hardware owns inversion
}

TEST_CASE_METHOD(Stm32CubeMockFixture, "SetLowActive flips polarity, preserves duty",
                 "[stm32cube_hal]") {
    htim.Init.Period = 1000;
    auto hal = Stm32CubeHal({&htim, TIM_CHANNEL_3});
    hal.analogWrite<uint16_t>(12345);  // establish an in-flight compare value
    const uint32_t duty = mock.compare[stm32MockChanIndex(TIM_CHANNEL_3)];

    SECTION("low active -> OCPOLARITY_LOW") {
        hal.SetLowActive(true);
        REQUIRE(mock.last_config_channel == TIM_CHANNEL_3);
        REQUIRE(mock.last_oc_config.OCMode == TIM_OCMODE_PWM1);
        REQUIRE(mock.last_oc_config.OCPolarity == TIM_OCPOLARITY_LOW);
        REQUIRE(mock.last_oc_config.Pulse == duty);  // duty preserved
    }

    SECTION("high active -> OCPOLARITY_HIGH") {
        hal.SetLowActive(false);
        REQUIRE(mock.last_oc_config.OCPolarity == TIM_OCPOLARITY_HIGH);
        REQUIRE(mock.last_oc_config.Pulse == duty);
    }
}

TEST_CASE_METHOD(Stm32CubeMockFixture, "Stm32CubeClock::millis forwards to HAL_GetTick",
                 "[stm32cube_hal]") {
    mock.tick = 4242;
    REQUIRE(Stm32CubeClock::millis() == 4242);
}
