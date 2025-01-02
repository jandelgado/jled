# pre-calculated fade-on function. This table samples the function
# y(x) =  exp(sin((t - period / 2.) * PI / period)) - 0.36787944) * 108.
# at x={0,32,...,256}. In FadeOnFunc() we us linear interpolation to
# approximate the original function (so we do not need fp-ops).
# fade-off and breath functions are all derived from fade-on, see
# below.
# static constexpr uint8_t kFadeOnTable[] = {0,   3,   13,  33, 68,
#                                           118, 179, 232, 255}; // NOLINT
from math import exp, sin, pi as PI
from numpy import uint8, uint16 

# original fade funtion (v4)
def f8(t, period) -> uint8:
    y = (exp(sin((t-period/2.)*PI/period)) - 0.36787944)*108
    return uint8(y)

# 16 bit fade function
def f16(t, period) -> uint16:
    y = (exp(sin((t-period/2.)*PI/period)) - 0.36787944)*27756
    return uint16(y)

def approx8(t, period, lut) -> uint8:
    assert len(lut) == 9

    t = int(t)
    if t + 1 >= period: return uint8(255)
    i = ((t<<3)//period)  # -> i=0..7
    y0, y1 = lut[i], lut[i+1]
    x0 = (i*period)>>3
    dx = period >> 3
    return ((((t - x0) * (y1 - y0)) //dx) + y0)

def approx8_faster(t, period, lut) -> uint8:
    """like approx8 but using bit-shifts in final calc instead of division"""
    assert len(lut) == 9

    t = int(t)
    if t + 1 >= period: return uint8(255)
    t = ((t<<8) // period)
    i = t >> 5  # 0..7    # 32
    y0, y1 = lut[i], lut[i+1]
    x0 = i << 5   # *32
    return ((((t - x0) * (y1 - y0)) >> 5) + y0)

def approx16(t, period, lut):
    assert len(lut) == 17

    t = int(t)
    if t + 1 >= period: return uint16(-1)
    i = (t << 4) // period  # 0..15
    y0, y1 = lut[i], lut[i+1]
    x0 = (i*period)>>4
    dx = period >> 4
    return ((((t - x0) * (y1 - y0)) // dx) + y0)

def approx16_faster(t, period, lut) -> uint16:
    """like approx8 but using bit-shifts in final calc instead of division"""
    assert len(lut) == 17

    t = int(t)
    if t + 1 >= period: return uint16(-1)
    t = ((t<<8) // period)
    i = t >> 4  # 0..15
    y0, y1 = lut[i], lut[i+1]
    x0 = i << 4
    return ((((t - x0) * (y1 - y0)) >> 4) + y0)

