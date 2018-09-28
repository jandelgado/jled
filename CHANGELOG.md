# JLed changelog

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
