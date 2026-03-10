// JLed Unit tests for the ESP32 HAL (runs on host).
// Copyright 2017-2022 Jan Delgado jdelgado@gmx.net
#define ESP_IDF_VERSION_MAJOR 5
#include <esp32_hal.h>  // NOLINT
#include "catch2/catch_amalgamated.hpp"

using jled::Esp32ChanMapper;
using jled::Esp32Hal;

TEST_CASE("channel mapper returns new channels for different pins",
          "[esp32_hal]") {
    auto m = Esp32ChanMapper();

    // expect a new channel starting with 0 for different pins and the same
    // channel if a pin is used again
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

TEST_CASE("channel mapper starts over when channels are exhausted",
          "[esp32_hal]") {
    auto m = Esp32ChanMapper();

    for (auto i = 0; i < Esp32ChanMapper::kLedcMaxChan; i++) {
        REQUIRE(m.chanForPin(i) == i);
    }

    // now all channels are used and the mapper starts over at 0
    REQUIRE(m.chanForPin(100) == 0);
    REQUIRE(m.chanForPin(101) == 1);
}

TEST_CASE("ledc ctor correctly initializes hardware", "[esp32_hal]") {
    esp32_mock_init();

    constexpr auto kChan = 5;
    constexpr auto kPin = 10;
    auto hal[[gnu::unused]] = Esp32Hal(kPin, kChan);

    // check that ledc is initialized correctly
    auto timer_config = esp32_mock_get_ledc_timer_config_args();
    REQUIRE(timer_config.speed_mode == LEDC_LOW_SPEED_MODE);
    REQUIRE(timer_config.duty_resolution == LEDC_TIMER_8_BIT);
    REQUIRE(timer_config.timer_num == LEDC_TIMER_0);
    REQUIRE(timer_config.freq_hz == 5000);
    REQUIRE(timer_config.clk_cfg == LEDC_AUTO_CLK);

    auto chan_config = esp32_mock_get_ledc_channel_config_args();
    REQUIRE(chan_config.gpio_num == kPin);
    REQUIRE(chan_config.speed_mode == LEDC_LOW_SPEED_MODE);
    REQUIRE(chan_config.channel == kChan);
    REQUIRE(chan_config.intr_type == LEDC_INTR_DISABLE);
    REQUIRE(chan_config.timer_sel == LEDC_TIMER_0);
    REQUIRE(chan_config.hpoint == 0);
    REQUIRE(chan_config.duty == 0);
    REQUIRE(chan_config.flags.output_invert == 0);
}

TEST_CASE("ledc selects same channel for same pin", "[esp32_hal]") {
    constexpr auto kPin = 10;

    // note: we test a static property here (auto incremented next channel
    // number). so test has side effects. TODO avoid/reset
    auto hal1 = Esp32Hal(kPin);
    auto hal2 = Esp32Hal(kPin);

    // same channel is to be selected, since pin did not change
    REQUIRE(hal1.chan() == hal2.chan());
}

TEST_CASE("ledc selects different channels for different pins", "[esp32_hal]") {
    constexpr auto kPin = 10;

    auto hal1 = Esp32Hal(kPin);
    auto hal2 = Esp32Hal(kPin + 1);

    REQUIRE(hal1.chan() != hal2.chan());
}

TEST_CASE("analogWrite() writes value (8-bit)", "[esp32_hal]") {
    esp32_mock_init();

    constexpr auto kChan = 5;
    constexpr auto kPin = 10;
    auto hal = Esp32Hal(kPin, kChan);

    hal.analogWrite<uint8_t>(123);

    auto set_duty = esp32_mock_get_ledc_set_duty_args((ledc_channel_t)kChan);
    REQUIRE(set_duty.speed_mode == LEDC_LOW_SPEED_MODE);
    REQUIRE(set_duty.duty == 123);

    auto update_duty =
        esp32_mock_get_ledc_update_duty_args((ledc_channel_t)kChan);
    REQUIRE(update_duty.speed_mode == LEDC_LOW_SPEED_MODE);
}

TEST_CASE("analogWrite() writes 0 as 0 (8-bit)", "[esp32_hal]") {
    esp32_mock_init();

    // attach channel 2 to pin 1
    constexpr auto kChan = 5;
    constexpr auto kPin = 10;
    auto hal = Esp32Hal(kPin, kChan);

    hal.analogWrite<uint8_t>(0);

    auto set_duty = esp32_mock_get_ledc_set_duty_args((ledc_channel_t)kChan);
    REQUIRE(set_duty.duty == 0);
}

TEST_CASE("analogWrite() writes 255 as 256 (8-bit)", "[esp32_hal]") {
    esp32_mock_init();

    constexpr auto kChan = 5;
    constexpr auto kPin = 10;
    auto hal = Esp32Hal(kPin, kChan);

    hal.analogWrite<uint8_t>(255);

    auto set_duty = esp32_mock_get_ledc_set_duty_args((ledc_channel_t)kChan);
    REQUIRE(set_duty.duty == 256);
}

TEST_CASE("analogWrite() writes 16-bit values correctly (16-bit)", "[esp32_hal]") {
    esp32_mock_init();

    constexpr auto kChan = 5;
    constexpr auto kPin = 10;
    auto hal = Esp32Hal(kPin, kChan);

    // Test downscaling from 16-bit to 8-bit
    hal.analogWrite<uint16_t>(0);
    auto set_duty1 = esp32_mock_get_ledc_set_duty_args((ledc_channel_t)kChan);
    REQUIRE(set_duty1.duty == 0);

    hal.analogWrite<uint16_t>(65535);  // max 16-bit
    auto set_duty2 = esp32_mock_get_ledc_set_duty_args((ledc_channel_t)kChan);
    REQUIRE(set_duty2.duty == 256);  // max for 8-bit HAL (255 becomes 256 per ESP32 spec)

    hal.analogWrite<uint16_t>(32768);  // mid 16-bit
    auto set_duty3 = esp32_mock_get_ledc_set_duty_args((ledc_channel_t)kChan);
    REQUIRE(set_duty3.duty == 128);  // mid 8-bit
}

TEST_CASE("millis() returns correct time", "[esp32_hal]") {
    esp32_mock_set_esp_timer(0);
    REQUIRE(jled::Esp32Clock::millis() == 0);

    esp32_mock_set_esp_timer(99 * 1000);
    REQUIRE(jled::Esp32Clock::millis() == 99);
}
