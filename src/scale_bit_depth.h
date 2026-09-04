// Copyright (c) 2017-2026 Jan Delgado <jdelgado[at]gmx.net>
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
#pragma once

#include <inttypes.h>

namespace jled {

namespace detail {
// Upscale via bit replication for the lower bits, while keeping min/max
// relationships: 0→0, max→max, linear in between.
template<uint8_t ResBits, uint8_t kSrcBits>
constexpr uint16_t scale_replicate_bits(uint16_t v) {
    return static_cast<uint16_t>(
        (v << (ResBits > kSrcBits ? ResBits - kSrcBits : 0)) |
        (v >> (ResBits > kSrcBits ? kSrcBits - (ResBits - kSrcBits) : 0)));
}
}  // namespace detail

// Scale a fixed-point level value to a target bit-width (e.g. a
// HAL's native PWM resolution, or widening an 8-bit color channel to 16-bit).
//
// ResBits: Target resolution in bits (1-16)
// Level: Source value type (uint8_t or uint16_t)
//
// Returns: Scaled value in range [0, 2^ResBits - 1]. Uses bit-replication when
// upscaling and preserves min/max relationships:
// - scale_bit_depth<ResBits,Level>(0) = 0
// - scale_bit_depth(kMaxLevel) = 2^ResBits-1 (e.g. 255 or 65535)
//
template<uint8_t ResBits, typename Level>
constexpr uint16_t scale_bit_depth(Level val) {
    static_assert(ResBits >= 1 && ResBits <= 16, "PWM resolution must be between 1 and 16 bits");
    static_assert(sizeof(Level) == 1 || sizeof(Level) == 2,
                  "Level must be uint8_t or uint16_t");
    // clang-format off
    // C++11 allows only a return statement in a constexpr function (AVR, ESP32) therefore
    // the split in 2 functions here.
    return ResBits == sizeof(Level) * 8
        ? static_cast<uint16_t>(val)
        : ResBits > sizeof(Level) * 8
            ? detail::scale_replicate_bits<ResBits, sizeof(Level) * 8>(static_cast<uint16_t>(val))
            // Downscale: simple right shift, max naturally maps to max
            // because (2^kSrcBits - 1) >> (kSrcBits - ResBits) == 2^ResBits - 1
            : static_cast<uint16_t>(val >> (sizeof(Level) * 8 - ResBits));
    // clang-format on
}

}  // namespace jled
