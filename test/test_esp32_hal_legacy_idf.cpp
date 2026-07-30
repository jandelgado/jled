// JLed unit tests for Esp32Hal on pre-4.4 ESP-IDF, where the LEDC driver has
// no output_invert flag (runs on host). jled.h wraps Esp32Hal in
// InvertableHal on these SDKs (see esp32_hal.h), so that's what's under test
// here; see test_esp32_hal.cpp for the native hardware-invert (ESP-IDF >=
// 4.4) path, where Esp32Hal is used bare.
// Copyright 2026 Jan Delgado jdelgado@gmx.net
#define ESP_IDF_VERSION_MAJOR 4
#define ESP_IDF_VERSION_MINOR 3
#include <esp32_hal.h>  // NOLINT

#include "catch2/catch_amalgamated.hpp"
#include "esp32_mock.h"      // NOLINT
#include "invertable_hal.h"  // NOLINT

using jled::Esp32Hal;
using jled::InvertableHal;

// Pre-4.4, Esp32Hal has no analogWrite(val, invert) or SetLowActive() of its
// own (see JLED_ESP32_HAS_LEDC_OUTPUT_INVERT in esp32_hal.h), so it doesn't
// satisfy the mandatory HAL contract on its own; jled.h relies on the same
// macro to decide to wrap it in InvertableHal instead.

struct Esp32LegacyMockFixture {
    Esp32State mock{};
    Esp32LegacyMockFixture() { esp32MockSetInstance(&mock); }
    ~Esp32LegacyMockFixture() { esp32MockSetInstance(nullptr); }
};

TEST_CASE_METHOD(Esp32LegacyMockFixture, "InvertableHal<Esp32Hal<8>> pre-4.4 ESP-IDF fallback",
                 "[esp32_hal]") {
    constexpr auto kChan = 5;
    constexpr auto kPin = 10;
    auto hal = InvertableHal<Esp32Hal<8>>(kPin, kChan);

    SECTION("analogWrite(val, false) passes the value through unchanged") {
        hal.analogWrite<uint8_t>(100, false);
        REQUIRE(mock.getLedcSetDuty((ledc_channel_t)kChan).duty == 100);
    }

    SECTION("analogWrite(val, true) inverts the value in software") {
        hal.analogWrite<uint8_t>(0, true);
        // 0 inverted is 255 (full brightness), triggering the same full-on
        // fix a plain analogWrite(255) would.
        REQUIRE(mock.getLedcSetDuty((ledc_channel_t)kChan).duty == 256);
    }
}
