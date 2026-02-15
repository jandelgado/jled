// JLed custom HAL example.
// Copyright 2019,2025 by Jan Delgado. All rights reserved.
// https://github.com/jandelgado/jled

#include <jled.h>

// a custom PWM HAL for the Arduino platform, inverting output.
// In general, a JLed HAL class must satisfy the following interface:
//
// class JLedHal {
//   public:
//     JLedHal(PinType pin);
//     void analogWrite(uint8_t val) const;
//  }
//
class CustomHal {
 public:
    using PinType = uint8_t;
    using NativeBrightness = uint8_t;
    static constexpr uint8_t kNativeBits = 8;

    explicit CustomHal(PinType pin) noexcept : pin_(pin) {}

    template<typename Brightness>
    void analogWrite(Brightness val) const {
        // some platforms, e.g. STM need lazy initialization
        if (!setup_) {
            ::pinMode(pin_, OUTPUT);
            setup_ = true;
        }
        // Scale to 8-bit and invert
        uint8_t val8;
        if (sizeof(Brightness) == 1) {
            val8 = val;
        } else {
            val8 = static_cast<uint8_t>(val >> 8);
        }
        ::analogWrite(pin_, 255 - val8);
    }

 private:
    mutable bool setup_ = false;
    PinType pin_;
};

// a custom JLed class using our CustomHal and the default clock defined
// for the platform.
class CustomJLed : public jled::TJLed<CustomHal, jled::JLedClockType, uint8_t, CustomJLed> {
    using jled::TJLed<CustomHal, jled::JLedClockType, uint8_t, CustomJLed>::TJLed;
};

// uses above defined CustomHal
auto led = CustomJLed(LED_BUILTIN).Blink(1000, 1000).Repeat(5);

void setup() {}

void loop() { led.Update(); }
