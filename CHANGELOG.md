# JLed changelog (github.com/jandelgado/jled)

## [Unreleased - scheduled for JLed 5.0]

JLed 5.0 is a major release with several breaking changes. See the "JLed 5.0 migration
guide" in README.md for migration details and code samples.

- new: `JLedGroup`/`JLedHDGroup` replace `JLedSequence`: a simple group supporting a
  single `JLed`/`JLedHD` type, e.g. `JLed leds[]`. Adds `Reverse()`/`Skip()`/
  `IsReversed()` for backward and ping-pong playback, `Pause()`/`Resume()`, and
  group-level lifecycle events via `GroupUpdateResult`. New `JLedRef` and `JLedRefGroup`
  let's you mix LED types or nest groups. See "Controlling a group of LEDs" in the README
- new: lifecycle events for `JLed`/`JLedHD` (start, first output, repeat start, entering
  delay-after, done), with matching `On*()` callbacks, via `UpdateResult`
- new: `Pause()`/`Resume()`/`IsPaused()` to freeze and later resume an effect. Also
  works on groups, pausing all members at once
- new: high resolution (up to 16-bit) effects with `JLedHD`
- new: `JLedRGB`/`JLedRGBHD` control a three-pin RGB LED with the same API and
  effects as `JLed`/`JLedHD`, using `jled::RGBColor<uint8_t>`/`jled::RGBColor<uint16_t>`
  colors in place of a plain brightness value. `Blink`/`Set`/`On`/`Off`/`Candle` write the
  given color exactly, with no color-space conversion; `Breathe`/`FadeOn`/`FadeOff`/`Fade`
  internally calculate transitions using the HSV color model, so a sweep between two
  differently-hued colors ramps through hue space instead of a flat per-channel RGB
  blend's muddy midpoint. `JLedRGBGroup`/`JLedRGBHDGroup` group them, see "JLedRGB"
- new: `Breathe` takes trailing color parameters. The defaults reproduce the previous behaviour
  exactly, so existing code is unaffected
- breaking: `Candle` takes leading `color_on`/`color_off` parameters:
  `Candle(color_on, color_off, speed, jitter, period, offset)`. The flicker now ramps
  between the two instead of always dimming towards implicit black, e.g. a red/yellow
  flicker on `JLedRGB`. On `JLed`/`JLedHD` these are plain brightness values, defaulting
  to fully on / fully off (the previous behaviour)
- new: `WriteRaw(val)` writes a brightness value straight to the hardware, bypassing the
  effect logic. Handy from a lifecycle callback
- new: `Percentage`/`_pct` literal to set `MinBrightness`/`MaxBrightness` as a percentage
- new: native STM32Cube HAL, see the [stm32cube_demo](examples/stm32cube_demo) example
- new: `LowActive(bool on = true)` takes a parameter, so it can be toggled off again with
  `LowActive(false)`
- new: `Blink` effect now has a repeat parameter and parameters for the on- and off-color. On
  `JLed`/`JLedHD` these are plain brightness values, so e.g. `Blink(250, 250, 1, 200, 20)` blinks
  between the two brightness 200 and 20 levels instead of on and off. In `JLedRGB` these are RGB
  colors.
- perf: `ArduinoHal` now sets up the pin eagerly in its constructor by default
- fix: `JLedGroup::Update()` could silently skip a repetition if called more than once
  within the same millisecond at a repetition boundary
- breaking: `FadeOn(duration, from, to)`'s color parameters are reordered to
  `FadeOn(duration, to, from)`, so the single positional color argument (`to`) means what
  the name implies: "fade on, to this color". `FadeOff` is unchanged, since its own name
- breaking: `Update(int16_t* pLast)` removed; use `UpdateResult::Value()` /
  `HasValue()` instead
- breaking: `JLed::brightness_t`/`JLedHD::brightness_t` renamed to `value_t`
- breaking: lifecycle callbacks (`OnStart`, `OnDone`, ...) now receive the `JLed`/
  `JLedHD`/`JLedGroup` by reference (`JLed&`) instead of by pointer (`JLed*`)
- breaking: `eStopMode` renamed to `eIdleMode` and moved to the `jled` namespace
- breaking: custom `BrightnessEvaluator`s are now templated on the value type, and
  their `Eval()` takes `jled::period_t t` instead of `uint32_t t`
- breaking: the free-standing legacy `jled::kFullBrightness`/`jled::kZeroBrightness`
  constants are removed; use `jled::ValueTraits<uint8_t>::kMaxValue()`/`kOffColor()`
  instead
- breaking (custom HAL/`TJLed` authors): HALs no longer provide `millis()`; time now
  comes from a separate `Clock` class, and `TJLed` takes two more template parameters
- breaking (HAL authors only): `analogWrite()` gains a required `invert` parameter, and
  HALs need a new `SetLowActive(bool)` method. Wrap an old single-argument HAL in the new
  `InvertableHal<Hal>` to get both for free; ESP32 and Pico invert natively instead

## [2024-12-01] 4.15.0

- new: `Update()` methods now optionally return the last brightness value
  calculated and written out to the LED. See `examples/last_brightness`

## [2024-09-21] 4.14

- new: make `JLed::Update(unit32_t t)` public, allowing optimizations and
  simplified tests

## [2023-09-10] 4.13.1

