// Minimal STM32Cube HAL mock for host-testing JLed's Stm32CubeHal.
// Stands in for the family header (stm32XXxx_hal.h) that is unavailable on the
// host, mirroring test/esp-idf/driver/ledc.h for ESP-IDF.
// Copyright 2026 Jan Delgado jdelgado@gmx.net
#ifndef TEST_STM32CUBE_HAL_MOCK_H_
#define TEST_STM32CUBE_HAL_MOCK_H_

#include <stdint.h>

// --- vendor types (subset used by Stm32CubeHal) ---
typedef enum { HAL_OK = 0 } HAL_StatusTypeDef;

struct TIM_TypeDef {};  // opaque peripheral register block

typedef struct {
    uint32_t Period;  // ARR value; the only Init field Stm32CubeHal reads
} TIM_Base_InitTypeDef;

typedef struct {
    TIM_TypeDef* Instance;
    TIM_Base_InitTypeDef Init;
} TIM_HandleTypeDef;

typedef struct {
    uint32_t OCMode;
    uint32_t Pulse;
    uint32_t OCPolarity;
    uint32_t OCFastMode;
} TIM_OC_InitTypeDef;

// --- vendor constants (real STM32Cube values) ---
#define TIM_CHANNEL_1 0x00000000U
#define TIM_CHANNEL_2 0x00000004U
#define TIM_CHANNEL_3 0x00000008U
#define TIM_CHANNEL_4 0x0000000CU
#define TIM_OCMODE_PWM1 0x00000060U
#define TIM_OCPOLARITY_HIGH 0x00000000U
#define TIM_OCPOLARITY_LOW 0x00000002U
#define TIM_OCFAST_DISABLE 0x00000000U

// --- mock state, one instance installed per test ---
struct Stm32CubeMockState {
    uint32_t tick = 0;
    int pwm_start_count = 0;
    uint32_t last_start_channel = 0;
    uint32_t compare[4] = {};  // per-channel CCR, indexed by channel / 4
    TIM_OC_InitTypeDef last_oc_config = {};
    uint32_t last_config_channel = 0;
};

void stm32MockSetInstance(Stm32CubeMockState* state);
int stm32MockChanIndex(uint32_t channel);

// --- vendor API (mocked) ---
extern "C" {
uint32_t HAL_GetTick(void);
HAL_StatusTypeDef HAL_TIM_PWM_Start(TIM_HandleTypeDef* htim, uint32_t channel);
HAL_StatusTypeDef HAL_TIM_PWM_ConfigChannel(TIM_HandleTypeDef* htim,
                                            const TIM_OC_InitTypeDef* config,
                                            uint32_t channel);
}

// __HAL_TIM_SET_COMPARE / _GET_COMPARE are macros in the real SDK; keep them
// macros here so Stm32CubeHal's call sites are exercised unchanged.
uint32_t stm32MockGetCompare(TIM_HandleTypeDef* htim, uint32_t channel);
void stm32MockSetCompare(TIM_HandleTypeDef* htim, uint32_t channel, uint32_t val);
#define __HAL_TIM_SET_COMPARE(htim, ch, val) stm32MockSetCompare((htim), (ch), (val))
#define __HAL_TIM_GET_COMPARE(htim, ch) stm32MockGetCompare((htim), (ch))

#endif  // TEST_STM32CUBE_HAL_MOCK_H_
