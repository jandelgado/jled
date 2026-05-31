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

namespace jled {

// pre-calculated fade-on function. This look up table samples the function
//   y(x) =  exp(sin((t - period / 2.) * PI / period)) - 0.36787944) * 108.
// at x={0,32,...,256} (8-bit case). We us linear interpolation to
// approximate the original function (so we do not need fp-ops).
// fade-off and breath functions are all derived from fade-on.
//
// https://www.wolframalpha.com/input/?i=plot+(exp(sin((x-100%2F2.)*PI%2F100))-0.36787944)*108.0++x%3D0+to+100
// The fade-on func is an approximation of
//   y(x) = exp(sin((t-period/2.) * PI / period)) - 0.36787944) * 108.)

// 8-bit specialization
template <>
uint8_t fadeon_func<uint8_t>(uint32_t t, uint16_t period) {
    // pre-calculated fade-on function at x={0,16,...,256}
    static constexpr uint8_t lut[] = {
        0, 0, 3, 7, 13, 22, 33, 49, 68, 91, 118, 148, 179, 208, 232, 248, 255};
    return lut_lerp(t, period, lut);
}

// t = 0..period-1
template <>
uint16_t fadeon_func<uint16_t>(uint32_t t, uint16_t period) {
    // pre-calculated fade-on function at x={0,2048,...,65536}
    static constexpr uint16_t lut[] = {
        0,     49,    198,   448,   807,   1278,  1874,  2600,  3474,  4505,
        5714,  7110,  8719,  10548, 12625, 14949, 17545, 20398, 23524, 26888,
        30485, 34254, 38166, 42127, 46081, 49910, 53536, 56829, 59707, 62054,
        63801, 64874, 65535};
    return lut_lerp(t, period, lut);
}

static uint8_t hash8(uint32_t x) {
    x += 0x9e3779b9u;
    x ^= x >> 16;
    x *= 0x45d9f3bu;
    x ^= x >> 16;
    return static_cast<uint8_t>(x);
}

template <>
uint8_t candle_func<uint8_t>(uint32_t t, uint8_t speed, uint8_t jitter) {
    // gamma-corrected (γ=2.2) flicker values
    static constexpr uint8_t kCandleTable[] = {
        9, 14, 21, 29, 38, 48, 60, 74, 89, 106, 124, 143, 165, 188, 212, 238};
    const uint32_t slot = t >> (speed & 0x1f);
    if (hash8(slot) >= jitter) return kFullBrightness;
    return kCandleTable[hash8(~slot) & 0xf];
}

template <>
uint16_t candle_func<uint16_t>(uint32_t t, uint8_t speed, uint8_t jitter) {
    const uint8_t val8 = candle_func<uint8_t>(t, speed, jitter);
    return static_cast<uint16_t>((val8 << 8) | val8);
}

};  // namespace jled
