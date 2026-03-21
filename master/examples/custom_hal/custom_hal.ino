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

    explicit CustomHal(PinType pin) noexcept : pin_(pin) {}

    void analogWrite(uint8_t val) const {
        // some platforms, e.g. STM need lazy initialization
        if (!setup_) {
            ::pinMode(pin_, OUTPUT);
            setup_ = true;
        }
        ::analogWrite(pin_, 255 - val);
    }

 private:
    mutable bool setup_ = false;
    PinType pin_;
};


// a custom JLed class using our CustomHal and the default clock defined
// for the platform.
class CustomJLed : public jled::TJLed<CustomHal, jled::JLedClockType, CustomJLed> {
    using jled::TJLed<CustomHal, jled::JLedClockType, CustomJLed>::TJLed;
};

// uses above defined CustomHal
auto led = CustomJLed(LED_BUILTIN).Blink(1000, 1000).Repeat(5);

void setup() {}

void loop() { led.Update(); }
