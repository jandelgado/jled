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

#include <inttypes.h>  // types, e.g. uint8_t NOLINT

// Classic AVR uses separate program and data address spaces. As a result,
// plain `const` globals are typically copied into RAM at startup unless
// placed in flash with PROGMEM and accessed via pgm_read_*().
//
// On most other supported platforms, flash is memory-mapped into the normal
// address space (XIP), so `const` data already remains in flash and can be
// accessed with ordinary pointer/array reads. There, JLED_PROGMEM and
// FlashReader are effectively no-ops.
//
// This is a standalone, __AVR__-gated concern, independent of jled.h's
// platform detection (which selects HAL/Clock implementations, not storage
// attributes). Kept here so jled_effects.h/.cpp, which this exists for,
// don't need to depend on jled.h.
#if defined(__AVR__)
#include <avr/pgmspace.h>
#define JLED_PROGMEM PROGMEM

template<typename T>
struct FlashReader {
    static_assert(sizeof(T) == 1 || sizeof(T) == 2, "FlashReader supports 8/16-bit types only");
    static T Read(const T* p) {
        return sizeof(T) == 1 ? static_cast<T>(pgm_read_byte(p)) : static_cast<T>(pgm_read_word(p));
    }
};
#else
#define JLED_PROGMEM

template<typename T>
struct FlashReader {
    static T Read(const T* p) { return *p; }
};
#endif
