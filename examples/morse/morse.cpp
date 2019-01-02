#include <Arduino.h>
#include <jled.h>
#include "morse.h"

class MorseEffect : public jled::BrightnessEvaluator {
    Morse morse_;
    // duration of a single 'dit' in ms
    const uint16_t speed_;

 public:
    MorseEffect(const char* message, uint16_t speed = 200)
        : morse_(message), speed_(speed) {}

    uint8_t Eval(uint32_t t) override {
        const auto pos = t/speed_;
        if (pos >= morse_.size()) return 0;
        return 255 * (morse_.test(pos) ? 1 : 0);
    }

    uint16_t Period() const override { return (morse_.size()+1) * speed_; }
};

MorseEffect morseEffect("HELLO JLED");
auto morseLed = JLed(LED_BUILTIN)
                    .UserFunc(&morseEffect)
                    .DelayAfter(2000)
                    .Forever();

void setup() {
}

void loop() { morseLed.Update(); }
