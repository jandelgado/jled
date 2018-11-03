// JLed Unit tests  (run on host).
// Copyright 2017 Jan Delgado jdelgado@gmx.net
#include "catch.hpp"
#include <esp8266_hal.h>  // NOLINT

using jled::Esp8266Hal;

TEST_CASE("properly scale 8bit to 10bit for ESP8266 support",
          "[esp8266_analog_writer]") {
  class TestableWriter : public Esp8266Hal {
   public:
    static void test() {
      REQUIRE(TestableWriter::ScaleTo10Bit(0) == 0);
      REQUIRE(TestableWriter::ScaleTo10Bit(127) == (127 << 2) + 3);
      REQUIRE(TestableWriter::ScaleTo10Bit(255) == 1023);
    }
  };
  TestableWriter::test();
}

TEST_CASE("analogWrite() writes correct value", "[esp8266_analog_writer]") {
    arduinoMockInit();

    constexpr auto kPin = 10;
    auto aw = Esp8266Hal(kPin);

    aw.analogWrite(123);

    // expect the value to be scaled to 10bit written to port
    REQUIRE(arduinoMockGetPinState(kPin) == (123<<2)+3);
}

TEST_CASE("millis() returns correct time", "[esp8266_hal]") {
    arduinoMockInit();
    auto h = Esp8266Hal(1);
    REQUIRE(h.millis() == 0);
    arduinoMockSetMillis(99);
    REQUIRE(h.millis() == 99);
}

