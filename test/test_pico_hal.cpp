// JLed Unit tests for the pico_hal class (run on host).
#include "catch2/catch_amalgamated.hpp"

#include <pico_hal.h>   // NOLINT
#include "pico_mock.h"  // NOLINT

using jled::PicoHal;

// pin=10 → slice=5, channel=0 (PWM_CHAN_A)
static constexpr auto kPin   = 10u;
static constexpr auto kSlice = kPin / 2;  // 5
static constexpr auto kChan  = kPin % 2;  // 0

TEST_CASE("constructor sets GPIO function to PWM", "[pico_hal]") {
    picoMockInit();
    PicoHal<> hal(kPin);
    REQUIRE(picoMockGetGpioFunction(kPin) == GPIO_FUNC_PWM);
}

TEST_CASE("constructor configures PWM wrap to 2^kResBits_-1", "[pico_hal]") {
    picoMockInit();
    PicoHal<> hal(kPin);
    REQUIRE(picoMockGetPwmWrap(kSlice) == 255);  // 2^8 - 1
}

TEST_CASE("PicoHal<10>: constructor sets wrap to 1023", "[pico_hal]") {
    picoMockInit();
    PicoHal<10> hal(kPin);
    REQUIRE(picoMockGetPwmWrap(kSlice) == 1023);  // 2^10 - 1
}

TEST_CASE("constructor sets clock divider to 1.0", "[pico_hal]") {
    picoMockInit();
    PicoHal<> hal(kPin);
    REQUIRE(picoMockGetPwmDivInt(kSlice)  == 1);
    REQUIRE(picoMockGetPwmDivFrac(kSlice) == 0);
}

TEST_CASE("constructor enables PWM slice", "[pico_hal]") {
    picoMockInit();
    PicoHal<> hal(kPin);
    REQUIRE(picoMockGetPwmEnabled(kSlice) == true);
}

TEST_CASE("analogWrite() 8-bit: duty passes through directly", "[pico_hal]") {
    picoMockInit();
    PicoHal<> hal(kPin);

    hal.analogWrite<uint8_t>(0);
    REQUIRE(picoMockGetPwmChanLevel(kSlice, kChan) == 0);

    hal.analogWrite<uint8_t>(128);
    REQUIRE(picoMockGetPwmChanLevel(kSlice, kChan) == 128);

    hal.analogWrite<uint8_t>(255);
    REQUIRE(picoMockGetPwmChanLevel(kSlice, kChan) == 255);
}

TEST_CASE("PicoHal<10>: 8-bit input upscaled to 10-bit", "[pico_hal]") {
    picoMockInit();
    PicoHal<10> hal(kPin);

    hal.analogWrite<uint8_t>(0);
    REQUIRE(picoMockGetPwmChanLevel(kSlice, kChan) == 0);

    hal.analogWrite<uint8_t>(255);
    // scaleToNative<10>(255u8) = (255<<2)|(255>>6) = 1020|3 = 1023
    REQUIRE(picoMockGetPwmChanLevel(kSlice, kChan) == 1023);
}

TEST_CASE("PicoHal<10>: 16-bit input downscaled to 10-bit", "[pico_hal]") {
    picoMockInit();
    PicoHal<10> hal(kPin);

    hal.analogWrite<uint16_t>(0);
    REQUIRE(picoMockGetPwmChanLevel(kSlice, kChan) == 0);

    hal.analogWrite<uint16_t>(65535);
    // scaleToNative<10>(65535u16) = 65535 >> 6 = 1023
    REQUIRE(picoMockGetPwmChanLevel(kSlice, kChan) == 1023);
}

TEST_CASE("PicoClock::millis() returns 0 at boot", "[pico_hal]") {
    picoMockInit();
    picoMockSetBootTimeUs(0);
    REQUIRE(jled::PicoClock::millis() == 0);
}

TEST_CASE("PicoClock::millis() converts microseconds to milliseconds",
          "[pico_hal]") {
    picoMockInit();
    picoMockSetBootTimeUs(99000);
    REQUIRE(jled::PicoClock::millis() == 99);
}
