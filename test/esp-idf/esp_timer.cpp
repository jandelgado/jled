// ESP32 ESP-IDF mock
// Copyright (c) 2017-2022 Jan Delgado <jdelgado[at]gmx.net>
//
#include "esp_timer.h"  // NOLINT
#include <cassert>
#include "../esp32_mock.h"  // NOLINT

extern Esp32State* gState_;

int64_t esp_timer_get_time() {
    assert(gState_);
    return gState_->timer_us;
}
