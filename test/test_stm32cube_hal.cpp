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
    (void)hal_copy;
}
