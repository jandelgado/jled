// Minimal ESP-IDF esp_idf_version.h mock for testing JLed's ESP-IDF
// version-gated code paths (runs on host).
// Copyright 2026 Jan Delgado jdelgado@gmx.net
//
// adapted from https://github.com/espressif/esp-idf include files
// SPDX-FileCopyrightText: 2019-2022 Espressif Systems (Shanghai) CO LTD
#pragma once

// Unlike the real header (which bakes in the SDK's own version), the
// defaults here can be overridden by #define-ing these before #include-ing
// esp32_hal.h, so tests can target a specific ESP-IDF version.
#ifndef ESP_IDF_VERSION_MAJOR
#define ESP_IDF_VERSION_MAJOR 5
#endif
#ifndef ESP_IDF_VERSION_MINOR
#define ESP_IDF_VERSION_MINOR 0
#endif
#ifndef ESP_IDF_VERSION_PATCH
#define ESP_IDF_VERSION_PATCH 0
#endif

#define ESP_IDF_VERSION_VAL(major, minor, patch) (((major) << 16) | ((minor) << 8) | (patch))

#define ESP_IDF_VERSION \
    ESP_IDF_VERSION_VAL(ESP_IDF_VERSION_MAJOR, ESP_IDF_VERSION_MINOR, ESP_IDF_VERSION_PATCH)
