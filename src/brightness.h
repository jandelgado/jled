// Copyright (c) 2017-2025 Jan Delgado <jdelgado[at]gmx.net>
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
#ifndef SRC_BRIGHTNESS_H_
#define SRC_BRIGHTNESS_H_

#include <inttypes.h>

namespace jled {

// Simple type comparison (avoids std::is_same dependency for Arduino compatibility)
template<typename T, typename U>
struct is_same {
    static constexpr bool value = false;
};

template<typename T>
struct is_same<T, T> {
    static constexpr bool value = true;
};

// Helper for C++14 compatibility (avoids variable templates which cause warnings)
template<typename T, typename U>
struct is_same_helper {
    static constexpr bool value = is_same<T, U>::value;
};

// C++14-style variable template for convenience (with fallback for pre-C++14)
#if __cplusplus >= 201402L
template<typename T, typename U>
constexpr bool is_same_v = is_same<T, U>::value;
#endif

// Type traits for brightness types - provides constants and metadata
template<typename T>
struct BrightnessTypeTraits;

template<>
struct BrightnessTypeTraits<uint8_t> {
    using value_t = uint8_t;
    static constexpr uint8_t kFullBrightness = 255;
    static constexpr uint8_t kZeroBrightness = 0;
    static constexpr uint8_t kBits = 8;
};

template<>
struct BrightnessTypeTraits<uint16_t> {
    using value_t = uint16_t;
    static constexpr uint16_t kFullBrightness = 65535;
    static constexpr uint16_t kZeroBrightness = 0;
    static constexpr uint8_t kBits = 16;
};

// Convenience aliases for brightness types (follow _t naming convention)
using brightness8_t = uint8_t;
using brightness16_t = uint16_t;

}  // namespace jled

#endif  // SRC_BRIGHTNESS_H_
