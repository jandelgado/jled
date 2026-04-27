// Copyright (c) 2017-2020 Jan Delgado <jdelgado[at]gmx.net>
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

namespace jled {

// C++14-compatible conditional type selector (substitute for std::conditional)
template <bool, typename T, typename F> struct Conditional { using type = F; };
template <typename T, typename F>
struct Conditional<true, T, F> { using type = T; };

// C++14-compatible enable_if (substitute for std::enable_if)
template <bool B, typename T = void> struct EnableIf {};
template <typename T> struct EnableIf<true, T> { using type = T; };

// C++14-compatible is_base_of (substitute for std::is_base_of).
// Uses the pointer-conversion trick: if Derived* is implicitly convertible to
// Base*, then Base is a public base of Derived.
template <typename Base, typename Derived>
struct IsBaseOf {
 private:
    static char check(const Base*);
    static long check(...);  // NOLINT
 public:
    static constexpr bool value =
        sizeof(check(static_cast<const Derived*>(nullptr))) == sizeof(char);
};

}  // namespace jled
