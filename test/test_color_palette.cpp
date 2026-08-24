// Unit tests for jled::color's basic color palette.
// Copyright 2026 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include "catch2/catch_amalgamated.hpp"
#include "color_palette.h"  // NOLINT

using jled::RGBColor;
using jled::color::kBlack;
using jled::color::kBlue;
using jled::color::kCyan;
using jled::color::kGreen;
using jled::color::kLime;
using jled::color::kMagenta;
using jled::color::kRed;
using jled::color::kWhite;
using jled::color::kYellow;
using jled::color::RGB;
using jled::color::Widen;

TEST_CASE("RGB() unpacks a 0xRRGGBB literal into RGBColor<uint8_t>", "[color_palette]") {
    CHECK(RGB(0xFF3849) == RGBColor<uint8_t>{0xFF, 0x38, 0x49});
    CHECK(RGB(0x000000) == RGBColor<uint8_t>{0, 0, 0});
}

TEST_CASE("basic color palette matches expected RGBColor<uint8_t> values", "[color_palette]") {
    CHECK(kBlack == RGBColor<uint8_t>{0, 0, 0});
    CHECK(kWhite == RGBColor<uint8_t>{255, 255, 255});
    CHECK(kRed == RGBColor<uint8_t>{255, 0, 0});
    CHECK(kGreen == RGBColor<uint8_t>{0, 128, 0});
    CHECK(kLime == RGBColor<uint8_t>{0, 255, 0});
    CHECK(kBlue == RGBColor<uint8_t>{0, 0, 255});
    CHECK(kYellow == RGBColor<uint8_t>{255, 255, 0});
    CHECK(kCyan == RGBColor<uint8_t>{0, 255, 255});
    CHECK(kMagenta == RGBColor<uint8_t>{255, 0, 255});
}

TEST_CASE("Widen() bit-replicates each channel to 16 bit", "[color_palette]") {
    CHECK(Widen(kBlack) == RGBColor<uint16_t>{0, 0, 0});
    CHECK(Widen(kRed) == RGBColor<uint16_t>{0xFFFF, 0, 0});
    CHECK(Widen(RGB(0x38FFFF)) == RGBColor<uint16_t>{0x3838, 0xFFFF, 0xFFFF});
}

TEST_CASE("named colors and RGB()/Widen() are usable in constexpr context", "[color_palette]") {
    constexpr auto c8 = RGB(0x123456);
    constexpr auto c16 = Widen(c8);
    STATIC_REQUIRE(c8 == RGBColor<uint8_t>{0x12, 0x34, 0x56});
    STATIC_REQUIRE(c16 == RGBColor<uint16_t>{0x1212, 0x3434, 0x5656});
}
