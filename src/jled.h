// Copyright (c) 2017 Jan Delgado <jdelgado[at]gmx.net>
// https://github.com/jandelgado/jled
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//
#pragma once

// JLed - non-blocking LED abstraction library.
//
// Example Arduino sketch:
//   JLed led = JLed(LED_BUILTIN).Blink(500, 500).Repeat(10).DelayBefore(1000);
//
//   void setup() {}
//
//   void loop() {
//     led.Update();
//   }

#include "jled_base.h"  // NOLINT

// Raspberry Pi Pico
//
// PICO_SDK_VERSION_MAJOR is also defined by Earle Philhower's arduino-pico SDK
// (which wraps the Pico SDK).
//
// Define JLED_FORCE_ARDUINO_HAL (e.g. via platformio.ini build_flags) to bypass
// the native Pico/ESP32 HALs and use the standard ArduinoHal on those platforms.
#if defined(PICO_SDK_VERSION_MAJOR) && !defined(JLED_FORCE_ARDUINO_HAL)
#include "pico_hal.h"   // NOLINT
namespace jled {
using JLedHal = PicoHal<8>;
using JLedHalHD = PicoHal<16>;
using JLedClockType = PicoClock;
}  // namespace jled

// MBED Plattform
//
#elif defined(__MBED__) && !defined(ARDUINO_API_VERSION)
#include "mbed_hal.h"  // NOLINT
namespace jled {
using JLedHal = MbedHal<8>;
using JLedHalHD = MbedHal<16>;
using JLedClockType = MbedClock;
}  // namespace jled

// ESP32
//
#elif defined(ESP32) && !defined(JLED_FORCE_ARDUINO_HAL)
#include "esp32_hal.h"  // NOLINT
namespace jled {
using JLedHal = Esp32Hal<8, LEDC_TIMER_0>;
using JLedHalHD = Esp32Hal<13, LEDC_TIMER_1>;
using JLedClockType = Esp32Clock;
}  // namespace jled

#else
// Use standard Arduino HAL.
//
#include "arduino_hal.h"  // NOLINT
namespace jled {
// ESP8266 Core v1/v2: analogWrite() accepts 10-bit values (0–1023) natively.
// ESP8266 Core v3+: reverted to standard 8-bit (0–255) for compatibility.
// All other Arduino-compatible platforms: 8-bit.
#if defined(ESP8266) && \
    !(defined(HAS_ESP8266_VERSION_NUMERIC) && ARDUINO_ESP8266_VERSION_MAJOR >= 3)
using JLedHal = ArduinoHal<10>;
#else
using JLedHal = ArduinoHal<8>;
#endif
// JLedHD uses the best practical PWM resolution per platform.
// Bit-depth is chosen to keep PWM frequency well above the visible flicker
// threshold (~100 Hz) while maximising smoothness:
//   - Platforms where frequency and resolution are independent (Teensy): 16-bit
//   - Platforms where they are coupled (SAMD21, Arduino Due, STM32, nRF5): 12-bit (~4-12 kHz)
//   - RP2040 (Earle Philhower arduino-pico SDK): 16-bit
//   - ESP8266 Core v1/v2: 10-bit (Core v3+ reverted to 8-bit for compatibility)
//   - All other Arduino-compatible platforms: 8-bit
#if defined(ARDUINO_ARCH_RP2040)    // only hit if also JLED_FORCE_ARDUINO_HAL is set
using JLedHalHD = ArduinoHal<16>;
#elif defined(__IMXRT1062__) || defined(KINETISK) || defined(KINETISL)  // Teensy 4.x/3.x/LC
using JLedHalHD = ArduinoHal<16>;  // frequency/resolution are independent on Teensy
#elif defined(__SAMD21__) || defined(ARDUINO_ARCH_SAM)  // SAMD21 (Zero, MKR), Arduino Due
using JLedHalHD = ArduinoHal<12>;  // 12-bit -> ~11.7 kHz on 48 MHz GCLK (SAMD21) / 84 MHz (Due)
#elif defined(ARDUINO_ARCH_STM32)
using JLedHalHD = ArduinoHal<12>;  // 12-bit avoids timer prescaler issues in STM32duino
#elif defined(ARDUINO_ARCH_NRF5)
using JLedHalHD = ArduinoHal<12>;  // 16-bit -> ~244 Hz on nRF52; 12-bit -> ~3.9 kHz
#elif defined(ESP8266) && \
    !(defined(HAS_ESP8266_VERSION_NUMERIC) && ARDUINO_ESP8266_VERSION_MAJOR >= 3)
using JLedHalHD = ArduinoHal<10>;
#else
using JLedHalHD = ArduinoHal<8>;
#endif
using JLedClockType = ArduinoClock;
}  // namespace jled
#endif

namespace jled {
// JLed: 8-bit brightness control (backwards compatible)
class JLed : public TJLed<JLedHal, JLedClockType, uint8_t, JLed> {
    using TJLed<JLedHal, JLedClockType, uint8_t, JLed>::TJLed;
};

// JLedHD: high-definition brightness control for smoother effects on high-resolution MCUs
class JLedHD : public TJLed<JLedHalHD, JLedClockType, uint16_t, JLedHD> {
    using TJLed<JLedHalHD, JLedClockType, uint16_t, JLedHD>::TJLed;
};

// a group of JLed objects which can be controlled simultanously
class JLedSequence : public TJLedSequence<JLed, JLedClockType, JLedSequence> {
    using TJLedSequence<JLed, JLedClockType, JLedSequence>::TJLedSequence;
};

// a group of JLedHD objects which can be controlled simultanously
class JLedSequenceHD : public TJLedSequence<JLedHD, JLedClockType, JLedSequenceHD> {
    using TJLedSequence<JLedHD, JLedClockType, JLedSequenceHD>::TJLedSequence;
};

};  // namespace jled

using JLed = jled::JLed;
using JLedHD = jled::JLedHD;
using JLedSequence = jled::JLedSequence;
using JLedSequenceHD = jled::JLedSequenceHD;

