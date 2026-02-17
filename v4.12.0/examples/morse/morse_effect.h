// Copyright (c) 2019 Jan Delgado <jdelgado[at]gmx.net>
// https://github.com/jandelgado/jled
#ifndef EXAMPLES_MORSE_MORSE_EFFECT_H_
#define EXAMPLES_MORSE_MORSE_EFFECT_H_

#include <jled.h>
#include "morse.h"  // NOLINT

class MorseEffect : public jled::BrightnessEvaluator {
    Morse morse_;
    // duration of a single 'dit' in ms
    const uint16_t speed_;

 public:
    explicit MorseEffect(const char* message, uint16_t speed = 200)
        : morse_(message), speed_(speed) {}

    uint8_t Eval(uint32_t t) const override {
        const auto pos = t / speed_;
        if (pos >= morse_.size()) return 0;
        return morse_.test(pos) ? 255 : 0;
    }

    uint16_t Period() const override { return (morse_.size() + 1) * speed_; }
};

#endif  // EXAMPLES_MORSE_MORSE_EFFECT_H_
