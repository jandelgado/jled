// JLed group_ref demo. Mix JLed, JLedHD and nested groups via JLedRef.
// Uses JLedRef/JLedRefGroup: each slot stores only a pointer, with no
// internal buffer. LED objects must be declared as named variables.
// Copyright 2026 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include <jled.h>

static JLed   inner0 = JLed(5).Blink(250, 250).Repeat(2);
static JLedHD inner1 = JLedHD(6).FadeOn(1000);
static JLedRef inner_refs[] = {&inner0, &inner1};
static auto innerGroup = JLedRefGroup::Parallel(inner_refs);

static JLed   led0 = JLed(4).Blink(750, 250).Repeat(2);
static JLedHD led1 = JLedHD(3).Breathe(2000);
static JLedRef leds[] = {&led0, &led1, &innerGroup};

auto group = JLedRefGroup::Sequential(leds);

void setup() { }

void loop() {
    group.Update();
    delay(1);
}
