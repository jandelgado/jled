// Copyright 2017-2020 Jan Delgado jdelgado@gmx.net
//
#ifndef TEST_MOCK_BRIGHTNESS_EVAL_H_
#define TEST_MOCK_BRIGHTNESS_EVAL_H_

#include <jled_base.h>  // NOLINT
#include <cstdint>
#include <cassert>
#include <vector>

using BrightnessVec = std::vector<uint16_t>;

// a brightness evaluator used for the test. returns predefined values f(t)=y
// for each point in time t.
class MockBrightnessEvaluator : public jled::BrightnessEvaluator {
    BrightnessVec values_;
    mutable uint16_t count_ = 0;

 public:
    explicit MockBrightnessEvaluator(BrightnessVec values) : values_(values) {}
    uint16_t TimesEvalWasCalled() const { return count_; }
    uint16_t Period() const { return values_.size(); }
    uint16_t Eval(uint32_t t) const {
        assert(t < values_.size());
        count_++;
        return values_[t];
    }
};

#endif  // TEST_MOCK_BRIGHTNESS_EVAL_H_

