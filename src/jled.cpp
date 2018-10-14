#include <Arduino.h>

// pre-calculated fade-on function. This table samples the function
//   y(x) =  exp(sin((t - period / 2.) * PI / period)) - 0.36787944)
//   * 108.
// at x={0,32,...,256}. In FadeOnFunc() we us linear interpolation
// to
// approximate the original function (so we do not need fp-ops).
// fade-off and breath functions are all derived from fade-on, see
// below.
// (To save some additional bytes, we could place it in PROGMEM
// sometime)
static constexpr uint8_t kFadeOnTable[] = {0,   3,   13,  33, 68,
                                           118, 179, 232, 255};

// https://www.wolframalpha.com/input/?i=plot+(exp(sin((x-100%2F2.)*PI%2F100))-0.36787944)*108.0++x%3D0+to+100
// The fade-on func is an approximation of
//   y(x) = exp(sin((t-period/2.) * PI / period)) - 0.36787944) * 108.)
uint8_t jled_fadeon_func(uint32_t t, uint16_t period) {
    if (t + 1 >= period) return 255;  // kFullBrightness;

    // approximate by linear interpolation.
    // scale t according to period to 0..255
    t = ((t << 8) / period) & 0xff;
    const auto i = (t >> 5);  // -> i will be in range 0 .. 7
    const auto y0 = kFadeOnTable[i];
    const auto y1 = kFadeOnTable[i + 1];
    const auto x0 = i << 5;  // *32

    // y(t) = mt+b, with m = dy/dx = (y1-y0)/32 = (y1-y0) >> 5
    return (((t - x0) * (y1 - y0)) >> 5) + y0;
}
