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

    void analogWrite(uint8_t val) { val_ = val; }
    time_t millis() const { return millis_; }

    // mock functions
    void SetMillis(time_t millis) { millis_ = millis; }
    uint8_t Pin() const { return pin_; }
    uint8_t Value() const { return val_; }

 private:
    time_t millis_ = 0;
    uint8_t val_ = 0;
    PinType pin_ = 0;
};

#endif  // TEST_HAL_MOCK_H_
