// JLed custom HAL example.
// Copyright 2019-2026 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled

#include <jled.h>

// a custom PWM HAL for the Arduino platform. In general, a JLed HAL class
// must satisfy the following interface:
//
// class JLedHal {
//   public:
//     JLedHal(PinType pin);
//
//     template<typename Brightness>
//     void analogWrite(Brightness val, bool invert) const;
//
//     void SetLowActive(bool invert) const;
//  }
//
// analogWrite() receives an effect-space brightness value and whether the
// connected LED is configured via LowActive(). It is entirely up to the HAL
// how (or whether) it applies the inversion. This example applies it in
// software, the same way jled::InvertableHal<Hal> does for any HAL that has
// no native inversion support. Other HALs could do it in hardware.
class CustomHal {
 public:
    using PinType = uint8_t;

    explicit CustomHal(PinType pin) noexcept : pin_(pin) {}

    template<typename Brightness>
    void analogWrite(Brightness val, bool invert) const {
        // some platforms, e.g. STM need lazy initialization
        if (!setup_) {
            ::pinMode(pin_, OUTPUT);
            setup_ = true;
        }
        // Scale to 8-bit
        uint8_t val8;
        if (sizeof(Brightness) == 1) {
            val8 = val;
        } else {
            val8 = static_cast<uint8_t>(val >> 8);
        }
        ::analogWrite(pin_, invert ? 255 - val8 : val8);
    }

    // No hardware polarity register on this platform; invert is already
    // applied above on every analogWrite(), so there is nothing to do here.
    void SetLowActive(bool) const {}

 private:
    mutable bool setup_ = false;
    PinType pin_;
};

// a custom JLed class using our CustomHal and the default clock defined
// for the platform.
class CustomJLed : public jled::TJLed<CustomHal, jled::JLedClockType, uint8_t, CustomJLed> {
    using jled::TJLed<CustomHal, jled::JLedClockType, uint8_t, CustomJLed>::TJLed;
};

// uses above defined CustomHal; LowActive() now works correctly since
// CustomHal applies the invert flag CustomJLed passes it.
auto led = CustomJLed(LED_BUILTIN).Blink(1000, 1000).Repeat(5).LowActive();

void setup() {}

void loop() {
    led.Update();
}
