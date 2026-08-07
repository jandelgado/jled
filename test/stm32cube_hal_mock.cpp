// Copyright 2026 Jan Delgado jdelgado@gmx.net
#include "stm32cube_hal_mock.h"

#include <cassert>

static Stm32CubeMockState* gState_ = nullptr;

void stm32MockSetInstance(Stm32CubeMockState* state) {
    gState_ = state;
    if (state) *state = Stm32CubeMockState{};
}

int stm32MockChanIndex(uint32_t channel) {
    const int idx = static_cast<int>(channel / 4);
    assert(idx >= 0 && idx < 4);
    return idx;
}

extern "C" uint32_t HAL_GetTick(void) {
    assert(gState_);
    return gState_->tick;
}

extern "C" HAL_StatusTypeDef HAL_TIM_PWM_Start(TIM_HandleTypeDef* /*htim*/, uint32_t channel) {
    assert(gState_);
    gState_->pwm_start_count++;
    gState_->last_start_channel = channel;
    return HAL_OK;
}

extern "C" HAL_StatusTypeDef HAL_TIM_PWM_ConfigChannel(TIM_HandleTypeDef* /*htim*/,
                                                       const TIM_OC_InitTypeDef* config,
                                                       uint32_t channel) {
    assert(gState_);
    gState_->last_oc_config = *config;
    gState_->last_config_channel = channel;
    return HAL_OK;
}

uint32_t stm32MockGetCompare(TIM_HandleTypeDef* /*htim*/, uint32_t channel) {
    assert(gState_);
    return gState_->compare[stm32MockChanIndex(channel)];
}

void stm32MockSetCompare(TIM_HandleTypeDef* /*htim*/, uint32_t channel, uint32_t val) {
    assert(gState_);
    gState_->compare[stm32MockChanIndex(channel)] = val;
}
