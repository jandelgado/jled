// Copyright (c) 2026 Jan Delgado <jdelgado[at]gmx.net>
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
// The chromaticity mapping in detail::hsv2rgb_rainbow_chroma is adapted from
// FastLED's hsv2rgb_rainbow (src/hsv2rgb.cpp.hpp, hsv2rgb_raw_C sibling in
// the FastLED project, https://github.com/FastLED/FastLED), MIT licensed,
// Copyright (c) FastLED contributors. It reproduces the eight-sector
// rainbow chromaticity curve (with orange/yellow's Y1 boost, the default in
// upstream FastLED); saturation and value scaling below are JLed's own,
// built on jled::scale8() rather than porting FastLED's video-scaling
// (scale8_video) exactly, so output is not guaranteed byte-identical to
// FastLED at every input, only the same rainbow shape.
#include "value_rgb.h"  // NOLINT

#include "scale_bit_depth.h"  // scale_bit_depth, for Widen8to16 below
#include "scale_lerp.h"  // scale

namespace jled {
namespace detail {

// Maps hue (0..255) to a chromaticity-only RGB triple (saturation/value not
// yet applied). One of eight 32-wide sectors, `third`/`two_third` fade
// linearly within each sector. Adapted from FastLED.
RGBColor<uint8_t> hsv2rgb_rainbow_chroma(uint8_t hue) {
    const uint8_t offset = hue & 0x1f;                          // 0..31 within the sector
    const uint8_t offset8 = static_cast<uint8_t>(offset << 3);  // 0..248
    const uint8_t third = scale8(offset8, 85);                  // 0..~85 (256/3)

    uint8_t r, g, b;
    if (!(hue & 0x80)) {
        if (!(hue & 0x40)) {
            if (!(hue & 0x20)) {  // sector 0: red -> orange
                r = static_cast<uint8_t>(255 - third);
                g = third;
                b = 0;
            } else {  // sector 1: orange -> yellow
                r = 171;
                g = static_cast<uint8_t>(85 + third);
                b = 0;
            }
        } else {
            if (!(hue & 0x20)) {  // sector 2: yellow -> green
                const uint8_t two_third = scale8(offset8, 170);
                r = static_cast<uint8_t>(171 - two_third);
                g = static_cast<uint8_t>(170 + third);
                b = 0;
            } else {  // sector 3: green -> aqua
                r = 0;
                g = static_cast<uint8_t>(255 - third);
                b = third;
            }
        }
    } else {
        if (!(hue & 0x40)) {
            if (!(hue & 0x20)) {  // sector 4: aqua -> blue
                const uint8_t two_third = scale8(offset8, 170);
                r = 0;
                g = static_cast<uint8_t>(171 - two_third);
                b = static_cast<uint8_t>(85 + two_third);
            } else {  // sector 5: blue -> purple
                r = third;
                g = 0;
                b = static_cast<uint8_t>(255 - third);
            }
        } else {
            if (!(hue & 0x20)) {  // sector 6: purple -> pink
                r = static_cast<uint8_t>(85 + third);
                g = 0;
                b = static_cast<uint8_t>(171 - third);
            } else {  // sector 7: pink -> red
                r = static_cast<uint8_t>(170 + third);
                g = 0;
                b = static_cast<uint8_t>(85 - third);
            }
        }
    }
    return {r, g, b};
}

}  // namespace detail

RGBColor<uint8_t> hsv_to_rgb(HSV<uint8_t> c) {
    auto rgb = detail::hsv2rgb_rainbow_chroma(c.h);

    if (c.s != 255) {
        if (c.s == 0) {
            rgb = {255, 255, 255};
        } else {
            const uint8_t desat =
                scale8(static_cast<uint8_t>(255 - c.s), static_cast<uint8_t>(255 - c.s));
            const uint8_t satscale = static_cast<uint8_t>(255 - desat);
            rgb.r = static_cast<uint8_t>(scale8(rgb.r, satscale) + desat);
            rgb.g = static_cast<uint8_t>(scale8(rgb.g, satscale) + desat);
            rgb.b = static_cast<uint8_t>(scale8(rgb.b, satscale) + desat);
        }
    }

    if (c.v != 255) {
        if (c.v == 0) {
            rgb = {0, 0, 0};
        } else {
            rgb.r = scale8(rgb.r, c.v);
            rgb.g = scale8(rgb.g, c.v);
            rgb.b = scale8(rgb.b, c.v);
        }
    }
    return rgb;
}

// runs the 8-bit chromaticity mapping on the high bytes of h/s at full value, widens
// the result, and applies v at full 16-bit precision. Hue/saturation
// resolve to 256 steps, value to 65536.
RGBColor<uint16_t> hsv_to_rgb(HSV<uint16_t> c) {
    const auto rgb8 = hsv_to_rgb(
        HSV<uint8_t>{static_cast<uint8_t>(c.h >> 8), static_cast<uint8_t>(c.s >> 8), 255});
    return {scale<uint16_t>(scale_bit_depth<16>(rgb8.r), c.v),
            scale<uint16_t>(scale_bit_depth<16>(rgb8.g), c.v),
            scale<uint16_t>(scale_bit_depth<16>(rgb8.b), c.v)};
}

}  // namespace jled
