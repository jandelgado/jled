// Copyright (c) 2026 Jan Delgado <jdelgado[at]gmx.net>
// https://github.com/jandelgado/jled
#pragma once

namespace jled {

// a HAL that writes its Value values via a user-provided Writer to an
// target (e.g. FastLED's CRGB[]), or to whatever needs the
// calculated value. See the fastled example for how this can be used.
// Plain single-argument analogWrite(Value): has no native inversion
// capability. jled.h exports the ready-to-use, inverting composition as the
// global WriterHal alias (InvertableHal<jled::WriterHal<...>>); use this
// class directly only if you need the uninverted building block.
template<typename Value, typename Target, typename Writer>
class WriterHal {
    Writer writer_;
    Target* target_;

 public:
    using PinType = Target*;

    explicit WriterHal(Target* target) : target_(target) {}

    template<typename V>
    void analogWrite(V val) const {
        writer_.Write(val, target_);
    }
};

}  // namespace jled
