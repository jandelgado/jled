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

#ifdef PICO_SDK_VERSION_MAJOR
#include "pico_hal.h"   // NOLINT
namespace jled {using JLedHalType = PicoHal;}
#elif defined(__MBED__) && !defined(ARDUINO_API_VERSION)
#include "mbed_hal.h"  // NOLINT
namespace jled {using JLedHalType = MbedHal;}
#elif ESP32
#include "esp32_hal.h"  // NOLINT
namespace jled {using JLedHalType = Esp32Hal;}
#elif ESP8266
#include "esp8266_hal.h"  // NOLINT
namespace jled {using JLedHalType = Esp8266Hal;}
#else
#include "arduino_hal.h"  // NOLINT
namespace jled {using JLedHalType = ArduinoHal;}
#endif

namespace jled {
class JLed : public TJLed<JLedHalType, JLed> {
    using TJLed<JLedHalType, JLed>::TJLed;
};

// a group of JLed objects which can be controlled simultanously
class JLedSequence : public TJLedSequence<JLed> {
    using TJLedSequence<JLed>::TJLedSequence;
};

};  // namespace jled

using JLed = jled::JLed;
using JLedSequence = jled::JLedSequence;

#endif  // SRC_JLED_H_
