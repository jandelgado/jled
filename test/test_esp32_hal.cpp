// JLed Unit tests for the ESP32 HAL (runs on host).
// Copyright 2017-2025 Jan Delgado jdelgado@gmx.net
#define ESP_IDF_VERSION_MAJOR 5
#include <esp32_hal.h>  // NOLINT
#include "catch2/catch_amalgamated.hpp"
#include "esp32_mock.h"  // NOLINT

using jled::Esp32ChanMapper;
using jled::Esp32Hal;

struct Esp32MockFixture {
    Esp32State mock{};
    Esp32MockFixture()  { esp32MockSetInstance(&mock); }
    ~Esp32MockFixture() { esp32MockSetInstance(nullptr); }
};

TEST_CASE("Esp32ChanMapper", "[esp32_hal]") {
    SECTION("new channels for different pins") {
        auto m = Esp32ChanMapper();

        REQUIRE(m.chanForPin(10) == 0);
        REQUIRE(m.chanForPin(15) == 1);
        REQUIRE(m.chanForPin(3) == 2);
        REQUIRE(m.chanForPin(1) == 3);

        // no change when same pins are requested
        REQUIRE(m.chanForPin(10) == 0);
        REQUIRE(m.chanForPin(15) == 1);
        REQUIRE(m.chanForPin(3) == 2);
        REQUIRE(m.chanForPin(1) == 3);

        REQUIRE(m.chanForPin(7) == 4);
    }

    SECTION("wraps when exhausted") {
        auto m = Esp32ChanMapper();

        for (auto i = 0; i < Esp32ChanMapper::kLedcMaxChan; i++) {
            REQUIRE(m.chanForPin(i) == i);
        }

        // now all channels are used and the mapper starts over at 0
        REQUIRE(m.chanForPin(100) == 0);
        REQUIRE(m.chanForPin(101) == 1);
    }
}

TEST_CASE_METHOD(Esp32MockFixture, "Esp32Hal<8> constructor", "[esp32_hal]") {
    constexpr auto kChan = 5;
    constexpr auto kPin = 10;
    auto hal[[gnu::unused]] = Esp32Hal<8>(kPin, kChan);

    SECTION("timer config correct") {
        auto timer_config = mock.getLedcTimerConfig();
        REQUIRE(timer_config.speed_mode == LEDC_LOW_SPEED_MODE);
        REQUIRE(timer_config.duty_resolution == LEDC_TIMER_8_BIT);
        REQUIRE(timer_config.timer_num == LEDC_TIMER_0);
        REQUIRE(timer_config.freq_hz == 5000);
        REQUIRE(timer_config.clk_cfg == LEDC_AUTO_CLK);
    }

    SECTION("channel config correct") {
        auto chan_config = mock.getLedcChannelConfig();
        REQUIRE(chan_config.gpio_num == kPin);
        REQUIRE(chan_config.speed_mode == LEDC_LOW_SPEED_MODE);
        REQUIRE(chan_config.channel == kChan);
        REQUIRE(chan_config.intr_type == LEDC_INTR_DISABLE);
        REQUIRE(chan_config.timer_sel == LEDC_TIMER_0);
        REQUIRE(chan_config.hpoint == 0);
        REQUIRE(chan_config.duty == 0);
        REQUIRE(chan_config.flags.output_invert == 0);
    }
}

TEST_CASE_METHOD(Esp32MockFixture, "Esp32Hal channel selection", "[esp32_hal]") {
    constexpr auto kPin = 10;

    SECTION("same channel for same pin") {
        auto hal1 = Esp32Hal<8>(kPin);
        auto hal2 = Esp32Hal<8>(kPin);
        REQUIRE(hal1.chan() == hal2.chan());
    }

    SECTION("different channels for different pins") {
        auto hal1 = Esp32Hal<8>(kPin);
        auto hal2 = Esp32Hal<8>(kPin + 1);
        REQUIRE(hal1.chan() != hal2.chan());
    }
}

TEST_CASE_METHOD(Esp32MockFixture, "Esp32Hal<8> analogWrite", "[esp32_hal]") {
    constexpr auto kChan = 5;
    constexpr auto kPin = 10;
    auto hal = Esp32Hal<8>(kPin, kChan);

    SECTION("writes value") {
        hal.analogWrite<uint8_t>(123);
        auto set_duty = mock.getLedcSetDuty((ledc_channel_t)kChan);
        REQUIRE(set_duty.speed_mode == LEDC_LOW_SPEED_MODE);
        REQUIRE(set_duty.duty == 123);
        auto update_duty = mock.getLedcUpdateDuty((ledc_channel_t)kChan);
        REQUIRE(update_duty.speed_mode == LEDC_LOW_SPEED_MODE);
    }

    SECTION("writes 0 as 0") {
        hal.analogWrite<uint8_t>(0);
        auto set_duty = mock.getLedcSetDuty((ledc_channel_t)kChan);
        REQUIRE(set_duty.duty == 0);
    }

    SECTION("writes 255 as 256 (full-on fix)") {
        hal.analogWrite<uint8_t>(255);
        auto set_duty = mock.getLedcSetDuty((ledc_channel_t)kChan);
        REQUIRE(set_duty.duty == 256);
    }
}

TEST_CASE_METHOD(Esp32MockFixture, "Esp32Hal<13> analogWrite 16-bit", "[esp32_hal]") {
    constexpr auto kChan = 5;
    constexpr auto kPin = 10;
    auto hal = Esp32Hal<13>(kPin, kChan);

    SECTION("0 passthrough") {
        hal.analogWrite<uint16_t>(0);
        auto set_duty = mock.getLedcSetDuty((ledc_channel_t)kChan);
        REQUIRE(set_duty.duty == 0);
    }

    SECTION("max full-on fix") {
        hal.analogWrite<uint16_t>(65535);
        auto set_duty = mock.getLedcSetDuty((ledc_channel_t)kChan);
        REQUIRE(set_duty.duty == (1 << 13));
    }

    SECTION("mid value") {
        hal.analogWrite<uint16_t>(32768);
        auto set_duty = mock.getLedcSetDuty((ledc_channel_t)kChan);
        REQUIRE(set_duty.duty == 4096);
    }
}

TEST_CASE_METHOD(Esp32MockFixture, "Esp32Clock::millis()", "[esp32_hal]") {
    SECTION("returns 0") {
        mock.setTimer(0);
        REQUIRE(jled::Esp32Clock::millis() == 0);
    }

    SECTION("us to ms conversion") {
        mock.setTimer(99 * 1000);
        REQUIRE(jled::Esp32Clock::millis() == 99);
    }
}
