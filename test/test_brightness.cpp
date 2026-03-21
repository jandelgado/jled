// Copyright (c) 2025 Jan Delgado <jdelgado[at]gmx.net>
// https://github.com/jandelgado/jled
//
// Unit tests for brightness module (brightness.h)
//
#include <catch2/catch_amalgamated.hpp>
#include "brightness.h"  // NOLINT

using jled::Percentage;
using jled::scaleToNative;
using jled::operator""_pct;

TEST_CASE("Percentage - converts to uint8_t", "[brightness]") {
    REQUIRE(static_cast<uint8_t>(Percentage(0))   == 0);
    REQUIRE(static_cast<uint8_t>(Percentage(1))   == 2);    // 1*255/100
    REQUIRE(static_cast<uint8_t>(Percentage(50))  == 127);  // 50*255/100
    REQUIRE(static_cast<uint8_t>(Percentage(75))  == 191);  // 75*255/100
    REQUIRE(static_cast<uint8_t>(Percentage(99))  == 252);  // 99*255/100
    REQUIRE(static_cast<uint8_t>(Percentage(100)) == 255);
}

TEST_CASE("Percentage - converts to uint16_t", "[brightness]") {
    REQUIRE(static_cast<uint16_t>(Percentage(0))   == 0);
    REQUIRE(static_cast<uint16_t>(Percentage(1))   == 655);    // 1*65535/100
    REQUIRE(static_cast<uint16_t>(Percentage(50))  == 32767);  // 50*65535/100
    REQUIRE(static_cast<uint16_t>(Percentage(75))  == 49151);  // 75*65535/100
    REQUIRE(static_cast<uint16_t>(Percentage(99))  == 64879);  // 99*65535/100
    REQUIRE(static_cast<uint16_t>(Percentage(100)) == 65535);
}

TEST_CASE("Percentage - _pct literal matches Percentage constructor", "[brightness]") {
    REQUIRE(static_cast<uint8_t>(0_pct)   == static_cast<uint8_t>(Percentage(0)));
    REQUIRE(static_cast<uint8_t>(50_pct)  == static_cast<uint8_t>(Percentage(50)));
    REQUIRE(static_cast<uint8_t>(100_pct) == static_cast<uint8_t>(Percentage(100)));

    REQUIRE(static_cast<uint16_t>(0_pct)   == static_cast<uint16_t>(Percentage(0)));
    REQUIRE(static_cast<uint16_t>(50_pct)  == static_cast<uint16_t>(Percentage(50)));
    REQUIRE(static_cast<uint16_t>(100_pct) == static_cast<uint16_t>(Percentage(100)));
}

TEST_CASE("scaleToNative - 8-bit to 8-bit (no scaling)", "[brightness]") {
    REQUIRE(scaleToNative<8>(static_cast<uint8_t>(0)) == 0);
    REQUIRE(scaleToNative<8>(static_cast<uint8_t>(128)) == 128);
    REQUIRE(scaleToNative<8>(static_cast<uint8_t>(255)) == 255);
}

TEST_CASE("scaleToNative - 8-bit to 10-bit (upscaling)", "[brightness]") {
    REQUIRE(scaleToNative<10>(static_cast<uint8_t>(0)) == 0);
    REQUIRE(scaleToNative<10>(static_cast<uint8_t>(1)) == 4);      // 1 << 2
    REQUIRE(scaleToNative<10>(static_cast<uint8_t>(128)) == 514);	// not 512 because of "bit replication"
    REQUIRE(scaleToNative<10>(static_cast<uint8_t>(254)) == 1019);
    // Special case: 255 maps to full brightness (1023, not 1020)
    REQUIRE(scaleToNative<10>(static_cast<uint8_t>(255)) == 1023);
}

TEST_CASE("scaleToNative - 8-bit to 12-bit (upscaling)", "[brightness]") {
    REQUIRE(scaleToNative<12>(static_cast<uint8_t>(0)) == 0);
    REQUIRE(scaleToNative<12>(static_cast<uint8_t>(1)) == 16);     // 1 << 4
    REQUIRE(scaleToNative<12>(static_cast<uint8_t>(128)) == 2056);
    REQUIRE(scaleToNative<12>(static_cast<uint8_t>(254)) == 4079);
    // Special case: 255 maps to full brightness (4095, not 4080)
    REQUIRE(scaleToNative<12>(static_cast<uint8_t>(255)) == 4095);
}

