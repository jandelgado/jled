// JLed unit tests (run on host).
// Verifies the compile-time HAL resolution selection for ESP8266 in jled.h.
// Copyright 2017-2026 Jan Delgado jdelgado@gmx.net

// Simulate the Arduino ESP8266 core v3+ environment before including jled.h, so
// the platform-detection macros in jled.h select the ESP8266 branch. Core v3+
// reverted analogWrite() to an 8-bit default but provides analogWriteResolution(),
// which the HAL uses to restore 10-bit output.
#define ARDUINO 100
#define ESP8266 1
#define HAS_ESP8266_VERSION_NUMERIC 1
#define ARDUINO_ESP8266_VERSION_MAJOR 3

#include <type_traits>
#include <jled.h>  // NOLINT
#include "catch2/catch_amalgamated.hpp"

// On ESP8266 core v3+ JLed keeps 8-bit resolution (backwards compatible with the
// 8-bit analogWrite default), while JLedHD restores full 10-bit resolution via
// analogWriteResolution(). Before the fix, JLedHD wrongly fell back to 8-bit here.
TEST_CASE("ESP8266 core v3+: JLed maps to 8-bit, JLedHD to 10-bit HAL",
          "[jled][esp8266]") {
    // Compile-time: a wrong HAL selection fails the build, not just the run.
    static_assert(std::is_same<jled::JLedHal, jled::ArduinoHal<8>>::value,
                  "JLed must use ArduinoHal<8> on ESP8266 core v3+");
    static_assert(std::is_same<jled::JLedHalHD, jled::ArduinoHal<10>>::value,
                  "JLedHD must use ArduinoHal<10> on ESP8266 core v3+");
    SUCCEED("ESP8266 core v3+ HAL selection verified at compile time");
}
