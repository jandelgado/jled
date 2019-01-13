// Copyright (c) 2019 Jan Delgado <jdelgado[at]gmx.net>
// https://github.com/jandelgado/jled
#ifndef EXAMPLES_MORSE_MORSE_EFFECT_H_
#define EXAMPLES_MORSE_MORSE_EFFECT_H_

#include "morse.h"  // NOLINT
#include <jled.h>

class MorseEffect : public jled::BrightnessEvaluator {
    Morse morse_;
    // duration of a single 'dit' in ms
    const uint16_t speed_;

 public:
    explicit MorseEffect(const char* message, uint16_t speed = 200)
        : morse_(message), speed_(speed) {}

    uint8_t Eval(uint32_t t) override {
        const auto pos = t / speed_;
        if (pos >= morse_.size()) return 0;
        return 255 * (morse_.test(pos) ? 1 : 0);
    }

    uint16_t Period() const override { return (morse_.size() + 1) * speed_; }
};

#endif  // EXAMPLES_MORSE_MORSE_EFFECT_H_
