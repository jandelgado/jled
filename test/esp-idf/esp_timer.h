// ESP32 ESP-IDF mock
// Copyright (c) 2017-2022 Jan Delgado <jdelgado[at]gmx.net>
//
#pragma once
#include <stdint.h>

int64_t esp_timer_get_time();

void esp32_mock_set_esp_timer(int64_t t);

