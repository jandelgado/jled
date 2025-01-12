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
#include "jled_base.h"  // NOLINT

template <typename T>
class TD;
namespace jled {

// pre-calculated fade-on function. This table samples the function
//   y(x) =  exp(sin((t - period / 2.) * PI / period)) - 0.36787944)
//   * 108.
// at x={0,32,...,256}. In FadeOnFunc() we us linear interpolation to
// approximate the original function (so we do not need fp-ops).
// fade-off and breath functions are all derived from fade-on, see
// below.
// https://www.wolframalpha.com/input/?i=plot+(exp(sin((x-100%2F2.)*PI%2F100))-0.36787944)*108.0++x%3D0+to+100
// The fade-on func is an approximation of
//   y(x) = exp(sin((t-period/2.) * PI / period)) - 0.36787944) * 108.)
//

// 8-bit version with lower accuracy, but slightly faster then fadeon_func
brightness_t fadeon_func8(uint32_t t, uint16_t period) {
    // kFadeOnTable stores brightness samples for x=0..255 in steps of 32
    static constexpr uint8_t kFadeOnTable[] = {0,   3,   13,  33, 68,
                                               118, 179, 232, 255};  // NOLINT
    if (t + 1 >= period) return kFullBrightness;

    // approximate by linear interpolation.

    // t is now >= 0 and < period-1. scale t according to period to 0..255
    t = ((t << 8) / period) & 0xff;
    const auto i = (t >> 5);  // -> i will be in range 0 .. 7
    //
    const uint8_t y0 = kFadeOnTable[i];
    const uint8_t y1 = kFadeOnTable[i + 1];
    const auto x0 = i << 5;  // *32

    // y(t) = mt+b, with m = dy/dx = (y1-y0)/32 = (y1-y0) >> 5
    // return (((t - x0) * (y1 - y0)) >> 5) + y0;
    return ((((t - x0) * (y1 - y0)) >> 5) + y0) << 8;
}

// fadeon_func calculates the fadeon_func function with a higher resolution
brightness_t fadeon_func(uint32_t t, uint16_t period) {
    // kFadeOnTable stores brightness samples for x=0..255 in steps of 16
    static constexpr uint16_t kFadeOnTable[] = {
        0,     198,   807,   1874,  3474,  5714,  8719,  12625, 17545,
        23524, 30485, 38166, 46081, 53536, 59707, 63801, 65535};

    if (t + 1 >= period) return kFullBrightness;

    // approximate by linear interpolation.
    // scale t according to period to 0..255
    const uint8_t i = ((t << 4) / period);  // -> i will be in range 0 .. 15
    const uint16_t y0 = kFadeOnTable[i];
    const uint16_t y1 = kFadeOnTable[i + 1];
//    const auto x0 = i << 4;  // *16
    const uint16_t x0 = (static_cast<uint32_t>(i) * period) >> 4;
    const uint16_t dx = period >> 4;

    // y(t) = mt+b, with m = dy/dx = (y1-y0)/32 = (y1-y0) >> 5
    // return (((t - x0) * (y1 - y0)) >> 5) + y0;
    return (static_cast<uint32_t>((t - x0) * (y1 - y0)) / dx) + y0;
}

static uint32_t rand_ = 0;

void rand_seed(uint32_t seed) { rand_ = seed; }

uint8_t rand8() {
    if (rand_ & 1) {
        rand_ = (rand_ >> 1);
    } else {
        rand_ = (rand_ >> 1) ^ 0x7FFFF159;
    }

    return (uint8_t)rand_;
}

// scale a byte (val) by a byte (factor). scale8 has the following properties:
//   scale8(0, f) == 0 for all f
//   scale8(x, 255) == x for all x
uint8_t scale8(uint8_t val, uint8_t factor) {
    return (static_cast<uint16_t>(val) * (1+static_cast<uint16_t>(factor))) >> 8;
}

// interpolate a byte (val) to the interval [a,b].
uint8_t lerp8by8(uint8_t val, uint8_t a, uint8_t b) {
    if (a == 0 && b == 255) return val;  // optimize for most common case
    const uint8_t delta = b - a;
    return a + scale8(val, delta);
}

// the inverse of lerp8by8: invlerp8by8(lerp8by8(x, a, b,), a, b,) = x
uint8_t invlerp8by8(uint8_t val, uint8_t a, uint8_t b) {
    const uint16_t delta = b - a;
    if (delta == 0) return 0;
    return (static_cast<uint16_t>(val - a) * 255) / (delta);
}

//   scale16(0, f) == 0 for all f
//   scale16(x, 65535) == x for all x
uint16_t scale16(uint16_t val, uint16_t factor) {
    return (static_cast<uint32_t>(val) * (1+static_cast<uint32_t>(factor))) >> 16;
}

// interpolate a word (val) to the interval [a,b].
uint16_t lerp16by16(uint16_t val, uint16_t a, uint16_t b) {
    if (a == 0 && b == 0xffff) return val;  // optimize for most common case
    const uint16_t delta = b - a;
    return a + scale16(val, delta);
}
};  // namespace jled
