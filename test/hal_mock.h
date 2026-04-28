// a HAL mock for the JLed unit tests. Behaves like a JLed HAL but
// but can be intrumented & queried.
// Copyright 2017-2020 Jan Delgado jdelgado@gmx.net
//

#ifndef TEST_HAL_MOCK_H_
#define TEST_HAL_MOCK_H_

class HalMock {
 public:
    using PinType = uint8_t;

    HalMock() {}
    explicit HalMock(PinType pin) : pin_(pin) {}

    template<typename Brightness>
    void analogWrite(Brightness val) {
        val_ = static_cast<uint16_t>(val);
        pin_values()[pin_] = val_;
    }

    uint8_t  Pin()   const { return pin_; }
    uint16_t Value() const { return val_; }

    // Global pin-state table: allows reading back brightness from LEDs stored
    // inside type-erased containers (where GetHal() is not accessible).
    static uint16_t PinValue(uint8_t pin) { return pin_values()[pin]; }

    static void Init() {
        uint16_t* p = pin_values();
        for (int i = 0; i < 256; i++) p[i] = 0;
    }

 private:
    uint16_t val_ = 0;
    PinType  pin_ = 0;

    static uint16_t* pin_values() {
        static uint16_t values[256] = {};
        return values;
    }
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