TEST_CASE("scaleToNative - 8-bit to 13-bit (upscaling)", "[brightness]") {
    REQUIRE(scaleToNative<13>(static_cast<uint8_t>(0)) == 0);
    REQUIRE(scaleToNative<13>(static_cast<uint8_t>(1)) == 32);     // 1 << 5
    REQUIRE(scaleToNative<13>(static_cast<uint8_t>(128)) == 4112);
    REQUIRE(scaleToNative<13>(static_cast<uint8_t>(254)) == 8159);
    // Special case: 255 maps to full brightness (8191, not 8160)
    REQUIRE(scaleToNative<13>(static_cast<uint8_t>(255)) == 8191);
}

TEST_CASE("scaleToNative - 8-bit to 16-bit (upscaling)", "[brightness]") {
    REQUIRE(scaleToNative<16>(static_cast<uint8_t>(0)) == 0);
    REQUIRE(scaleToNative<16>(static_cast<uint8_t>(1)) == 257);
    REQUIRE(scaleToNative<16>(static_cast<uint8_t>(128)) == 32896);
    REQUIRE(scaleToNative<16>(static_cast<uint8_t>(254)) == 65278);
    // Special case: 255 maps to full brightness (65535, not 65280)
    REQUIRE(scaleToNative<16>(static_cast<uint8_t>(255)) == 65535);
}

TEST_CASE("scaleToNative - 8-bit to 4-bit (downscaling)", "[brightness]") {
    REQUIRE(scaleToNative<4>(static_cast<uint8_t>(0)) == 0);
    REQUIRE(scaleToNative<4>(static_cast<uint8_t>(16)) == 1);    // 16 >> 4
    REQUIRE(scaleToNative<4>(static_cast<uint8_t>(128)) == 8);   // 128 >> 4
    REQUIRE(scaleToNative<4>(static_cast<uint8_t>(240)) == 15);  // 240 >> 4
    // Special case: 255 maps to full brightness (15, not 15)
    REQUIRE(scaleToNative<4>(static_cast<uint8_t>(255)) == 15);
}

TEST_CASE("scaleToNative - 8-bit to 1-bit (extreme downscaling)", "[brightness]") {
    REQUIRE(scaleToNative<1>(static_cast<uint8_t>(0)) == 0);
    REQUIRE(scaleToNative<1>(static_cast<uint8_t>(1)) == 0);     // 1 >> 7 = 0
    REQUIRE(scaleToNative<1>(static_cast<uint8_t>(127)) == 0);   // 127 >> 7 = 0
    REQUIRE(scaleToNative<1>(static_cast<uint8_t>(128)) == 1);   // 128 >> 7 = 1
    REQUIRE(scaleToNative<1>(static_cast<uint8_t>(254)) == 1);   // 254 >> 7 = 1
    // Special case: 255 maps to full brightness (1)
    REQUIRE(scaleToNative<1>(static_cast<uint8_t>(255)) == 1);
}

TEST_CASE("scaleToNative - 16-bit to 16-bit (no scaling)", "[brightness]") {
    REQUIRE(scaleToNative<16>(static_cast<uint16_t>(0)) == 0);
    REQUIRE(scaleToNative<16>(static_cast<uint16_t>(32768)) == 32768);
    REQUIRE(scaleToNative<16>(static_cast<uint16_t>(65535)) == 65535);
}

TEST_CASE("scaleToNative - 16-bit to 8-bit (downscaling)", "[brightness]") {
    REQUIRE(scaleToNative<8>(static_cast<uint16_t>(0)) == 0);
    REQUIRE(scaleToNative<8>(static_cast<uint16_t>(256)) == 1);      // 256 >> 8
    REQUIRE(scaleToNative<8>(static_cast<uint16_t>(32768)) == 128);  // 32768 >> 8
    REQUIRE(scaleToNative<8>(static_cast<uint16_t>(65280)) == 255);  // 65280 >> 8
    // Special case: 65535 maps to full brightness (255)
    REQUIRE(scaleToNative<8>(static_cast<uint16_t>(65535)) == 255);
}

