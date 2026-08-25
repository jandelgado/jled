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

#include "color_palette.h"    // NOLINT
#include "invertable_hal.h"   // NOLINT
#include "jled_base.h"        // NOLINT
#include "jled_group_base.h"  // NOLINT
#include "jled_rgb.h"         // NOLINT
#include "jled_std.h"         // NOLINT
#include "value_rgb.h"        // NOLINT
#include "writer_hal.h"       // NOLINT

// Raspberry Pi Pico
//
// PICO_SDK_VERSION_MAJOR is also defined by Earle Philhower's arduino-pico SDK
// (which wraps the Pico SDK).
//
// Define JLED_FORCE_ARDUINO_HAL (e.g. via platformio.ini build_flags) to bypass
// the native Pico/ESP32 HALs and use the standard ArduinoHal on those platforms.
#if defined(PICO_SDK_VERSION_MAJOR) && !defined(JLED_FORCE_ARDUINO_HAL)
#include "pico_hal.h"  // NOLINT
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
using JLedHal = InvertableHal<MbedHal<8>>;
using JLedHalHD = InvertableHal<MbedHal<16>>;
using JLedClockType = MbedClock;
}  // namespace jled

// ESP32
//
#elif defined(ESP32) && !defined(JLED_FORCE_ARDUINO_HAL)
#include "esp32_hal.h"  // NOLINT
namespace jled {
#if JLED_ESP32_HAS_LEDC_OUTPUT_INVERT
using JLedHal = Esp32Hal<8, LEDC_TIMER_0>;
using JLedHalHD = Esp32Hal<13, LEDC_TIMER_1>;
#else
// Pre-4.4 ESP-IDF has no documented hardware invert for LEDC (see
// esp32_hal.h); invert in software instead, same as Arduino/ESP8266/mbed.
using JLedHal = InvertableHal<Esp32Hal<8, LEDC_TIMER_0>>;
using JLedHalHD = InvertableHal<Esp32Hal<13, LEDC_TIMER_1>>;
#endif
using JLedClockType = Esp32Clock;
}  // namespace jled