- fix: `Update()` sometimes returning wrong state (https://github.com/jandelgado/jled/issues/122)

## [2023-08-20] 4.13.0

- new: `Stop()` takes optional parameter allowing to turn LED fully off

## [2023-06-29] 4.12.2

- fix: `JLedSequence` starting again after call to `Stop` (https://github.com/jandelgado/jled/issues/115)

## [2023-01-11] 4.12.1

- fix: add missing keywords to keywords.txt

## [2022-11-13] 4.12.0

- new: add `MinBrightness` method and scale output to interval defined by
  `MinBrightness` and `MaxBrightness`.

## [2022-11-13] 4.11.1

- improve: reduce memory consumption of JLed objects by 3 bytes, simplify
  state management.

## [2022-03-29] 4.11.0

- change: `JLedSequence` objects are now assignable, making switching
  effects easier. See https://github.com/jandelgado/jled-example-switch-sequence for an example.

## [2022-03-24] 4.10.0

- new: `On`, `Off` and `Set` now take an optional `duration` value, making
  these effects behave like any other in this regard. This allows to add
  an `On` effect to a `JLedSequence` for a specific amount of time.

## [2022-02-24] 4.9.1

- fix: make sure JLedSequence methods like `Repeat` and `Forever` are chainable
  like in the `JLed` class

## [2022-02-13] 4.9.0

- new: support ESP-IDF platform for the ESP32 (#87, thanks to @troky for the
  initial work). See also repositories
  https://github.com/jandelgado/jled-esp-idf-example and
  https://github.com/jandelgado/jled-esp-idf-platformio-example

## [2021-10-18] 4.8.0

- new: make `Breathe` method more flexible (#78, thanks to @boraozgen)

## [2021-10-13] 4.7.1

- fix: correct handling of time rollover (#80, thanks to @boraozgen)

## [2021-02-02] 4.7.0

- new: support for Raspberry Pi Pico added

## [2021-02-02] 4.6.1

- fix: `Forever()` on sequence had no effect (#68)

## [2021-01-24] 4.6.0

- new: JLedSequence can be configured to play the sequence multiple times
  using the `Repeat()` and `Forever()` methods
- drop travis-ci, use github actions

## [2020-10-24] 4.5.2

- fix: ESP32 led glimming when using low-active connection (#60)

## [2020-07-01] 4.5.1

- fix: support for Nano 33 BLE (#53)

## [2020-02-23] 4.5.0

- new: `JLed::MaxBrightness(uint8_t level)` method to limit output of effects
  (implements #43).

## [2020-02-21] 4.4.0

- JLed now supports the mbed framework. See README.md and `examples/multiled_mbed`
  for examples.

## [2019-09-21] 4.3.0

- new example: [custom HAL](examples/custom_hal/custom_hal.ino) showing
  how to implment a custom HAL.

## [2019-08-30] 4.2.1

- fix: make sure memory alignment is correct (caused hard fault on
  SAMD21). Fixes #27.

## [2019-06-20] 4.2.0

- changing an effect resets the Jled object so it starts over with the
  new effect (see #25). Prior to this change, calling `Reset()` was
  necessary.

## [2019-05-11] 4.1.2

- fix: ESP32 dynamic channel assignment fixed. Sequence demo now working
  as expected (see #22).

## [2019-05-07] 4.1.1

- fix: version format in library.properties (removed leading `v`; see #21)

## [2019-03-10] v4.1.0

- change: clean up interface and simplify code: `On()` no longer takes an
  optional brightness argument. Call `Set(uint8_t brightness)` instead.
- documentation update

## [2019-03-10] v4.0.0

In addition to the changes introduced with `v4.0.0-rc0` and `v4.0.0-rc1`, the
`v4.0.0` relases adds/changes the following:

### Added

- new `Candle()` effect added for candles and fire like effects

### Changed

- The user provided brightness class no longer needs a `Clone()` method. See
  [example](examples/user_func/user_func.ino) for an example

### Fixed

- Makefile (unit testing) dependency checking fixed

## [2019-02-17] v4.0.0-rc1

### Changed

- fix: byte buffer alignment for ESP8266 set to DWORD boundary making ESP8266
  run again with JLed 4.x
- arduino HAL now does lazy call to pinMode() to prevent STM32 problems
- simplified morse example code

## [2019-01-23] v4.0.0-rc0

### Added

- `JLed::Reset()` - resets the led to it's initial state allowing to
  to start over
- `JLed::IsRunning()` - return true if effect is active, else false
- new class `JLedSequence` to update JLed objects simultanously or
  sequentially. See [README](README.md#controlling-a-group-of-leds) for details.
- added new [morse example](examples/morse)
- clean separation between hardware specific and common code, making
  extendability easy
- added STM32 example to [platformio.ini](platformio.ini)

### Changed

- the brightness user function pointer was replaced by an object of type
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

- `JLed::Invert()` method was removed since became redundant with LowActive()

## [2018-10-03] v3.0.0

- Major refactoring making support of different platforms easier
- ESP32 support added
- Unit tests refactored

## [2018-09-22] v2.4.0

- `JLed::Update()` now returns a `bool` indicating if the effect is still
  active (true), or finished (false).

## [2018-09-22] v2.3.0

- ESP8266 platform: scaling from 8 to 10 bit improved. The scaling makes sure
  that 0 is mapped to 0 and 255 is mapped to 1023, preserving min/max
  relationships in both ranges.

## [2018-06-09] v2.2.3

### Fixes

- ESP8266 platform: analogWrite() resoultion of 10 bit is now honoured.
  Previously only a range of 0..255 was used, which resulted in output being
  dimmed.

### Added

- It's never to late for a changelog ;)
- ESP8266 environment added to [platform.ini](platform.ini)
