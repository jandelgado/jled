# JLed - Extended LED Library

[![Build Status](https://travis-ci.org/jandelgado/jled.svg?branch=master)](https://travis-ci.org/jandelgado/jled)

An Arduino library to control LEDs. It uses a **non-blocking** approach and can
control LEDs in simple (**on**/**off**) and complex (**blinking**,
**breathing**) ways in a **time-driven** manner.

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
* [Usage](#usage)
    * [Static on](#static-on)
        * [Static on example](#static-on-example)
    * [Static off](#static-off)
    * [Blinking](#blinking)
        * [Blinking example](#blinking-example)
    * [Breathing](#breathing)
        * [Breathing example](#breathing-example)
    * [Immediate Stop](#immediate-stop)
* [Parameter overview](#parameter-overview)
* [Memory footprint](#memory-footprint)
* [Author](#author)
* [License](#license)

<!-- vim-markdown-toc -->

## Features

* non-blocking
* simple on/off
* breathe effect
* blinking effect
* supports reversed polarity
* easy configuration using fluent interface

## Usage

First the LED object is constructed and configured, then the state is updated
with subsequent calls to the `Update()` method, typically from the `loop()`
function. The constructor takes the pin, to which the LED is connected to as
only parameter. Further configuration of the LED object is done using a fluent
interface, e.g. `JLed led = JLed(13).Breathe(2000).DelayAfter(1000).Repeat(5)`.
See examples and [Parameter overview](#parameter-oveview) section below for
further details.

### Static on

Calling `On()` turns the LED on on after the an optional time, specified by
`DelayBefore()`, has elapsed. To immediately turn a LED on, you can make a call
like `JLed(LED_BUILTIN).On().Update()`.

#### Static on example

```c++
#include <jled.h>

// turn builtin LED on after 1 second.
JLed led = JLed(LED_BUILTIN).On().DelayBefore(1000);

void setup() {
}

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

### Immediate Stop

Call `Stop()` to immediately turn the LED off and stop any running effects.

## Parameter overview

The following table shows the applicability of the various parameters in
dependence of the operating mode (on, off, blink, breath).

| Method         | Description                                    | Default | On  | Off | Blink | Breath |
|----------------|------------------------------------------------|---------|:---:|:---:|:-----:|:------:|
| DelayBefore(t) | time to wait before state is initially changed | 0       | Yes | Yes | Yes   | Yes    |
| DelayAfter(t)  | time to wait after each period                 | 0       |     |     | Yes   | Yes    |
| Repeat(n)      | repeat for given number of periods             | 1       |     |     | Yes   | Yes    |
| Forever()      | repeat infinitely                              | false   |     |     | Yes   | Yes    |
| LowActive()    | set output to be low-active                    | false   | Yes | Yes | Yes   | Yes    |

* all times are specified in milliseconds
* time specified by `DelayBefore()` is relative to first invocation of 
  `Update()`

## Memory footprint

An instance of the JLed class consumes 21 bytes of memory (nanoatmega328 target).

## Author

Jan Delgado, jdelgado[at]gmx.net.

## License

[MIT](LICENSE)

