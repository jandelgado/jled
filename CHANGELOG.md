# JLed changelog (github.com/jandelgado/jled)

## [2021-02-02] 4.7.0

* new: support for Raspberry Pi Pico added

## [2021-02-02] 4.6.1

* fix: `Forever()` on sequence had no effect (#68)

## [2021-01-24] 4.6.0

* new: JLedSequence can be configured to play the sequence multiple times
       using the `Repeat()` and `Forever()` methods
* drop travis-ci, use github actions

## [2020-10-24] 4.5.2

* fix: ESP32 led glimming when using low-active connection (#60)

## [2020-07-01] 4.5.1

* fix: support for Nano 33 BLE (#53)

## [2020-02-23] 4.5.0

* new: `JLed::MaxBrightness(uint8_t level)` method to limit output of effects
  (implements #43).

## [2020-02-21] 4.4.0

* JLed now supports the mbed framework. See README.md and `examples/multiled_mbed`
  for examples.

## [2019-09-21] 4.3.0

* new example: [custom HAL](examples/custom_hal/custom_hal.ino) showing
  how to implment a custom HAL.

## [2019-08-30] 4.2.1

* fix: make sure memory alignment is correct (caused hard fault on 
  SAMD21). Fixes #27.

## [2019-06-20] 4.2.0

* changing an effect resets the Jled object so it starts over with the 
  new effect (see #25). Prior to this change, calling `Reset()` was
  necessary.

## [2019-05-11] 4.1.2

* fix: ESP32 dynamic channel assignment fixed. Sequence demo now working
       as expected (see #22).

## [2019-05-07] 4.1.1

* fix: version format in library.properties (removed leading `v`; see #21)

## [2019-03-10] v4.1.0

* change: clean up interface and simplify code: `On()` no longer takes an
  optional brightness argument. Call `Set(uint8_t brightness)` instead.
* documentation update

## [2019-03-10] v4.0.0

In addition to the changes introduced with `v4.0.0-rc0` and `v4.0.0-rc1`, the
`v4.0.0` relases adds/changes the following:

### Added

* new `Candle()` effect added for candles and fire like effects

### Changed

* The user provided brightness class no longer needs a `Clone()` method. See
  [example](examples/user_func/user_func.ino) for an example

### Fixed

* Makefile (unit testing) dependency checking fixed

## [2019-02-17] v4.0.0-rc1

### Changed

* fix: byte buffer alignment for ESP8266 set to DWORD boundary making ESP8266
  run again with JLed 4.x
* arduino HAL now does lazy call to pinMode() to prevent STM32 problems
* simplified morse example code

## [2019-01-23] v4.0.0-rc0

### Added

* `JLed::Reset()` - resets the led to it's initial state allowing to 
  to start over
* `JLed::IsRunning()` - return true if effect is active, else false
* new class `JLedSequence` to update JLed objects simultanously or 
  sequentially. See [README](README.md#controlling-a-group-of-leds) for details.
* added new [morse example](examples/morse)
* clean separation between hardware specific and common code, making
  extendability easy
* added STM32 example to [platformio.ini](platformio.ini)

### Changed

* the brightness user function pointer was replaced by an object of type
  BrightnessEvaluator. Migration of code should be straight forward, see
  below

#### old brightness function 

In JLed version prio to version 4.0.0, a function pointer was used to specify
a user provided brightness function.

```c++
// this function returns changes between 0 and 255 and vice versa every 250 ms.
uint8_t blinkFunc(uint32_t t, uint16_t, uintptr_t) {
  return 255*((t/250)%2);
}

// Run blinkUserFunc for 5000ms
JLed led = JLed(LED_BUILTIN).UserFunc(blinkFunc, 5000);
```

#### new BrightnessEvaluator class

The user function is replaced by a class, which provides more flexibility:

```c++
class UserEffect : public jled::BrightnessEvaluator {
    uint8_t Eval(uint32_t t) const {
        // this function returns changes between 0 and 255 and
        // vice versa every 250 ms.
        return 255*((t/250)%2);
    }
    uint16_t Period() const { return 5000; }
};

UserEffect userEffect;
JLed led = JLed(LED_BUILTIN).UserFunc(&userEffect);
```

### Removed

* `JLed::Invert()` method was removed since became redundant with LowActive()


## [2018-10-03] v3.0.0

* Major refactoring making support of different platforms easier
* ESP32 support added
* Unit tests refactored

## [2018-09-22] v2.4.0

* `JLed::Update()` now returns a `bool` indicating if the effect is still
  active (true), or finished (false).

## [2018-09-22] v2.3.0

* ESP8266 platform: scaling from 8 to 10 bit improved. The scaling makes sure
  that 0 is mapped to 0 and 255 is mapped to 1023, preserving min/max 
  relationships in both ranges.

## [2018-06-09] v2.2.3

### Fixes

* ESP8266 platform: analogWrite() resoultion of 10 bit is now honoured.
  Previously only a range of 0..255 was used, which resulted in output being
  dimmed.

### Added

* It's never to late for a changelog ;)
* ESP8266 environment added to [platform.ini](platform.ini)
