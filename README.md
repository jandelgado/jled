# JLed - Extended LED Library

[![Build Status](https://travis-ci.org/jandelgado/jled.svg?branch=master)](https://travis-ci.org/jandelgado/jled)
[![Coverage Status](https://coveralls.io/repos/github/jandelgado/jled/badge.svg?branch=master&dummy=1)](https://coveralls.io/github/jandelgado/jled?branch=master)

An Arduino library to control LEDs. It uses a **non-blocking** approach and can
control LEDs in simple (**on**/**off**) and complex (**blinking**,
**breathing**) ways in a **time-driven** manner.

JLed got some [coverage on Hackaday](https://hackaday.com/2018/06/13/simplifying-basic-led-effects/)
and someone did a [video tutorial for JLed](https://youtu.be/x5V2vdpZq1w)  - Thanks! 

[![breathing, blinking, fadeon and -off at the same time](doc/jled.gif)](examples/multiled)

```c++
// blink and breathe two LEDs (builtin and gpio 9) for 12 seconds.
#include <jled.h>

JLed led_breathe = JLed(9).Breathe(1500).Repeat(6).DelayAfter(500);
JLed led_blink = JLed(LED_BUILTIN).Blink(500, 500).Repeat(11).DelayBefore(1000);

void setup() { }

void loop() {
  led_blink.Update();
  led_breathe.Update();
}
```

## Contents

<!-- vim-markdown-toc GFM -->

* [Features](#features)
* [Installation](#installation)
    * [Arduino IDE](#arduino-ide)
    * [PlatformIO](#platformio)
* [Usage](#usage)
    * [Static on](#static-on)
        * [Static on example](#static-on-example)
    * [Static off](#static-off)
    * [Blinking](#blinking)
        * [Blinking example](#blinking-example)
    * [Breathing](#breathing)
        * [Breathing example](#breathing-example)
    * [FadeOn](#fadeon)
        * [FadeOn example](#fadeon-example)
    * [FadeOff](#fadeoff)
    * [User provided brightness function](#user-provided-brightness-function)
        * [User provided brightness function example](#user-provided-brightness-function-example)
    * [Immediate Stop](#immediate-stop)
* [Parameter overview](#parameter-overview)
* [Platform notes](#platform-notes)
    * [ESP8266](#esp8266)
    * [ESP32](#esp32)
* [Example sketches](#example-sketches)
    * [PlatformIO](#platformio-1)
    * [Arduino IDE](#arduino-ide-1)
* [Unit tests](#unit-tests)
* [Contributing](#contributing)
* [Author](#author)
* [License](#license)

<!-- vim-markdown-toc -->

## Features

* non-blocking
* simple on/off
* breathe effect
* blinking effect
* fade-on and -off effect
* user provided effects
* supports reversed polarity of LED
* easy configuration using fluent interface
* Arduino, ESP8266 and ESP32 platform compatible

## Installation

### Arduino IDE

In the main menu of the Arduino IDE, select `Sketch` > `Include Library` >
`Manage Libraries...` and search for `jled`, then press `install`.

### PlatformIO

Add `jled` to your library dependencies in your `platformio.ini` project file,
e.g.

```ini
...
[env:nanoatmega328]
platform = atmelavr
board = nanoatmega328
framework = arduino
lib_deps=jled
...
```

## Usage

First the LED object is constructed and configured, then the state is updated
with subsequent calls to the `Update()` method, typically from the `loop()`
function. While the effect is active, `Update` returns `true`, otherwise
false.

The constructor takes the pin, to which the LED is connected to as
only parameter. Further configuration of the LED object is done using a fluent
interface, e.g. `JLed led = JLed(13).Breathe(2000).DelayAfter(1000).Repeat(5)`.
See examples and [Parameter overview](#parameter-oveview) section below for
further details.

### Static on

Calling `On()` turns the LED on on after an optional time, specified by
`DelayBefore()`, has elapsed. To immediately turn a LED on, make a call
like `JLed(LED_BUILTIN).On().Update()`.

#### Static on example

```c++
#include <jled.h>

// turn builtin LED on after 1 second.
JLed led = JLed(LED_BUILTIN).On().DelayBefore(1000);

void setup() { }

void loop() {
  led.Update();
}
```

### Static off

`Off()` works like `On()`, except that it turns the LED off.

### Blinking

In blinking mode, the LED cycles through a given number of on-off cycles.

#### Blinking example

```c++
#include <jled.h>

// blink internal LED every second.
JLed led = JLed(LED_BUILTIN).Blink(1000, 1000).Forever();

void setup() { }

void loop() {
  led.Update();
}
```

### Breathing

In breathing mode, the LED smoothly changes brightness using PWM.

#### Breathing example

```c++
#include <jled.h>

// connect LED to pin 13 (PWM capable). LED will breathe with period of
// 2000ms and a delay of 1000ms after each period.
JLed led = JLed(13).Breathe(2000).DelayAfter(1000).Forever();

void setup() { }

void loop() {
  led.Update();
}
```

### FadeOn

In FadeOn mode, the LED is smoothly faded on to 100% brightness using PWM.

#### FadeOn example

```c++
#include <jled.h>

// LED is connected to pin 9 (PWM capable) gpio
JLed led = JLed(9).FadeOn(1000).DelayBefore(2000);

void setup() { }

void loop() {
  led.Update();
}
```

### FadeOff

In FadeOff mode, the LED is smoothly faded off using PWM. The fade starts
at 100% brightness. Internally it is implemented as a mirrored version of 
the FadeOn function, i.e. FadeOn(t) = FadeOff(period-t)

### User provided brightness function

It is also possible to provide a user defined brightness function. The
signature of such a function is `unit8_t func(unit32_t t, uint16_t period, uintptr_t param)`.
The function must return the brightness in range 0..255 in dependence of the
current time t.

#### User provided brightness function example

The example uses a user provided function to calculate the brightness.

```c++
#include <jled.h>

// this function returns changes between 0 and 255 and vice versa every 250 ms.
uint8_t blinkFunc(uint32_t t, uint16_t /*period*/, uintptr_t /*param*/) {
  return 255*((t/250)%2);
}

// Run blinkUserFunc for 5000ms
JLed led = JLed(LED_BUILTIN).UserFunc(blinkFunc, 5000);

void setup() {
}

void loop() {
  led.Update();
}
```

### Immediate Stop

Call `Stop()` to immediately turn the LED off and stop any running effects.

## Parameter overview

The following table shows the applicability of the various parameters in
dependence of the chosen effect:

| Method         | Description                                      | Default | On  | Off | Blink | Breath | FadeOn | FadeOff | UserFunc |
|----------------|--------------------------------------------------|---------|:---:|:---:|:-----:|:------:|:------:|:-------:|:--------:|
| DelayBefore(t) | time to wait before state is initially changed   | 0       | Yes | Yes | Yes   | Yes    | Yes    | Yes     | Yes      |
| DelayAfter(t)  | time to wait after each period                   | 0       |     |     |       | Yes    | Yes    | Yes     | Yes      |
| Repeat(n)      | repeat effect for given number of periods        | 1       |     |     | Yes   | Yes    | Yes    | Yes     | Yes      |
| Forever()      | repeat infinitely                                | false   |     |     | Yes   | Yes    | Yes    | Yes     | Yes      |
| LowActive()    | set output to be low-active (i.e. invert output) | false   | Yes | Yes | Yes   | Yes    | Yes    | Yes     | Yes      |

* all times are specified in milliseconds
* time specified by `DelayBefore()` is relative to first invocation of 
  `Update()`

## Platform notes

### ESP8266

The DAC of the ESP8266 operates with 10 bits, every value JLed writes out gets
automatically scaled to 10 bits, since JLed internally only uses 8 bits.  The
scaling methods makes sure that min/max relationships are preserved, i.e. 0 is
mapped to 0 and 255 is mapped to 1023. When using a user defined brightness
function on the ESP8266, 8 bit values must be returned, all scaling is done by
JLed transparently for the application, yielding platform independent code.

### ESP32

The ESP32 Arduino SDK does not provide an `analogWrite()` function. To
be able to use PWM, we use the `ledc` functions of the ESP32 SDK. 
(See [esspressif documentation](https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/peripherals/ledc.html) for details).

The `ledc` API connects so called channels to GPIO pins, enabling them to use
PWM. There are 16 channels available. Unless otherwise specified, JLed
automatically picks the next free channel, starting with channel 0 and wrapping
over after channel 15. To manually specify a channel, the JLed object must be
constructed this way:

```
JLed esp32Led = JLed(Esp32AnalogWriter(2, 7)).Blink(1000, 1000).Forever();
```

The `Esp32AnalogWriter(pin, chan)` constructor takes the pin number as the
first argument and the channel number on second position. Note that using the
above mentioned constructor yields non-platform independent code.

See [ESP32 multi led example](examples/multiled_esp32).

## Example sketches

Examples sketches are provided in the [examples](examples/) directory. 

### PlatformIO

To build an example using [the PlatformIO ide](http://platformio.org/), simply
uncomment the example to be built in the [platformio.ini](platformio.ini)
project file, e.g.:

```ini
...
[platformio]
; uncomment example to build
src_dir = examples/hello
;src_dir = examples/breathe
...
```

### Arduino IDE

To build an example sketch in the Arduino IDE, simply select an example from
the `File` > `Examples` > `JLed` menu.

## Unit tests

Jled comes with an exhaustive host based unit test suite. Info on how to run
the host based provided unit tests [is provided here](test/README.md).

## Contributing

* fork this repository
* create your feature branch
* add code
* add unit test(s) 
* add documentation
* make sure linter does not report problems (make lint)
* commit changes
* submit a PR

## Author

Jan Delgado, jdelgado[at]gmx.net.

## License

[MIT](LICENSE)

