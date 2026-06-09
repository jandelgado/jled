// Copyright 2017-2026 Jan Delgado jdelgado@gmx.net
//
#ifndef TEST_MOCK_BRIGHTNESS_EVAL_H_
#define TEST_MOCK_BRIGHTNESS_EVAL_H_

#include <jled_base.h>  // NOLINT
#include <cstdint>
#include <cassert>
#include <vector>

// A brightness evaluator used for tests. Returns predefined values f(t)=y
// for each point in time t. Templated on brightness type to support both
// 8-bit (JLed) and 16-bit (JLedHD) LEDs.
template <typename B>
class MockBrightnessEvaluatorT : public jled::BrightnessEvaluator<B> {
    std::vector<B> values_;
    mutable uint16_t count_ = 0;

 public:
    explicit MockBrightnessEvaluatorT(std::vector<B> values) : values_(values) {}
    uint16_t TimesEvalWasCalled() const { return count_; }
    uint16_t Period() const { return values_.size(); }
    B Eval(uint32_t t) const {
        assert(t < values_.size());
        count_++;
        return values_[t];
    }
};

using MockBrightnessEvaluator = MockBrightnessEvaluatorT<uint8_t>;

#endif  // TEST_MOCK_BRIGHTNESS_EVAL_H_
