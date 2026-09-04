// Unit tests for WriterHal.
// Copyright 2026 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled
#include "catch2/catch_amalgamated.hpp"
#include "invertable_hal.h"  // NOLINT
#include "value_rgb.h"       // NOLINT
#include "writer_hal.h"      // NOLINT

using jled::InvertableHal;
using jled::RGBColor;
using jled::WriterHal;

// Mirrors FastLED's CRGB: plain r/g/b aggregate, the external target
// WriterHal writes to.
struct MockCRGB {
    uint8_t r, g, b;
};

class MockWriter {
 public:
    static void Write(RGBColor<uint8_t> val, MockCRGB* target) {
        target->r = val.r;
        target->g = val.g;
        target->b = val.b;
    }
};

using MockWriterHal = WriterHal<RGBColor<uint8_t>, MockCRGB, MockWriter>;

TEST_CASE("jled::WriterHal::analogWrite forwards values to the target unchanged", "[writer_hal]") {
    MockCRGB target{};
    MockWriterHal hal(&target);

    hal.analogWrite(RGBColor<uint8_t>{10, 20, 30});

    CHECK(target.r == 10);
    CHECK(target.g == 20);
    CHECK(target.b == 30);
}

// jled.h exports InvertableHal<jled::WriterHal<...>> as the global WriterHal
// alias; this is that composition, built directly from the two headers above.
TEST_CASE("InvertableHal<jled::WriterHal>::analogWrite inverts each channel when inverted",
          "[writer_hal]") {
    MockCRGB target{};
    InvertableHal<MockWriterHal> hal(&target);

    hal.analogWrite(RGBColor<uint8_t>{10, 20, 30}, true);

    CHECK(target.r == 245);
    CHECK(target.g == 235);
    CHECK(target.b == 225);
}