// STM32Cube (native HAL), see stm32cube_hal.h
//
// "USE_HAL_DRIVER" #define marks a build against ST's STM32Cube HAL SDK (CubeMX projects
// and PlatformIO framework = stm32cube). STM32duino also defines it but always
// defines ARDUINO too. Excluding ARDUINO keeps STM32duino on its existing
// InvertableHal<ArduinoHal<...>> path in the #else block below.
#elif defined(USE_HAL_DRIVER) && !defined(ARDUINO)
#include "stm32cube_hal.h"  // NOLINT
namespace jled {
// One Stm32CubeHal type serves both 8- and 16-bit brightness: it has no
// resolution template parameter, scaling duty to the user's timer period.
using JLedHal = Stm32CubeHal;
using JLedHalHD = Stm32CubeHal;
using JLedClockType = Stm32CubeClock;
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
using JLedHal = InvertableHal<ArduinoHal<10>>;
#elif defined(ARDUINO_ARCH_STM32)
// STM32duino forbids touching hardware from a global constructor; defer
// pinMode()/analogWriteResolution() to the first analogWrite() call instead.
// Using 12 bits as the default for JLedHal allows mixing JLed and JLedHD in the code.
using JLedHal = InvertableHal<ArduinoHal<12, /* kLazyInit_ = */ true>>;
#else
using JLedHal = InvertableHal<ArduinoHal<8>>;
#endif
// JLedHD uses the best practical PWM resolution per platform.
// Bit-depth is chosen to keep PWM frequency well above the visible flicker
// threshold (~100 Hz) while maximising smoothness:
//   - Platforms where frequency and resolution are independent (Teensy): 16-bit
//   - Platforms where they are coupled (SAMD21, Arduino Due, STM32, nRF5): 12-bit (~4-12 kHz)
//   - RP2040 (Earle Philhower arduino-pico SDK): 16-bit
//   - ESP8266 (all core versions): 10-bit. Core v1/v2 are natively 10-bit; on
//     Core v3+ (which defaults to 8-bit) the 10-bit range is restored via
//     analogWriteResolution(), so JLedHD keeps full 10-bit resolution there.
//   - All other Arduino-compatible platforms: 8-bit
#if defined(ARDUINO_ARCH_RP2040)  // only hit if also JLED_FORCE_ARDUINO_HAL is set
using JLedHalHD = InvertableHal<ArduinoHal<16>>;
#elif defined(__IMXRT1062__) || defined(KINETISK) || defined(KINETISL)  // Teensy 4.x/3.x/LC
using JLedHalHD = InvertableHal<ArduinoHal<16>>;  // frequency/resolution are independent on Teensy
#elif defined(__SAMD21__) || defined(ARDUINO_ARCH_SAM)  // SAMD21 (Zero, MKR), Arduino Due
using JLedHalHD = InvertableHal<ArduinoHal<12>>;  // 12-bit -> ~11.7 kHz on 48/84 MHz GCLK
#elif defined(ARDUINO_ARCH_STM32)
// 12-bit avoids STM32duino prescaler issues; kLazyInit_=true, see JLedHal above
using JLedHalHD = InvertableHal<ArduinoHal<12, true>>;
#elif defined(ARDUINO_ARCH_NRF5)
using JLedHalHD = InvertableHal<ArduinoHal<12>>;  // 16-bit -> ~244 Hz on nRF52; 12-bit -> ~3.9 kHz
#elif defined(ESP8266)
using JLedHalHD = InvertableHal<ArduinoHal<10>>;  // 10-bit on all core versions (see note above)
#else
using JLedHalHD = InvertableHal<ArduinoHal<8>>;
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

// JLedGroup/JLedHDGroup: homogeneous groups, elements stored by value, constructed inline.
// A user-defined LED type needs no wrapper, just a matching alias:
//   using MyLedGroup = TJLedGroup<JLedClockType, MyLed>;
using JLedGroup = TJLedGroup<JLedClockType, JLed>;
using JLedHDGroup = TJLedGroup<JLedClockType, JLedHD>;

// JLedRef: non-owning reference to an externally-managed JLed, JLedHD, or JLedGroup.
// JLedRefGroup: heterogeneous groups, elements referenced by pointer. Use it for mixed
// resolutions, nested groups, user-defined types, or LEDs already declared elsewhere.
// The LED objects must outlive the JLedRef / JLedRefGroup that references them.
using JLedRef = TJLedRef;
using JLedRefGroup = TJLedGroup<JLedClockType, JLedRef>;

// JLedRGB: RGBColor<uint8_t> brightness control for a three-pin RGB LED,
// over the platform's standard-resolution HAL.
class JLedRGB : public TJLedRGB<JLedHal, JLedClockType, RGBColor<uint8_t>, JLedRGB> {
    using Base = TJLedRGB<JLedHal, JLedClockType, RGBColor<uint8_t>, JLedRGB>;

 public:
    using Base::Base;
};

// JLedRGBGroup: homogeneous, by-value group of JLedRGB, mirroring
// JLedGroup/JLedHDGroup
using JLedRGBGroup = TJLedGroup<JLedClockType, JLedRGB>;

// JLedRGBHD: the RGBColor<uint16_t> counterpart of JLedRGB, over the
// platform's high-resolution HAL.
class JLedRGBHD : public TJLedRGB<JLedHalHD, JLedClockType, RGBColor<uint16_t>, JLedRGBHD> {
    using Base = TJLedRGB<JLedHalHD, JLedClockType, RGBColor<uint16_t>, JLedRGBHD>;

 public:
    using Base::Base;
};

// JLedRGBHDGroup: the JLedRGBHD counterpart of JLedRGBGroup, exactly as
// JLedHDGroup is to JLedGroup.
using JLedRGBHDGroup = TJLedGroup<JLedClockType, JLedRGBHD>;

};  // namespace jled

using JLed = jled::JLed;
using JLedHD = jled::JLedHD;
using JLedGroup = jled::JLedGroup;
using JLedHDGroup = jled::JLedHDGroup;
using JLedRef = jled::JLedRef;
using JLedRefGroup = jled::JLedRefGroup;
using JLedRGB = jled::JLedRGB;
using JLedRGBGroup = jled::JLedRGBGroup;
using JLedRGBHD = jled::JLedRGBHD;
using JLedRGBHDGroup = jled::JLedRGBHDGroup;

// WriterHal: HAL that writes via a user-provided Writer to an external
// target (e.g. FastLED's CRGB[]).
template<typename Value, typename Target, typename Writer>
using WriterHal = jled::InvertableHal<jled::WriterHal<Value, Target, Writer>>;
