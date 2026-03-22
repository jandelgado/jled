// JLed Unit tests for the fadeon_func (runs on host).
// Copyright 2024-2025 Jan Delgado jdelgado@gmx.net
#include "catch2/catch_amalgamated.hpp"
#include "jled_base.h"

using jled::fadeon_func;

static uint16_t calc_inv_period(uint16_t period) {
    return static_cast<uint16_t>((1UL << 16) / period);
}

TEST_CASE("fadeon_func 8-bit edge cases and common values", "[fadeon]") {
    // With period=256, inv_period=256: tnorm = (t*256)>>8 = t exactly
    const uint16_t period = 256;
    const uint16_t ip = calc_inv_period(period);  // = 256

    SECTION("t=0 returns 0") {
        REQUIRE(fadeon_func<uint8_t>(0, period, ip) == 0);
    }

    SECTION("t=period-1 returns 255") {
        REQUIRE(fadeon_func<uint8_t>(period - 1, period, ip) == 255);
    }

    SECTION("LUT segment boundaries return exact LUT values") {
        // At tnorm = i*16 the interpolation fraction is 0, so result == lut8[i]
        // lut8 = {0,0,3,7,13,22,33,49,68,91,118,148,179,208,232,248,255}
        REQUIRE(fadeon_func<uint8_t>(32,  period, ip) == 3);    // lut8[2]
        REQUIRE(fadeon_func<uint8_t>(48,  period, ip) == 7);    // lut8[3]
        REQUIRE(fadeon_func<uint8_t>(128, period, ip) == 68);   // lut8[8]
        REQUIRE(fadeon_func<uint8_t>(192, period, ip) == 179);  // lut8[12]
        REQUIRE(fadeon_func<uint8_t>(240, period, ip) == 248);  // lut8[15]
    }

    SECTION("midpoint interpolation") {
        // t=24: tnorm=24, i=1, y0=0, y1=3, x0=16
        // result = (((24-16)*3)>>4) + 0 = (8*3)>>4 = 1
        REQUIRE(fadeon_func<uint8_t>(24, period, ip) == 1);
    }

    SECTION("output is monotonically non-decreasing") {
        uint8_t prev = 0;
        for (uint16_t t = 0; t < period; t++) {
            const auto val = fadeon_func<uint8_t>(t, period, ip);
            REQUIRE(val >= prev);
            prev = val;
        }
    }
}

TEST_CASE("fadeon_func 16-bit edge cases and common values", "[fadeon]") {
    // With period=32768, inv_period=2: tnorm = t*2, segments align at even t
    const uint16_t period = 32768;
    const uint16_t ip = calc_inv_period(period);  // = 2

    SECTION("t=0 returns 0") {
        REQUIRE(fadeon_func<uint16_t>(0, period, ip) == 0);
    }

    SECTION("t=period-1 returns 65535") {
        REQUIRE(fadeon_func<uint16_t>(period - 1, period, ip) == 65535);
    }

    SECTION("LUT segment boundaries return exact LUT values") {
        // With inv_period=2, tnorm = t*2. Segments are 2048 wide.
        // At t=1024: tnorm=2048, i=1, y0=49, result=49
        // lut16 = {0,49,198,448,807,...,65535}
        REQUIRE(fadeon_func<uint16_t>(1024,  period, ip) == 49);    // lut16[1]
        REQUIRE(fadeon_func<uint16_t>(2048,  period, ip) == 198);   // lut16[2]
        REQUIRE(fadeon_func<uint16_t>(16384, period, ip) == 17545);  // lut16[16]
        REQUIRE(fadeon_func<uint16_t>(24576, period, ip) == 46081);  // lut16[24]
    }

    SECTION("output is monotonically non-decreasing") {
        uint16_t prev = 0;
        // Step by 64 to keep test fast (32768 iterations would be slow)
        for (uint16_t t = 0; t < period; t += 64) {
            const auto val = fadeon_func<uint16_t>(t, period, ip);
            REQUIRE(val >= prev);
            prev = val;
        }
    }
}
