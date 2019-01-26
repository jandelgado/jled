# JLed changelog

## [2019-01-23] v4.0.0

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
```
// this function returns changes between 0 and 255 and vice versa every 250 ms.
uint8_t blinkFunc(uint32_t t, uint16_t, uintptr_t) {
  return 255*((t/250)%2);
}

// Run blinkUserFunc for 5000ms
JLed led = JLed(LED_BUILTIN).UserFunc(blinkFunc, 5000);
```

#### new BrightnessEvaluator class

The user function is replaced by a class, which provides more flexibility:

```
class UserEffect : public jled::BrightnessEvaluator {
    uint8_t Eval(uint32_t t) {
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
