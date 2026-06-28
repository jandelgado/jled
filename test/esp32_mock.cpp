// Copyright 2024-2025 Jan Delgado jdelgado@gmx.net
#include "esp32_mock.h"
#include <cassert>

Esp32State* gState_ = nullptr;

void esp32MockSetInstance(Esp32State* state) {
    gState_ = state;
    if (state) {
        *state = Esp32State{};
    }
}
