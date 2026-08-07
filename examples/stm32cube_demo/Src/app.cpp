/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
extern "C" {
#include "main.h"
#include "tim.h"  // htim1/htim4 handles
}

#include <jled.h>  // must come after main.h so the STM32Cube HAL header is visible

// Called from the generated main() in main.c, after all MX_TIMx_Init() calls.
// Hardware initialization stays in the generated code, so regenerating the
// project with STM32CubeMX needs no changes here. This never returns.
extern "C" void app_main(void) {
    // JLed objects are created here, after MX_TIMx_Init(), never as globals: the
    // Stm32CubeHal constructor starts PWM and reads htim->Init.Period eagerly.
    //
    // Timer channel to Nucleo F401RE pin mapping (configured in stm32.ioc):
    //   led1  htim1 CH1  PA8       led5  htim4 CH1  PB6
    //   led2  htim1 CH2  PA9       led6  htim4 CH2  PB7
    //   led3  htim1 CH3  PA10      led7  htim4 CH3  PB8
    //   led4  htim1 CH4  PA11      led8  htim4 CH4  PB9
    auto led1 = JLed({&htim1, TIM_CHANNEL_1}).Blink(500, 250, 3).DelayAfter(1000).Forever();
    auto led2 = JLedHD({&htim1, TIM_CHANNEL_2}).FadeOn(1500).Forever();
    auto led3 = JLedHD({&htim1, TIM_CHANNEL_3}).FadeOff(1500).DelayAfter(1000).Forever();
    auto led4 = JLed({&htim1, TIM_CHANNEL_4}).Blink(250, 250, 3).Forever();
    auto led5 = JLed({&htim4, TIM_CHANNEL_1}).Candle().Forever();
    auto led6 = JLedHD({&htim4, TIM_CHANNEL_2}).Breathe(2000).Forever();
    auto led7 = JLedHD({&htim4, TIM_CHANNEL_3}).Breathe(2000).MinBrightness(50).DelayAfter(1000).Forever();
    // A low-active channel exercises hardware invert via LowActive().
    auto led8 = JLedHD({&htim4, TIM_CHANNEL_4}).Breathe(15000).DelayAfter(1000).LowActive().Forever();

    JLedRef leds[]{&led1, &led2, &led3, &led4, &led5, &led6, &led7, &led8};
    auto group = JLedRefGroup::Parallel(leds);
    while (1) {
        group.Update();
    }
}
