// ESP32 ESP-IDF mock
// Copyright (c) 2017-2022 Jan Delgado <jdelgado[at]gmx.net>
//
#include "esp_timer.h"  // NOLINT

int64_t esp32_mock_time_;

int64_t esp_timer_get_time() {
    return esp32_mock_time_;
}

void esp32_mock_set_esp_timer(int64_t t) {
    esp32_mock_time_ = t;
}