TEST_CASE("scaleToNative - 16-bit to 10-bit (downscaling)", "[brightness]") {
    REQUIRE(scaleToNative<10>(static_cast<uint16_t>(0)) == 0);
    REQUIRE(scaleToNative<10>(static_cast<uint16_t>(64)) == 1);       // 64 >> 6
    REQUIRE(scaleToNative<10>(static_cast<uint16_t>(32768)) == 512);  // 32768 >> 6
    REQUIRE(scaleToNative<10>(static_cast<uint16_t>(65472)) == 1023);  // 65472 >> 6
    // Special case: 65535 maps to full brightness (1023)
    REQUIRE(scaleToNative<10>(static_cast<uint16_t>(65535)) == 1023);
}

TEST_CASE("scaleToNative - 16-bit to 13-bit (downscaling)", "[brightness]") {
    REQUIRE(scaleToNative<13>(static_cast<uint16_t>(0)) == 0);
    REQUIRE(scaleToNative<13>(static_cast<uint16_t>(8)) == 1);        // 8 >> 3
    REQUIRE(scaleToNative<13>(static_cast<uint16_t>(32768)) == 4096);  // 32768 >> 3
    REQUIRE(scaleToNative<13>(static_cast<uint16_t>(65528)) == 8191);  // 65528 >> 3
    // Special case: 65535 maps to full brightness (8191)
    REQUIRE(scaleToNative<13>(static_cast<uint16_t>(65535)) == 8191);
}

TEST_CASE("scaleToNative - 16-bit to 1-bit (extreme downscaling)", "[brightness]") {
    REQUIRE(scaleToNative<1>(static_cast<uint16_t>(0)) == 0);
    REQUIRE(scaleToNative<1>(static_cast<uint16_t>(32767)) == 0);  // 32767 >> 15 = 0
    REQUIRE(scaleToNative<1>(static_cast<uint16_t>(32768)) == 1);  // 32768 >> 15 = 1
    REQUIRE(scaleToNative<1>(static_cast<uint16_t>(65534)) == 1);  // 65534 >> 15 = 1
    // Special case: 65535 maps to full brightness (1)
    REQUIRE(scaleToNative<1>(static_cast<uint16_t>(65535)) == 1);
}

TEST_CASE("scaleToNative - edge case: full brightness always maps correctly", "[brightness]") {
    // 8-bit max (255) should map to target max for all resolutions
    REQUIRE(scaleToNative<1>(static_cast<uint8_t>(255)) == 1);
    REQUIRE(scaleToNative<4>(static_cast<uint8_t>(255)) == 15);
    REQUIRE(scaleToNative<8>(static_cast<uint8_t>(255)) == 255);
    REQUIRE(scaleToNative<10>(static_cast<uint8_t>(255)) == 1023);
    REQUIRE(scaleToNative<12>(static_cast<uint8_t>(255)) == 4095);
    REQUIRE(scaleToNative<13>(static_cast<uint8_t>(255)) == 8191);
    REQUIRE(scaleToNative<16>(static_cast<uint8_t>(255)) == 65535);

    // 16-bit max (65535) should map to target max for all resolutions
    REQUIRE(scaleToNative<1>(static_cast<uint16_t>(65535)) == 1);
    REQUIRE(scaleToNative<4>(static_cast<uint16_t>(65535)) == 15);
    REQUIRE(scaleToNative<8>(static_cast<uint16_t>(65535)) == 255);
    REQUIRE(scaleToNative<10>(static_cast<uint16_t>(65535)) == 1023);
    REQUIRE(scaleToNative<12>(static_cast<uint16_t>(65535)) == 4095);
    REQUIRE(scaleToNative<13>(static_cast<uint16_t>(65535)) == 8191);
    REQUIRE(scaleToNative<16>(static_cast<uint16_t>(65535)) == 65535);
}
