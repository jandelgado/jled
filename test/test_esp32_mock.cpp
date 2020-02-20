// JLed Unit tests  (run on host).
// Copyright 2017 Jan Delgado jdelgado@gmx.net

#include "catch.hpp"
#include "esp32.h"  // NOLINT

TEST_CASE("esp32 mock correctly initialized", "[mock]") {
    esp32MockInit();
    for (auto i = 0; i < ESP32_PINS; i++) {
        REQUIRE(arduinoMockGetLedcAttachPin(i) == 0);
        REQUIRE(arduinoMockGetLedcAttachPin(i) == 0);
    }
    for (auto i = 0; i < LEDC_CHANNELS; i++) {
        REQUIRE(arduinoMockGetLedcState(i) == 0);
        REQUIRE(arduinoMockGetLedcSetup(i).freq == 0);
        REQUIRE(arduinoMockGetLedcSetup(i).bit_num == 0);
    }
}

