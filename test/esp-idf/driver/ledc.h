#pragma once

#include <stdint.h>

/*
 * adapted from https://github.com/espressif/esp-idf include files
* SPDX-FileCopyrightText: 2015-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

typedef int esp_err_t;

typedef enum {
    LEDC_HIGH_SPEED_MODE = 0, /*!< LEDC high speed speed_mode */
    LEDC_LOW_SPEED_MODE,      /*!< LEDC low speed speed_mode */
    LEDC_SPEED_MODE_MAX,      /*!< LEDC speed limit */
} ledc_mode_t;

typedef enum {
    LEDC_INTR_DISABLE = 0,    /*!< Disable LEDC interrupt */
    LEDC_INTR_FADE_END,       /*!< Enable LEDC interrupt */
    LEDC_INTR_MAX,
} ledc_intr_type_t;

typedef enum {
    LEDC_AUTO_CLK = 0
} ledc_clk_cfg_t;

typedef enum {
    LEDC_TIMER_0 = 0, /*!< LEDC timer 0 */
    LEDC_TIMER_1,     /*!< LEDC timer 1 */
    LEDC_TIMER_2,     /*!< LEDC timer 2 */
    LEDC_TIMER_3,     /*!< LEDC timer 3 */
    LEDC_TIMER_MAX,
} ledc_timer_t;

typedef enum {
    LEDC_CHANNEL_0 = 0, /*!< LEDC channel 0 */
    LEDC_CHANNEL_1,     /*!< LEDC channel 1 */
    LEDC_CHANNEL_2,     /*!< LEDC channel 2 */
    LEDC_CHANNEL_3,     /*!< LEDC channel 3 */
    LEDC_CHANNEL_4,     /*!< LEDC channel 4 */
    LEDC_CHANNEL_5,     /*!< LEDC channel 5 */
    LEDC_CHANNEL_MAX,
} ledc_channel_t;

typedef enum {
    LEDC_TIMER_1_BIT = 1,   /*!< LEDC PWM duty resolution of  1 bits */
    LEDC_TIMER_2_BIT,       /*!< LEDC PWM duty resolution of  2 bits */
    LEDC_TIMER_3_BIT,       /*!< LEDC PWM duty resolution of  3 bits */
    LEDC_TIMER_4_BIT,       /*!< LEDC PWM duty resolution of  4 bits */
    LEDC_TIMER_5_BIT,       /*!< LEDC PWM duty resolution of  5 bits */
    LEDC_TIMER_6_BIT,       /*!< LEDC PWM duty resolution of  6 bits */
    LEDC_TIMER_7_BIT,       /*!< LEDC PWM duty resolution of  7 bits */
    LEDC_TIMER_8_BIT,       /*!< LEDC PWM duty resolution of  8 bits */
    LEDC_TIMER_9_BIT,       /*!< LEDC PWM duty resolution of  9 bits */
    LEDC_TIMER_10_BIT,      /*!< LEDC PWM duty resolution of 10 bits */
    LEDC_TIMER_11_BIT,      /*!< LEDC PWM duty resolution of 11 bits */
    LEDC_TIMER_12_BIT,      /*!< LEDC PWM duty resolution of 12 bits */
    LEDC_TIMER_13_BIT,      /*!< LEDC PWM duty resolution of 13 bits */
    LEDC_TIMER_14_BIT,      /*!< LEDC PWM duty resolution of 14 bits */
    LEDC_TIMER_BIT_MAX,
} ledc_timer_bit_t;

/**
 * @brief Configuration parameters of LEDC channel for ledc_channel_config function
 */
typedef struct {
    int gpio_num;                   /*!< the LEDC output gpio_num, if you want to use gpio16, gpio_num = 16 */
    ledc_mode_t speed_mode;         /*!< LEDC speed speed_mode, high-speed mode or low-speed mode */
    ledc_channel_t channel;         /*!< LEDC channel (0 - 7) */
    ledc_intr_type_t intr_type;     /*!< configure interrupt, Fade interrupt enable  or Fade interrupt disable */
    ledc_timer_t timer_sel;         /*!< Select the timer source of channel (0 - 3) */
    uint32_t duty;                  /*!< LEDC channel duty, the range of duty setting is [0, (2**duty_resolution)] */
    int hpoint;                     /*!< LEDC channel hpoint value, the max value is 0xfffff */
    struct {
        unsigned int output_invert: 1;/*!< Enable (1) or disable (0) gpio output invert */
    } flags;                        /*!< LEDC flags */

} ledc_channel_config_t;

/**
 * @brief Configuration parameters of LEDC Timer timer for ledc_timer_config function
 */
typedef struct {
    ledc_mode_t speed_mode;                /*!< LEDC speed speed_mode, high-speed mode or low-speed mode */
    ledc_timer_bit_t duty_resolution;      /*!< LEDC channel duty resolution */
    ledc_timer_t  timer_num;               /*!< The timer source of channel (0 - 3) */
    uint32_t freq_hz;                      /*!< LEDC timer frequency (Hz) */
    ledc_clk_cfg_t clk_cfg;                /*!< Configure LEDC source clock.
                                                For low speed channels and high speed channels, you can specify the source clock using LEDC_USE_REF_TICK, LEDC_USE_APB_CLK or LEDC_AUTO_CLK.
                                                For low speed channels, you can also specify the source clock using LEDC_USE_RTC8M_CLK, in this case, all low speed channel's source clock must be RTC8M_CLK*/
} ledc_timer_config_t;

/* mocked versions of APIs used by JLed */

esp_err_t ledc_channel_config(const ledc_channel_config_t* ledc_conf);
// returns last values passed to ledc_timer_config
ledc_channel_config_t esp32_mock_get_ledc_channel_config_args();

esp_err_t ledc_timer_config(const ledc_timer_config_t* timer_conf);
// returns last values passed to ledc_timer_config
ledc_timer_config_t esp32_mock_get_ledc_timer_config_args();

esp_err_t ledc_update_duty(ledc_mode_t speed_mode, ledc_channel_t channel);
typedef struct {
    ledc_mode_t speed_mode;
} esp32_mock_ledc_update_duty_args;
// returns last values passed to ledc_set_duty
esp32_mock_ledc_update_duty_args esp32_mock_get_ledc_update_duty_args(ledc_channel_t channel);

esp_err_t ledc_set_duty(ledc_mode_t speed_mode, ledc_channel_t channel, uint32_t duty);
typedef struct {
    ledc_mode_t speed_mode;
    uint32_t duty;
} esp32_mock_ledc_set_duty_args;
// returns last values passed to ledc_set_duty
esp32_mock_ledc_set_duty_args esp32_mock_get_ledc_set_duty_args(ledc_channel_t channel);

void esp32_mock_init();

