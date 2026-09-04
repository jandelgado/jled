// JLed group_ref demo: example for mixing LED types and nesting groups. JLedGroup only accepts one
// LED type per group, written inline. JLedRefGroup accept any mix (here: JLed, JLedHD, and a nested
// JLedRefGroup), but LEDs have to be defined as named variables now.
//
// Copyright 2026 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include <jled.h>

auto inner0 = JLed(5).Blink(250, 250).Repeat(2);
auto inner1 = JLedHD(6).FadeOn(1000);
JLedRef inner_refs[] = {&inner0, &inner1};
auto innerGroup = JLedRefGroup::Parallel(inner_refs);

auto led0 = JLed(4).Blink(750, 250).Repeat(2);
auto led1 = JLedHD(3).Breathe(2000);
JLedRef leds[] = {&led0, &led1, &innerGroup};

auto group = JLedRefGroup::Sequential(leds);

void setup() {}

void loop() {
    group.Update();
}
