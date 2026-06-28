// Minimal ESP-IDF ledc mock for testing JLed ESP32 hardware accessing functions
// Copyright 2022 Jan Delgado jdelgado@gmx.net
//
#include "ledc.h"
#include "../../esp32_mock.h"  // NOLINT
#include <cassert>

extern Esp32State* gState_;

esp_err_t ledc_channel_config(const ledc_channel_config_t* ledc_conf) {
    assert(gState_);
    gState_->channel_config = *ledc_conf;
    return (esp_err_t)0;
}

esp_err_t ledc_timer_config(const ledc_timer_config_t* timer_conf) {
    assert(gState_);
    gState_->timer_config = *timer_conf;
    return (esp_err_t)0;
}

esp_err_t ledc_update_duty(ledc_mode_t speed_mode, ledc_channel_t channel) {
    assert(gState_);
    assert(channel >= LEDC_CHANNEL_0 && channel < LEDC_CHANNEL_MAX);
    gState_->update_duty[channel] = esp32_mock_ledc_update_duty_args{speed_mode};
    return (esp_err_t)0;
}

esp_err_t ledc_set_duty(ledc_mode_t speed_mode, ledc_channel_t channel, uint32_t duty) {
    assert(gState_);
    assert(channel >= LEDC_CHANNEL_0 && channel < LEDC_CHANNEL_MAX);
    gState_->set_duty[channel] = esp32_mock_ledc_set_duty_args{speed_mode, duty};
    return (esp_err_t)0;
}
