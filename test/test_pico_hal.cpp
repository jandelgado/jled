// JLed Unit tests for the pico_hal class (run on host).
// Copyright 2024-2025 Jan Delgado jdelgado@gmx.net
#include "catch2/catch_amalgamated.hpp"

#include <pico_hal.h>   // NOLINT
#include "pico_mock.h"  // NOLINT

using jled::PicoHal;

// pin=10 → slice=5, channel=0 (PWM_CHAN_A)
static constexpr auto kPin   = 10u;
static constexpr auto kSlice = kPin / 2;  // 5
static constexpr auto kChan  = kPin % 2;  // 0

struct PicoMockFixture {
    PicoState mock{};
    PicoMockFixture()  { picoMockSetInstance(&mock); }
    ~PicoMockFixture() { picoMockSetInstance(nullptr); }
};

TEST_CASE_METHOD(PicoMockFixture, "PicoHal<> constructor", "[pico_hal]") {
    PicoHal<> hal(kPin);
    SECTION("sets GPIO function to PWM") {
        REQUIRE(mock.getGpioFunction(kPin) == GPIO_FUNC_PWM);
    }
    SECTION("configures PWM wrap to 2^kResBits_-1") {
        REQUIRE(mock.getPwmWrap(kSlice) == 255);  // 2^8 - 1
    }
    SECTION("sets clock divider to 1.0") {
        REQUIRE(mock.getPwmDivInt(kSlice)  == 1);
        REQUIRE(mock.getPwmDivFrac(kSlice) == 0);
    }
    SECTION("enables PWM slice") {
        REQUIRE(mock.getPwmEnabled(kSlice) == true);
    }
}

TEST_CASE_METHOD(PicoMockFixture, "PicoHal<10> constructor", "[pico_hal]") {
    PicoHal<10> hal(kPin);
    SECTION("sets wrap to 1023") {
        REQUIRE(mock.getPwmWrap(kSlice) == 1023);  // 2^10 - 1
    }
}

TEST_CASE_METHOD(PicoMockFixture, "PicoHal<> analogWrite", "[pico_hal]") {
    PicoHal<> hal(kPin);
    SECTION("8-bit: zero and mid-range pass through unchanged") {
        hal.analogWrite<uint8_t>(0);
        REQUIRE(mock.getPwmChanLevel(kSlice, kChan) == 0);

        hal.analogWrite<uint8_t>(128);
        REQUIRE(mock.getPwmChanLevel(kSlice, kChan) == 128);
    }
    SECTION("8-bit: max brightness gets full-on fix (255->256)") {
        hal.analogWrite<uint8_t>(255);
        REQUIRE(mock.getPwmChanLevel(kSlice, kChan) == 256);
    }
    SECTION("16-bit: max brightness gets full-on fix") {
        // scaleToNative<8>(65535) = 65535>>8 = 255 = kWrap; fix applies: 255+1 = 256
        hal.analogWrite<uint16_t>(65535);
        REQUIRE(mock.getPwmChanLevel(kSlice, kChan) == 256);
    }
}

TEST_CASE_METHOD(PicoMockFixture, "PicoHal<10> analogWrite", "[pico_hal]") {
    PicoHal<10> hal(kPin);
    SECTION("8-bit input upscaled to 10-bit") {
        hal.analogWrite<uint8_t>(0);
        REQUIRE(mock.getPwmChanLevel(kSlice, kChan) == 0);

        // scaleToNative<10>(255u8) = (255<<2)|(255>>6) = 1020|3 = 1023 = kWrap; fix: 1023+1=1024
        hal.analogWrite<uint8_t>(255);
        REQUIRE(mock.getPwmChanLevel(kSlice, kChan) == 1024);
    }
    SECTION("16-bit input downscaled to 10-bit") {
        hal.analogWrite<uint16_t>(0);
        REQUIRE(mock.getPwmChanLevel(kSlice, kChan) == 0);

        // scaleToNative<10>(65535u16) = 65535>>6 = 1023 = kWrap; fix: 1023+1=1024
        hal.analogWrite<uint16_t>(65535);
        REQUIRE(mock.getPwmChanLevel(kSlice, kChan) == 1024);
    }
    SECTION("max 8-bit input gets full-on fix (1023->1024)") {
        // scaleToNative<10>(255) = (255<<2)|(255>>6) = 1020|3 = 1023 = kWrap; fix: 1023+1 = 1024
        hal.analogWrite<uint8_t>(255);
        REQUIRE(mock.getPwmChanLevel(kSlice, kChan) == 1024);
    }
}

TEST_CASE_METHOD(PicoMockFixture, "PicoHal<16> analogWrite", "[pico_hal]") {
    PicoHal<16> hal(kPin);
    SECTION("max brightness does not apply full-on fix") {
        // kWrap+1 = 65536 overflows uint16_t; fix is intentionally skipped
        hal.analogWrite<uint16_t>(65535);
        REQUIRE(mock.getPwmChanLevel(kSlice, kChan) == 65535);
    }
}

TEST_CASE_METHOD(PicoMockFixture, "PicoClock::millis()", "[pico_hal]") {
    SECTION("returns 0 at boot") {
        mock.setBootTimeUs(0);
        REQUIRE(jled::PicoClock::millis() == 0);
    }
    SECTION("converts microseconds to milliseconds") {
        mock.setBootTimeUs(99000);
        REQUIRE(jled::PicoClock::millis() == 99);
    }
}
