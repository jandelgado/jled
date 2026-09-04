// Copyright (c) 2025-2026 Jan Delgado <jdelgado[at]gmx.net>
// https://github.com/jandelgado/jled
//
// Unit tests for scale_bit_depth (scale_bit_depth.h)
//
#include <catch2/catch_amalgamated.hpp>

#include "scale_bit_depth.h"  // NOLINT

using jled::scale_bit_depth;

TEST_CASE("scale_bit_depth - 8-bit to 8-bit (no scaling)", "[scale_bit_depth]") {
    REQUIRE(scale_bit_depth<8>(static_cast<uint8_t>(0)) == 0);
    REQUIRE(scale_bit_depth<8>(static_cast<uint8_t>(128)) == 128);
    REQUIRE(scale_bit_depth<8>(static_cast<uint8_t>(255)) == 255);
}

TEST_CASE("scale_bit_depth - 8-bit to 10-bit (upscaling)", "[scale_bit_depth]") {
    REQUIRE(scale_bit_depth<10>(static_cast<uint8_t>(0)) == 0);
    REQUIRE(scale_bit_depth<10>(static_cast<uint8_t>(1)) == 4);  // 1 << 2
    // not 512 because of "bit replication"
    REQUIRE(scale_bit_depth<10>(static_cast<uint8_t>(128)) == 514);
    REQUIRE(scale_bit_depth<10>(static_cast<uint8_t>(254)) == 1019);
    // Special case: 255 maps to full brightness (1023, not 1020)
    REQUIRE(scale_bit_depth<10>(static_cast<uint8_t>(255)) == 1023);
}

TEST_CASE("scale_bit_depth - 8-bit to 12-bit (upscaling)", "[scale_bit_depth]") {
    REQUIRE(scale_bit_depth<12>(static_cast<uint8_t>(0)) == 0);
    REQUIRE(scale_bit_depth<12>(static_cast<uint8_t>(1)) == 16);  // 1 << 4
    REQUIRE(scale_bit_depth<12>(static_cast<uint8_t>(128)) == 2056);
    REQUIRE(scale_bit_depth<12>(static_cast<uint8_t>(254)) == 4079);
    // Special case: 255 maps to full brightness (4095, not 4080)
    REQUIRE(scale_bit_depth<12>(static_cast<uint8_t>(255)) == 4095);
}

TEST_CASE("scale_bit_depth - 8-bit to 13-bit (upscaling)", "[scale_bit_depth]") {
    REQUIRE(scale_bit_depth<13>(static_cast<uint8_t>(0)) == 0);
    REQUIRE(scale_bit_depth<13>(static_cast<uint8_t>(1)) == 32);  // 1 << 5
    REQUIRE(scale_bit_depth<13>(static_cast<uint8_t>(128)) == 4112);
    REQUIRE(scale_bit_depth<13>(static_cast<uint8_t>(254)) == 8159);
    // Special case: 255 maps to full brightness (8191, not 8160)
    REQUIRE(scale_bit_depth<13>(static_cast<uint8_t>(255)) == 8191);
}

TEST_CASE("scale_bit_depth - 8-bit to 16-bit (upscaling)", "[scale_bit_depth]") {
    REQUIRE(scale_bit_depth<16>(static_cast<uint8_t>(0)) == 0);
    REQUIRE(scale_bit_depth<16>(static_cast<uint8_t>(1)) == 257);
    REQUIRE(scale_bit_depth<16>(static_cast<uint8_t>(128)) == 32896);
    REQUIRE(scale_bit_depth<16>(static_cast<uint8_t>(254)) == 65278);
    // Special case: 255 maps to full brightness (65535, not 65280)
    REQUIRE(scale_bit_depth<16>(static_cast<uint8_t>(255)) == 65535);
}

TEST_CASE("scale_bit_depth - 8-bit to 4-bit (downscaling)", "[scale_bit_depth]") {
    REQUIRE(scale_bit_depth<4>(static_cast<uint8_t>(0)) == 0);
    REQUIRE(scale_bit_depth<4>(static_cast<uint8_t>(16)) == 1);    // 16 >> 4
    REQUIRE(scale_bit_depth<4>(static_cast<uint8_t>(128)) == 8);   // 128 >> 4
    REQUIRE(scale_bit_depth<4>(static_cast<uint8_t>(240)) == 15);  // 240 >> 4
    // Special case: 255 maps to full brightness (15, not 15)
    REQUIRE(scale_bit_depth<4>(static_cast<uint8_t>(255)) == 15);
}

TEST_CASE("scale_bit_depth - 8-bit to 1-bit (extreme downscaling)", "[scale_bit_depth]") {
    REQUIRE(scale_bit_depth<1>(static_cast<uint8_t>(0)) == 0);
    REQUIRE(scale_bit_depth<1>(static_cast<uint8_t>(1)) == 0);    // 1 >> 7 = 0
    REQUIRE(scale_bit_depth<1>(static_cast<uint8_t>(127)) == 0);  // 127 >> 7 = 0
    REQUIRE(scale_bit_depth<1>(static_cast<uint8_t>(128)) == 1);  // 128 >> 7 = 1
    REQUIRE(scale_bit_depth<1>(static_cast<uint8_t>(254)) == 1);  // 254 >> 7 = 1
    // Special case: 255 maps to full brightness (1)
    REQUIRE(scale_bit_depth<1>(static_cast<uint8_t>(255)) == 1);
}

