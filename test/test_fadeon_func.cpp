// JLed Unit tests for the fadeon_func (runs on host).
// Copyright 2024-2025 Jan Delgado jdelgado@gmx.net
#include "catch2/catch_amalgamated.hpp"
#include "jled_base.h"

using jled::fadeon_func;
using jled::lut_lerp;

static uint16_t calc_inv_period(uint16_t period) {
    return static_cast<uint16_t>((1UL << 16) / period);
}

TEST_CASE("fadeon_func 8-bit edge cases and common values", "[fadeon]") {
    // With period=256, inv_period=256: tnorm = (t*256)>>8 = t exactly
    const uint16_t period = 256;
    const uint16_t inv_period = calc_inv_period(period);  // = 256

    SECTION("t=0 returns 0") {
        REQUIRE(fadeon_func<uint8_t>(0, period, inv_period) == 0);
    }

    SECTION("t=period-1 returns 255") {
        REQUIRE(fadeon_func<uint8_t>(period - 1, period, inv_period) == 255);
    }

    SECTION("LUT segment boundaries return exact LUT values") {
        // At tnorm = i*16 the interpolation fraction is 0, so result == lut8[i]
        // lut8 = {0,0,3,7,13,22,33,49,68,91,118,148,179,208,232,248,255}
        REQUIRE(fadeon_func<uint8_t>(32,  period, inv_period) == 3);    // lut8[2]
        REQUIRE(fadeon_func<uint8_t>(48,  period, inv_period) == 7);    // lut8[3]
        REQUIRE(fadeon_func<uint8_t>(128, period, inv_period) == 68);   // lut8[8]
        REQUIRE(fadeon_func<uint8_t>(192, period, inv_period) == 179);  // lut8[12]
        REQUIRE(fadeon_func<uint8_t>(240, period, inv_period) == 248);  // lut8[15]
    }

    SECTION("midpoint interpolation") {
        // t=24: tnorm=24, i=1, y0=0, y1=3, x0=16
        // result = (((24-16)*3)>>4) + 0 = (8*3)>>4 = 1
        REQUIRE(fadeon_func<uint8_t>(24, period, inv_period) == 1);
    }

    SECTION("output is monotonically non-decreasing") {
        uint8_t prev = 0;
        for (uint16_t t = 0; t < period; t++) {
            const auto val = fadeon_func<uint8_t>(t, period, inv_period);
            REQUIRE(val >= prev);
            prev = val;
        }
    }
}

TEST_CASE("lut_lerp with synthetic LUT", "[fadeon]") {
    // 5-entry uint8_t LUT: N-1=4 segments of width 64 in [0,256).
    // kNormShift=8, kSegShift=6. With period=256, inv_period=256:
    //   tnorm = (t * 256) >> 8 = t  (exact, no rounding)
    static constexpr uint8_t lut[] = {0, 64, 128, 192, 255};
    const uint16_t period = 256;
    const uint16_t inv_period = static_cast<uint16_t>((1UL << 16) / period);  // 256

    SECTION("t=0 returns lut[0]") { REQUIRE(lut_lerp(0, period, inv_period, lut) == 0); }

    SECTION("t=period-1 returns lut[4]") {
        REQUIRE(lut_lerp(period - 1, period, inv_period, lut) == 255);
    }

    SECTION("exact segment boundary t=64 returns lut[1]=64") {
        REQUIRE(lut_lerp(64, period, inv_period, lut) == 64);
    }

    SECTION("midpoint of first segment t=32 returns 32") {
        // i=0, tnorm=32, x0=0: ((32*(64-0))>>6)+0 = 32
        REQUIRE(lut_lerp(32, period, inv_period, lut) == 32);
    }

    SECTION("output is monotonically non-decreasing") {
        uint8_t prev = 0;
        for (uint16_t t = 0; t < period; t++) {
            const auto val = lut_lerp(t, period, inv_period, lut);
            REQUIRE(val >= prev);
            prev = val;
        }
    }
}

TEST_CASE("fadeon_func 16-bit edge cases and common values", "[fadeon]") {
    // With period=32768, inv_period=2: tnorm = t*2, segments align at even t
    const uint16_t period = 32768;
    const uint16_t inv_period = calc_inv_period(period);  // = 2

    SECTION("t=0 returns 0") {
        REQUIRE(fadeon_func<uint16_t>(0, period, inv_period) == 0);
    }

    SECTION("t=period-1 returns 65535") {
        REQUIRE(fadeon_func<uint16_t>(period - 1, period, inv_period) == 65535);
    }

    SECTION("LUT segment boundaries return exact LUT values") {
        // With inv_period=2, tnorm = t*2. Segments are 2048 wide.
        // At t=1024: tnorm=2048, i=1, y0=49, result=49
        // lut16 = {0,49,198,448,807,...,65535}
        REQUIRE(fadeon_func<uint16_t>(1024,  period, inv_period) == 49);    // lut16[1]
        REQUIRE(fadeon_func<uint16_t>(2048,  period, inv_period) == 198);   // lut16[2]
        REQUIRE(fadeon_func<uint16_t>(16384, period, inv_period) == 17545);  // lut16[16]
        REQUIRE(fadeon_func<uint16_t>(24576, period, inv_period) == 46081);  // lut16[24]
    }

    SECTION("output is monotonically non-decreasing") {
        uint16_t prev = 0;
        // Step by 64 to keep test fast (32768 iterations would be slow)
        for (uint16_t t = 0; t < period; t += 64) {
            const auto val = fadeon_func<uint16_t>(t, period, inv_period);
            REQUIRE(val >= prev);
            prev = val;
        }
    }
}
