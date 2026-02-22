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
#ifndef SRC_JLED_H_
#define SRC_JLED_H_

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

// PICO_SDK_VERSION_MAJOR is also defined by Earle Philhower's arduino-pico SDK
// (which wraps the Pico SDK), so exclude ARDUINO_ARCH_RP2040 here to let it fall
// through to the Arduino branch below where ArduinoHal<12> is selected.
#if defined(PICO_SDK_VERSION_MAJOR)
// #if defined(PICO_SDK_VERSION_MAJOR) && !defined(ARDUINO_ARCH_RP2040)
#include "pico_hal.h"   // NOLINT
namespace jled {
using JLedHal = PicoHal<8>;
using JLedHal16 = PicoHal<16>;
using JLedClockType = PicoClock;
}  // namespace jled

#elif defined(__MBED__) && !defined(ARDUINO_API_VERSION)
#include "mbed_hal.h"  // NOLINT
namespace jled {
using JLedHal = MbedHal;
using JLedHal16 = MbedHal;
using JLedClockType = MbedClock;
}  // namespace jled

#elif defined(ESP32)
#include "esp32_hal.h"  // NOLINT
namespace jled {
using JLedHal = Esp32Hal;
using JLedHal16 = Esp32Hal;
using JLedClockType = Esp32Clock;
}  // namespace jled

#else
#include "arduino_hal.h"  // NOLINT
namespace jled {
// JLed (8-bit) always uses 8-bit HAL — no benefit in upscaling 8-bit values
using JLedHal = ArduinoHal<8>;
// JLed16 uses the platform's native PWM resolution for full dynamic range:
//   RP2040 (Earle Philhower arduino-pico SDK): 12-bit
//   ESP8266 Core v1/v2: 10-bit (Core v3+ reverted to 8-bit for compatibility)
//   All other Arduino-compatible platforms: 8-bit
#if defined(ARDUINO_ARCH_RP2040)
using JLedHal16 = ArduinoHal<12>;
#elif defined(ESP8266) && \
    !(defined(HAS_ESP8266_VERSION_NUMERIC) && ARDUINO_ESP8266_VERSION_MAJOR >= 3)
using JLedHal16 = ArduinoHal<10>;
#else
using JLedHal16 = ArduinoHal<8>;
#endif
using JLedClockType = ArduinoClock;
}  // namespace jled
#endif

namespace jled {
// JLed: 8-bit brightness control (backwards compatible)
class JLed : public TJLed<JLedHal, JLedClockType, uint8_t, JLed> {
    using TJLed<JLedHal, JLedClockType, uint8_t, JLed>::TJLed;
};

// JLed16: 16-bit brightness control for smoother effects on high-resolution MCUs
class JLed16 : public TJLed<JLedHal16, JLedClockType, uint16_t, JLed16> {
    using TJLed<JLedHal16, JLedClockType, uint16_t, JLed16>::TJLed;
};

// a group of JLed objects which can be controlled simultanously
class JLedSequence : public TJLedSequence<JLed, JLedClockType, JLedSequence> {
    using TJLedSequence<JLed, JLedClockType, JLedSequence>::TJLedSequence;
};

// a group of JLed16 objects which can be controlled simultanously
class JLedSequence16 : public TJLedSequence<JLed16, JLedClockType, JLedSequence16> {
    using TJLedSequence<JLed16, JLedClockType, JLedSequence16>::TJLedSequence;
};

};  // namespace jled

using JLed = jled::JLed;
using JLed16 = jled::JLed16;
using JLedSequence = jled::JLedSequence;
using JLedSequence16 = jled::JLedSequence16;

#endif  // SRC_JLED_H_
