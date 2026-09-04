# RGB example

This example shows how to drive RGB-LEDs with JLed. Since `JLedRGB` shares the code with `JLed`,
the API is the same, except that you can pass `RGBColor` color values instead of brightness values.

The demo cycles through fade, candle, breathe and blink effects, including a sweep through the HSV
color wheel. In addition, the built-in LED blinks each time the RGB LED moves to the next effect. The LED used here is low-active ("common anode") and is connected to the GPIOs 13 (red), 14 (green) and 15 (blue).

## Wiring

![Wiring](../../doc/rgb_led_bb.png)
