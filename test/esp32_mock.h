// Copyright 2024-2025 Jan Delgado jdelgado@gmx.net
#ifndef TEST_ESP32_MOCK_H_
#define TEST_ESP32_MOCK_H_

#include <stdint.h>
#include "esp-idf/driver/ledc.h"  // for ledc_*_t types

struct Esp32State {
    int64_t                          timer_us       = 0;
    ledc_channel_config_t            channel_config = {};
    ledc_timer_config_t              timer_config   = {};
    esp32_mock_ledc_update_duty_args update_duty[LEDC_CHANNEL_MAX] = {};
    esp32_mock_ledc_set_duty_args    set_duty[LEDC_CHANNEL_MAX]    = {};

    void    setTimer(int64_t t) { timer_us = t; }
    int64_t getTimer()  const   { return timer_us; }

    ledc_channel_config_t getLedcChannelConfig() const { return channel_config; }
    ledc_timer_config_t   getLedcTimerConfig()   const { return timer_config; }

    esp32_mock_ledc_set_duty_args getLedcSetDuty(ledc_channel_t ch) const { return set_duty[ch]; }
    esp32_mock_ledc_update_duty_args getLedcUpdateDuty(ledc_channel_t ch) const {
        return update_duty[ch];
    }
};

void esp32MockSetInstance(Esp32State* state);

#endif  // TEST_ESP32_MOCK_H_
