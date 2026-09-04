// Copyright (c) 2017-2026 Jan Delgado <jdelgado[at]gmx.net>
// https://github.com/jandelgado/jled
//
// Unit tests for scale, lerp, lerp_ordered, scale_video, lerp_video,
// lerp_ordered_video (scale_lerp.h)
//
#include <catch2/catch_amalgamated.hpp>

#include "value_scalar.h"  // NOLINT ValueTraits<uint8_t/uint16_t>, needed to instantiate lerp<T>

using jled::lerp;
using jled::lerp_ordered;
using jled::lerp_ordered_video;
using jled::lerp_video;
using jled::scale;
using jled::scale8;
using jled::scale_video;

TEST_CASE("scale - uint8_t", "[scale]") {
    REQUIRE(scale<uint8_t>(0, 0) == 0);
    REQUIRE(scale<uint8_t>(255, 0) == 0);
    REQUIRE(scale<uint8_t>(0, 128) == 0);
    REQUIRE(scale<uint8_t>(100, 128) == 50);
    REQUIRE(scale<uint8_t>(255, 128) == 128);
    // factor == max: scale(x, max) == x
    REQUIRE(scale<uint8_t>(0, 255) == 0);
    REQUIRE(scale<uint8_t>(127, 255) == 127);
    REQUIRE(scale<uint8_t>(255, 255) == 255);
}

TEST_CASE("scale - uint16_t", "[scale]") {
    REQUIRE(scale<uint16_t>(0, 0) == 0);
    // factor == max: scale(x, max) == x
    REQUIRE(scale<uint16_t>(1000, 65535) == 1000);
    REQUIRE(scale<uint16_t>(65535, 65535) == 65535);
    // (1000 * 30001) >> 16
    REQUIRE(scale<uint16_t>(1000, 30000) == 457);
}

TEST_CASE("scale8 - alias for scale<uint8_t>", "[scale]") {
    REQUIRE(scale8(100, 128) == scale<uint8_t>(100, 128));
}

TEST_CASE("lerp - fast path: full range [0, kMax] returns val unchanged", "[lerp]") {
    REQUIRE(lerp<uint8_t>(0, 0, 255) == 0);
    REQUIRE(lerp<uint8_t>(128, 0, 255) == 128);
    REQUIRE(lerp<uint8_t>(255, 0, 255) == 255);
}

TEST_CASE("lerp - general case: [a, b] sub-range, b >= a", "[lerp]") {
    REQUIRE(lerp<uint8_t>(0, 50, 200) == 50);
    REQUIRE(lerp<uint8_t>(255, 50, 200) == 200);
    // delta = 150, scale(21, 150) = (21*151)>>8 = 12
    REQUIRE(lerp<uint8_t>(21, 50, 200) == 62);
}

TEST_CASE("lerp - uint16_t general case", "[lerp]") {
    REQUIRE(lerp<uint16_t>(0, 5000, 40000) == 5000);
    REQUIRE(lerp<uint16_t>(65535, 5000, 40000) == 40000);
    // delta = 35000, scale(1000, 35000) = (1000*35001)>>16 = 534
    REQUIRE(lerp<uint16_t>(1000, 5000, 40000) == 5534);
}

TEST_CASE("lerp_ordered - ascending (b >= a) delegates to lerp", "[lerp]") {
    REQUIRE(lerp_ordered<uint8_t>(0, 50, 200) == 50);
    REQUIRE(lerp_ordered<uint8_t>(255, 50, 200) == 200);
    REQUIRE(lerp_ordered<uint8_t>(21, 50, 200) == 62);

    // a == b: delta is 0, result is a regardless of alpha
    REQUIRE(lerp_ordered<uint8_t>(0, 100, 100) == 100);
    REQUIRE(lerp_ordered<uint8_t>(255, 100, 100) == 100);
}

TEST_CASE("lerp_ordered - descending (b < a) interpolates in reverse", "[lerp]") {
    REQUIRE(lerp_ordered<uint8_t>(0, 200, 50) == 200);
    REQUIRE(lerp_ordered<uint8_t>(255, 200, 50) == 50);
    REQUIRE(lerp_ordered<uint8_t>(21, 200, 50) == 188);
}

TEST_CASE("scale_video - uint8_t: like scale, but never floors a nonzero val to 0", "[scale]") {
    REQUIRE(scale_video<uint8_t>(0, 0) == 0);
    REQUIRE(scale_video<uint8_t>(255, 0) == 0);  // factor == 0 stays 0, even for nonzero val
    REQUIRE(scale_video<uint8_t>(0, 128) == 0);  // val == 0 stays 0
    REQUIRE(scale_video<uint8_t>(100, 128) == 50);
    REQUIRE(scale_video<uint8_t>(255, 255) == 255);
    // scale(1, 1) == 0 (would floor to black); scale_video keeps it lit
    REQUIRE(scale<uint8_t>(1, 1) == 0);
    REQUIRE(scale_video<uint8_t>(1, 1) == 1);
    // green channel (69) of orange-red at the first step of a fade: scale()
    // floors it to 0 while red (255) is already visible - scale_video keeps
    // it lit instead
    REQUIRE(scale<uint8_t>(69, 1) == 0);
    REQUIRE(scale_video<uint8_t>(69, 1) == 1);
}

TEST_CASE("scale_video - uint16_t", "[scale]") {
    REQUIRE(scale_video<uint16_t>(0, 0) == 0);
    REQUIRE(scale_video<uint16_t>(65535, 65535) == 65535);
    REQUIRE(scale<uint16_t>(1, 1) == 0);
    REQUIRE(scale_video<uint16_t>(1, 1) == 1);
}

TEST_CASE("lerp_video - fast path: full range [0, kMax] returns val unchanged", "[lerp]") {
    REQUIRE(lerp_video<uint8_t>(0, 0, 255) == 0);
    REQUIRE(lerp_video<uint8_t>(128, 0, 255) == 128);
    REQUIRE(lerp_video<uint8_t>(255, 0, 255) == 255);
}

TEST_CASE("lerp_video - keeps a small-delta channel lit instead of flooring to a", "[lerp]") {
    REQUIRE(lerp<uint8_t>(1, 0, 69) == 0);        // plain lerp floors to a (=0)
    REQUIRE(lerp_video<uint8_t>(1, 0, 69) == 1);  // lerp_video keeps it lit
    REQUIRE(lerp_video<uint8_t>(255, 0, 69) == 69);
}

TEST_CASE("lerp_ordered_video - ascending (b >= a) delegates to lerp_video", "[lerp]") {
    REQUIRE(lerp_ordered_video<uint8_t>(1, 0, 69) == 1);
    REQUIRE(lerp_ordered_video<uint8_t>(255, 0, 69) == 69);
    // a == b: delta is 0, result is a regardless of alpha
    REQUIRE(lerp_ordered_video<uint8_t>(0, 100, 100) == 100);
    REQUIRE(lerp_ordered_video<uint8_t>(255, 100, 100) == 100);
}

TEST_CASE("lerp_ordered_video - descending (b < a) keeps a small-delta channel lit", "[lerp]") {
    REQUIRE(lerp_ordered<uint8_t>(1, 69, 0) == 69);        // plain lerp_ordered stays at a
    REQUIRE(lerp_ordered_video<uint8_t>(1, 69, 0) == 68);  // lerp_ordered_video moves off it
}
