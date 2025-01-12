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

// native pico SDK
#if defined(PICO_SDK_VERSION_MAJOR) && !defined(ARDUINO_API_VERSION)
#include "pico_hal.h"   // NOLINT
namespace jled {using JLedHalType = PicoHal; using JLedClockType = PicoClock;}

// native MBED SDK
#elif defined(__MBED__) && !defined(ARDUINO_API_VERSION)
#include "mbed_hal.h"  // NOLINT
namespace jled {using JLedHalType = MbedHal; using JLedClockType = MbedClock;}

#elif defined(ESP32)
#include "esp32_hal.h"  // NOLINT
namespace jled {using JLedHalType = Esp32Hal; using JLedClockType = Esp32Clock;}

#else
#include "arduino_hal.h"  // NOLINT
namespace jled {using JLedHalType = ArduinoHal; using JLedClockType = ArduinoClock;}
#endif

namespace jled {
class JLed : public TJLed<JLedHalType, JLedClockType, JLed> {
    using TJLed<JLedHalType, JLedClockType, JLed>::TJLed;
};

// a group of JLed objects which can be controlled simultanously
class JLedSequence : public TJLedSequence<JLed, JLedClockType, JLedSequence> {
    using TJLedSequence<JLed, JLedClockType, JLedSequence>::TJLedSequence;
};

};  // namespace jled

using JLed = jled::JLed;
using JLedSequence = jled::JLedSequence;

#endif  // SRC_JLED_H_
