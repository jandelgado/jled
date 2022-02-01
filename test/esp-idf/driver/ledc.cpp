// Minimal ESP-IDF ledc mock for testing JLed ESP32 hardware accessing functions
// Copyright 2022 Jan Delgado jdelgado@gmx.net
//
#include "ledc.h"
#include <assert.h>
#include <cstring>    // NOLINT

struct ESP32State {
    ledc_channel_config_t channel_config;
    ledc_timer_config_t timer_config;
    esp32_mock_ledc_update_duty_args update_duty[LEDC_CHANNEL_MAX];
    esp32_mock_ledc_set_duty_args set_duty[LEDC_CHANNEL_MAX];
} ESP32State_;

void esp32_mock_init() {
    // TODO(jd) introduce UNDEFINED state to mock instead of initalizing with 0
    bzero(&ESP32State_, sizeof(ESP32State_));
}

esp_err_t ledc_channel_config(const ledc_channel_config_t* ledc_conf) {
    ESP32State_.channel_config = *ledc_conf;
    return (esp_err_t)0;
}

ledc_channel_config_t esp32_mock_get_ledc_channel_config_args() {
    return ESP32State_.channel_config;
}

esp_err_t ledc_timer_config(const ledc_timer_config_t* timer_conf) {
    ESP32State_.timer_config = *timer_conf;
    return (esp_err_t)0;
}

ledc_timer_config_t esp32_mock_get_ledc_timer_config_args() {
    return ESP32State_.timer_config;
}

esp_err_t ledc_update_duty(ledc_mode_t speed_mode, ledc_channel_t channel) {
    assert(channel >= LEDC_CHANNEL_0 && channel <= LEDC_CHANNEL_MAX);
    ESP32State_.update_duty[channel] = esp32_mock_ledc_update_duty_args{speed_mode};
    return (esp_err_t)0;
}

esp32_mock_ledc_update_duty_args esp32_mock_get_ledc_update_duty_args(ledc_channel_t channel) {
    assert(channel >= LEDC_CHANNEL_0 && channel <= LEDC_CHANNEL_MAX);
    return ESP32State_.update_duty[channel];
}

esp_err_t ledc_set_duty(ledc_mode_t speed_mode, ledc_channel_t channel, uint32_t duty) {
    assert(channel >= LEDC_CHANNEL_0 && channel <= LEDC_CHANNEL_MAX);
    ESP32State_.set_duty[channel] = esp32_mock_ledc_set_duty_args{speed_mode, duty};
    return (esp_err_t)0;
}

esp32_mock_ledc_set_duty_args esp32_mock_get_ledc_set_duty_args(ledc_channel_t channel) {
    assert(channel >= LEDC_CHANNEL_0 && channel <= LEDC_CHANNEL_MAX);
    return ESP32State_.set_duty[channel];
}