TEST_CASE("scale_bit_depth - 16-bit to 16-bit (no scaling)", "[scale_bit_depth]") {
    REQUIRE(scale_bit_depth<16>(static_cast<uint16_t>(0)) == 0);
    REQUIRE(scale_bit_depth<16>(static_cast<uint16_t>(32768)) == 32768);
    REQUIRE(scale_bit_depth<16>(static_cast<uint16_t>(65535)) == 65535);
}

TEST_CASE("scale_bit_depth - 16-bit to 8-bit (downscaling)", "[scale_bit_depth]") {
    REQUIRE(scale_bit_depth<8>(static_cast<uint16_t>(0)) == 0);
    REQUIRE(scale_bit_depth<8>(static_cast<uint16_t>(256)) == 1);      // 256 >> 8
    REQUIRE(scale_bit_depth<8>(static_cast<uint16_t>(32768)) == 128);  // 32768 >> 8
    REQUIRE(scale_bit_depth<8>(static_cast<uint16_t>(65280)) == 255);  // 65280 >> 8
    // Special case: 65535 maps to full brightness (255)
    REQUIRE(scale_bit_depth<8>(static_cast<uint16_t>(65535)) == 255);
}

TEST_CASE("scale_bit_depth - 16-bit to 10-bit (downscaling)", "[scale_bit_depth]") {
    REQUIRE(scale_bit_depth<10>(static_cast<uint16_t>(0)) == 0);
    REQUIRE(scale_bit_depth<10>(static_cast<uint16_t>(64)) == 1);        // 64 >> 6
    REQUIRE(scale_bit_depth<10>(static_cast<uint16_t>(32768)) == 512);   // 32768 >> 6
    REQUIRE(scale_bit_depth<10>(static_cast<uint16_t>(65472)) == 1023);  // 65472 >> 6
    // Special case: 65535 maps to full brightness (1023)
    REQUIRE(scale_bit_depth<10>(static_cast<uint16_t>(65535)) == 1023);
}

TEST_CASE("scale_bit_depth - 16-bit to 13-bit (downscaling)", "[scale_bit_depth]") {
    REQUIRE(scale_bit_depth<13>(static_cast<uint16_t>(0)) == 0);
    REQUIRE(scale_bit_depth<13>(static_cast<uint16_t>(8)) == 1);         // 8 >> 3
    REQUIRE(scale_bit_depth<13>(static_cast<uint16_t>(32768)) == 4096);  // 32768 >> 3
    REQUIRE(scale_bit_depth<13>(static_cast<uint16_t>(65528)) == 8191);  // 65528 >> 3
    // Special case: 65535 maps to full brightness (8191)
    REQUIRE(scale_bit_depth<13>(static_cast<uint16_t>(65535)) == 8191);
}

TEST_CASE("scale_bit_depth - 16-bit to 1-bit (extreme downscaling)", "[scale_bit_depth]") {
    REQUIRE(scale_bit_depth<1>(static_cast<uint16_t>(0)) == 0);
    REQUIRE(scale_bit_depth<1>(static_cast<uint16_t>(32767)) == 0);  // 32767 >> 15 = 0
    REQUIRE(scale_bit_depth<1>(static_cast<uint16_t>(32768)) == 1);  // 32768 >> 15 = 1
    REQUIRE(scale_bit_depth<1>(static_cast<uint16_t>(65534)) == 1);  // 65534 >> 15 = 1
    // Special case: 65535 maps to full brightness (1)
    REQUIRE(scale_bit_depth<1>(static_cast<uint16_t>(65535)) == 1);
}

TEST_CASE("scale_bit_depth - edge case: full brightness always maps correctly",
          "[scale_bit_depth]") {
    // 8-bit max (255) should map to target max for all resolutions
    REQUIRE(scale_bit_depth<1>(static_cast<uint8_t>(255)) == 1);
    REQUIRE(scale_bit_depth<4>(static_cast<uint8_t>(255)) == 15);
    REQUIRE(scale_bit_depth<8>(static_cast<uint8_t>(255)) == 255);
    REQUIRE(scale_bit_depth<10>(static_cast<uint8_t>(255)) == 1023);
    REQUIRE(scale_bit_depth<12>(static_cast<uint8_t>(255)) == 4095);
    REQUIRE(scale_bit_depth<13>(static_cast<uint8_t>(255)) == 8191);
    REQUIRE(scale_bit_depth<16>(static_cast<uint8_t>(255)) == 65535);

    // 16-bit max (65535) should map to target max for all resolutions
    REQUIRE(scale_bit_depth<1>(static_cast<uint16_t>(65535)) == 1);
    REQUIRE(scale_bit_depth<4>(static_cast<uint16_t>(65535)) == 15);
    REQUIRE(scale_bit_depth<8>(static_cast<uint16_t>(65535)) == 255);
    REQUIRE(scale_bit_depth<10>(static_cast<uint16_t>(65535)) == 1023);
    REQUIRE(scale_bit_depth<12>(static_cast<uint16_t>(65535)) == 4095);
    REQUIRE(scale_bit_depth<13>(static_cast<uint16_t>(65535)) == 8191);
    REQUIRE(scale_bit_depth<16>(static_cast<uint16_t>(65535)) == 65535);
}
