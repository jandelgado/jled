// a HAL mock for the JLed unit tests. Behaves like a JLed HAL but
// but can be intrumented & queried.
// Copyright 2017-2020 Jan Delgado jdelgado@gmx.net
//

#ifndef TEST_HAL_MOCK_H_
#define TEST_HAL_MOCK_H_

class HalMock {
 public:
    using PinType = uint8_t;
    using NativeBrightnessType = uint8_t;
    static constexpr uint8_t kNativeBits = 8;

    HalMock() {}
    explicit HalMock(PinType pin) : pin_(pin) {}

    template<typename BrightnessType>
    void analogWrite(BrightnessType val) {
        // For testing, always store as uint8_t (downscale if needed)
        // Use sizeof for compile-time optimization (optimizes same as if constexpr)
        if (sizeof(BrightnessType) == 1) {
            val_ = val;
        } else {
            val_ = static_cast<uint8_t>(val >> 8);
        }
    }

    uint8_t Pin() const { return pin_; }
    uint8_t Value() const { return val_; }

 private:
    uint8_t val_ = 0;
    PinType pin_ = 0;
};

class TimeMock {
 public:
    static uint32_t& millis() {
        static uint32_t millis_ = 0;
        return millis_;
    }
    static void set_millis(uint32_t t) { TimeMock::millis() = t; }
};

#endif  // TEST_HAL_MOCK_H_
