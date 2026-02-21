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

// we pre-calculated fade-on function. This table samples the function
//   y(x) =  exp(sin((t - period / 2.) * PI / period)) - 0.36787944) * 108.
// at x={0,32,...,256}. In FadeOnFunc() we us linear interpolation to
// approximate the original function (so we do not need fp-ops).
// fade-off and breath functions are all derived from fade-on, see
// below.
//
// https://www.wolframalpha.com/input/?i=plot+(exp(sin((x-100%2F2.)*PI%2F100))-0.36787944)*108.0++x%3D0+to+100
// The fade-on func is an approximation of
//   y(x) = exp(sin((t-period/2.) * PI / period)) - 0.36787944) * 108.)

// 8-bit specialization
template<>
uint8_t fadeon_func<uint8_t>(uint32_t t, uint16_t period) {
	// pre-calculated fade-on function at x={0,16,...,256}
	static constexpr uint8_t lut8[] = {
        0, 0, 3, 7, 13, 22, 33, 49, 68, 91, 118, 148, 179, 208, 232, 248, 255
    };
    if (t + 1 >= period) return 255;

    // approximate by linear interpolation.
    // normalize t to [0, 256) scale
    const auto tnorm = (t << 8) / period;
    const auto i = tnorm >> 4;  // segment index (0..15)

    const auto y0 = lut8[i];
    const auto y1 = lut8[i + 1];

    const auto x0 = i << 4;  // segment start in normalized space
    return (((tnorm - x0) * (y1 - y0)) >> 4) + y0;
}

// t = 0..period-1
template<>
uint16_t fadeon_func<uint16_t>(uint32_t t, uint16_t period) {
	// pre computed fade-func at x={0,2048,...,65536}
	static constexpr uint16_t lut16[] = {
        0, 49, 198, 448, 807, 1278, 1874, 2600, 3474, 4505,
        5714, 7110, 8719, 10548, 12625, 14949, 17545, 20398,
        23524, 26888, 30485, 34254, 38166, 42127, 46081, 49910,
        53536, 56829, 59707, 62054, 63801, 64874, 65535
    };

    if (t + 1 >= period) return 65535;

    // normalize t to [0, 65536) scale
    const auto tnorm = (t << 16) / period;
    const auto i = tnorm >> 11;  // segment index (0..31), since 65536/32 = 2048 = 2^11

    const auto y0 = lut16[i];
    const auto y1 = lut16[i + 1];

    const auto x0 = i << 11;  // segment start in normalized space
    return (((tnorm - x0) * (y1 - y0)) >> 11) + y0;
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

};  // namespace jled
