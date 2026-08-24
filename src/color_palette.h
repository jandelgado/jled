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
#pragma once

#include <inttypes.h>   // NOLINT

#include "value_rgb.h"        // RGBColor<T>
#include "scale_bit_depth.h"  // scale_bit_depth

// RGB color constants for JLed

namespace jled {
namespace color {

// Packs 32-bit hex encoded 0xRRGGBB RGB color into RGBColor<uint8_t>
constexpr RGBColor<uint8_t> RGB(uint32_t hex) {
    return {static_cast<uint8_t>((hex >> 16) & 0xFF),
            static_cast<uint8_t>((hex >> 8) & 0xFF),
            static_cast<uint8_t>(hex & 0xFF)};
}

// HTML/CSS named colors (https://www.w3.org/wiki/CSS/Properties/color/keywords)
constexpr RGBColor<uint8_t> kAliceBlue = RGB(0xF0F8FF);
constexpr RGBColor<uint8_t> kAntiqueWhite = RGB(0xFAEBD7);
constexpr RGBColor<uint8_t> kAqua = RGB(0x00FFFF);
constexpr RGBColor<uint8_t> kAquaMarine = RGB(0x7FFFD4);
constexpr RGBColor<uint8_t> kAzure = RGB(0xF0FFFF);
constexpr RGBColor<uint8_t> kBeige = RGB(0xF5F5DC);
constexpr RGBColor<uint8_t> kBisque = RGB(0xFFE4C4);
constexpr RGBColor<uint8_t> kBlack = RGB(0x000000);
constexpr RGBColor<uint8_t> kBlanchedAlmond = RGB(0xFFEBCD);
constexpr RGBColor<uint8_t> kBlue = RGB(0x0000FF);
constexpr RGBColor<uint8_t> kBlueViolet = RGB(0x8A2BE2);
constexpr RGBColor<uint8_t> kBrown = RGB(0xA52A2A);
constexpr RGBColor<uint8_t> kBurlyWood = RGB(0xDEB887);
constexpr RGBColor<uint8_t> kCadetBlue = RGB(0x5F9EA0);
constexpr RGBColor<uint8_t> kChartreuse = RGB(0x7FFF00);
constexpr RGBColor<uint8_t> kChocolate = RGB(0xD2691E);
constexpr RGBColor<uint8_t> kCoral = RGB(0xFF7F50);
constexpr RGBColor<uint8_t> kCornFlowerBlue = RGB(0x6495ED);
constexpr RGBColor<uint8_t> kCornSilk = RGB(0xFFF8DC);
constexpr RGBColor<uint8_t> kCrimson = RGB(0xDC143C);
constexpr RGBColor<uint8_t> kCyan = RGB(0x00FFFF);
constexpr RGBColor<uint8_t> kDarkBlue = RGB(0x00008B);
constexpr RGBColor<uint8_t> kDarkCyan = RGB(0x008B8B);
constexpr RGBColor<uint8_t> kDarkGoldenRod = RGB(0xB8860B);
constexpr RGBColor<uint8_t> kDarkGray = RGB(0xA9A9A9);
constexpr RGBColor<uint8_t> kDarkGrey = RGB(0xA9A9A9);
constexpr RGBColor<uint8_t> kDarkGreen = RGB(0x006400);
constexpr RGBColor<uint8_t> kDarkKhaki = RGB(0xBDB76B);
constexpr RGBColor<uint8_t> kDarkMagenta = RGB(0x8B008B);
constexpr RGBColor<uint8_t> kDarkOliveGreen = RGB(0x556B2F);
constexpr RGBColor<uint8_t> kDarkOrange = RGB(0xFF8C00);
constexpr RGBColor<uint8_t> kDarkOrchid = RGB(0x9932CC);
constexpr RGBColor<uint8_t> kDarkRed = RGB(0x8B0000);
constexpr RGBColor<uint8_t> kDarkSalmon = RGB(0xE9967A);
constexpr RGBColor<uint8_t> kDarkSeaGreen = RGB(0x8FBC8F);
constexpr RGBColor<uint8_t> kDarkSlateBlue = RGB(0x483D8B);
constexpr RGBColor<uint8_t> kDarkSlateGray = RGB(0x2F4F4F);
constexpr RGBColor<uint8_t> kDarkSlateGrey = RGB(0x2F4F4F);
constexpr RGBColor<uint8_t> kDarkTurquoise = RGB(0x00CED1);
constexpr RGBColor<uint8_t> kDarkViolet = RGB(0x9400D3);
constexpr RGBColor<uint8_t> kDeepPink = RGB(0xFF1493);
constexpr RGBColor<uint8_t> kDeepSkyBlue = RGB(0x00BFFF);
constexpr RGBColor<uint8_t> kDimGray = RGB(0x696969);
constexpr RGBColor<uint8_t> kDimGrey = RGB(0x696969);
constexpr RGBColor<uint8_t> kDodgerBlue = RGB(0x1E90FF);
constexpr RGBColor<uint8_t> kFireBrick = RGB(0xB22222);
constexpr RGBColor<uint8_t> kFloralWhite = RGB(0xFFFAF0);
constexpr RGBColor<uint8_t> kForestGreen = RGB(0x228B22);
constexpr RGBColor<uint8_t> kFuchsia = RGB(0xFF00FF);
constexpr RGBColor<uint8_t> kGainsboro = RGB(0xDCDCDC);
constexpr RGBColor<uint8_t> kGhostWhite = RGB(0xF8F8FF);
constexpr RGBColor<uint8_t> kGold = RGB(0xFFD700);
constexpr RGBColor<uint8_t> kGoldenRod = RGB(0xDAA520);
constexpr RGBColor<uint8_t> kGray = RGB(0x808080);
constexpr RGBColor<uint8_t> kGrey = RGB(0x808080);
constexpr RGBColor<uint8_t> kGreen = RGB(0x008000);
constexpr RGBColor<uint8_t> kGreenYellow = RGB(0xADFF2F);
constexpr RGBColor<uint8_t> kHoneyDew = RGB(0xF0FFF0);
constexpr RGBColor<uint8_t> kHotPink = RGB(0xFF69B4);
constexpr RGBColor<uint8_t> kIndianRed = RGB(0xCD5C5C);
constexpr RGBColor<uint8_t> kIndigo = RGB(0x4B0082);
constexpr RGBColor<uint8_t> kIvory = RGB(0xFFFFF0);
constexpr RGBColor<uint8_t> kKhaki = RGB(0xF0E68C);
constexpr RGBColor<uint8_t> kLavender = RGB(0xE6E6FA);
constexpr RGBColor<uint8_t> kLavenderBlush = RGB(0xFFF0F5);
constexpr RGBColor<uint8_t> kLawnGreen = RGB(0x7CFC00);
constexpr RGBColor<uint8_t> kLemonChiffon = RGB(0xFFFACD);
constexpr RGBColor<uint8_t> kLightBlue = RGB(0xADD8E6);
constexpr RGBColor<uint8_t> kLightCoral = RGB(0xF08080);
constexpr RGBColor<uint8_t> kLightCyan = RGB(0xE0FFFF);
constexpr RGBColor<uint8_t> kLightGoldenRodYellow = RGB(0xFAFAD2);
constexpr RGBColor<uint8_t> kLightGray = RGB(0xD3D3D3);
constexpr RGBColor<uint8_t> kLightGrey = RGB(0xD3D3D3);
constexpr RGBColor<uint8_t> kLightGreen = RGB(0x90EE90);
constexpr RGBColor<uint8_t> kLightPink = RGB(0xFFB6C1);
constexpr RGBColor<uint8_t> kLightSalmon = RGB(0xFFA07A);
constexpr RGBColor<uint8_t> kLightSeaGreen = RGB(0x20B2AA);
constexpr RGBColor<uint8_t> kLightSkyBlue = RGB(0x87CEFA);
constexpr RGBColor<uint8_t> kLightSlateGray = RGB(0x778899);
constexpr RGBColor<uint8_t> kLightSlateGrey = RGB(0x778899);
constexpr RGBColor<uint8_t> kLightSteelBlue = RGB(0xB0C4DE);
constexpr RGBColor<uint8_t> kLightYellow = RGB(0xFFFFE0);
constexpr RGBColor<uint8_t> kLime = RGB(0x00FF00);
constexpr RGBColor<uint8_t> kLimeGreen = RGB(0x32CD32);
constexpr RGBColor<uint8_t> kLinen = RGB(0xFAF0E6);
constexpr RGBColor<uint8_t> kMagenta = RGB(0xFF00FF);
constexpr RGBColor<uint8_t> kMaroon = RGB(0x800000);
constexpr RGBColor<uint8_t> kMediumAquaMarine = RGB(0x66CDAA);
constexpr RGBColor<uint8_t> kMediumBlue = RGB(0x0000CD);
constexpr RGBColor<uint8_t> kMediumOrchid = RGB(0xBA55D3);
constexpr RGBColor<uint8_t> kMediumPurple = RGB(0x9370DB);
constexpr RGBColor<uint8_t> kMediumSeaGreen = RGB(0x3CB371);
constexpr RGBColor<uint8_t> kMediumSlateBlue = RGB(0x7B68EE);
constexpr RGBColor<uint8_t> kMediumSpringGreen = RGB(0x00FA9A);
constexpr RGBColor<uint8_t> kMediumTurquoise = RGB(0x48D1CC);
constexpr RGBColor<uint8_t> kMediumVioletRed = RGB(0xC71585);
constexpr RGBColor<uint8_t> kMidnightBlue = RGB(0x191970);
constexpr RGBColor<uint8_t> kMintCream = RGB(0xF5FFFA);
constexpr RGBColor<uint8_t> kMistyRose = RGB(0xFFE4E1);
constexpr RGBColor<uint8_t> kMoccasin = RGB(0xFFE4B5);
constexpr RGBColor<uint8_t> kNavajoWhite = RGB(0xFFDEAD);
constexpr RGBColor<uint8_t> kNavy = RGB(0x000080);
constexpr RGBColor<uint8_t> kOldLace = RGB(0xFDF5E6);
constexpr RGBColor<uint8_t> kOlive = RGB(0x808000);
constexpr RGBColor<uint8_t> kOliveDrab = RGB(0x6B8E23);
constexpr RGBColor<uint8_t> kOrange = RGB(0xFFA500);
constexpr RGBColor<uint8_t> kOrangeRed = RGB(0xFF4500);
constexpr RGBColor<uint8_t> kOrchid = RGB(0xDA70D6);
constexpr RGBColor<uint8_t> kPaleGoldenRod = RGB(0xEEE8AA);
constexpr RGBColor<uint8_t> kPaleGreen = RGB(0x98FB98);
constexpr RGBColor<uint8_t> kPaleTurquoise = RGB(0xAFEEEE);
constexpr RGBColor<uint8_t> kPaleVioletRed = RGB(0xDB7093);
constexpr RGBColor<uint8_t> kPapayaWhip = RGB(0xFFEFD5);
constexpr RGBColor<uint8_t> kPeachPuff = RGB(0xFFDAB9);
constexpr RGBColor<uint8_t> kPeru = RGB(0xCD853F);
constexpr RGBColor<uint8_t> kPink = RGB(0xFFC0CB);
constexpr RGBColor<uint8_t> kPlum = RGB(0xDDA0DD);
constexpr RGBColor<uint8_t> kPowderBlue = RGB(0xB0E0E6);
constexpr RGBColor<uint8_t> kPurple = RGB(0x800080);
constexpr RGBColor<uint8_t> kRebeccaPurple = RGB(0x663399);
constexpr RGBColor<uint8_t> kRed = RGB(0xFF0000);
constexpr RGBColor<uint8_t> kRosyBrown = RGB(0xBC8F8F);
constexpr RGBColor<uint8_t> kRoyalBlue = RGB(0x4169E1);
constexpr RGBColor<uint8_t> kSaddleBrown = RGB(0x8B4513);
constexpr RGBColor<uint8_t> kSalmon = RGB(0xFA8072);
constexpr RGBColor<uint8_t> kSandyBrown = RGB(0xF4A460);
constexpr RGBColor<uint8_t> kSeaGreen = RGB(0x2E8B57);
constexpr RGBColor<uint8_t> kSeaShell = RGB(0xFFF5EE);
constexpr RGBColor<uint8_t> kSienna = RGB(0xA0522D);
constexpr RGBColor<uint8_t> kSilver = RGB(0xC0C0C0);
constexpr RGBColor<uint8_t> kSkyBlue = RGB(0x87CEEB);
constexpr RGBColor<uint8_t> kSlateBlue = RGB(0x6A5ACD);
constexpr RGBColor<uint8_t> kSlateGray = RGB(0x708090);
constexpr RGBColor<uint8_t> kSlateGrey = RGB(0x708090);
constexpr RGBColor<uint8_t> kSnow = RGB(0xFFFAFA);
constexpr RGBColor<uint8_t> kSpringGreen = RGB(0x00FF7F);
constexpr RGBColor<uint8_t> kSteelBlue = RGB(0x4682B4);
constexpr RGBColor<uint8_t> kTan = RGB(0xD2B48C);
constexpr RGBColor<uint8_t> kTeal = RGB(0x008080);
constexpr RGBColor<uint8_t> kThistle = RGB(0xD8BFD8);
constexpr RGBColor<uint8_t> kTomato = RGB(0xFF6347);
constexpr RGBColor<uint8_t> kTurquoise = RGB(0x40E0D0);
constexpr RGBColor<uint8_t> kViolet = RGB(0xEE82EE);
constexpr RGBColor<uint8_t> kWheat = RGB(0xF5DEB3);
constexpr RGBColor<uint8_t> kWhite = RGB(0xFFFFFF);
constexpr RGBColor<uint8_t> kWhiteSmoke = RGB(0xF5F5F5);
constexpr RGBColor<uint8_t> kYellow = RGB(0xFFFF00);
constexpr RGBColor<uint8_t> kYellowGreen = RGB(0x9ACD32);

// Widens an 8-bit color to 16-bit for JLedRGBHD, via bit replication
// (0xFF -> 0xFFFF, 0x38 -> 0x3838)
constexpr RGBColor<uint16_t> Widen(RGBColor<uint8_t> c) {
    return {scale_bit_depth<16>(c.r), scale_bit_depth<16>(c.g), scale_bit_depth<16>(c.b)};
}

}  // namespace color
}  // namespace jled
