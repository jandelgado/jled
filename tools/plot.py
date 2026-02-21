import matplotlib.pyplot as plt
import numpy as np
from functools import partial
from fade_funcs import f8, f16,approx8, approx8_faster,approx16,approx16_faster

lut16 = [0,198,807,1874,3474,5714,8719,12625,17545,23524,30485,38166,46081,53536,59707,63801,65535]
lut8 = [0,3,13,33,68,118,179,232,255]

period = 1000

# plot all functions
fig, (ax1, ax2) = plt.subplots(1,2,layout="constrained")

for ax in (ax1, ax2):
    ax.set_xlabel("time")
    ax.set_ylabel("brightness")

x = np.linspace(0, period, period)

# original function f (8 Bit version)
f_vectorized = np.vectorize(partial(f8, period=period))
ax1.plot(x, f_vectorized(x) , label="original")

g = partial(approx8_faster, period=period, lut=lut8)
g_vectorized = np.vectorize(g)
ax1.plot(x, g_vectorized(x) , label="approx8_faster")

# discrete versions
g = partial(approx8, period=period, lut=lut8)
g_vectorized = np.vectorize(g)
ax1.plot(x, g_vectorized(x) , label="approx8")

ax1.legend()

# ------------------------

f_vectorized = np.vectorize(partial(f16, period=period))
ax2.plot(x, f_vectorized(x) , label="original")

g = partial(approx16_faster, period=period, lut=lut16)
g_vectorized = np.vectorize(g)
ax2.plot(x, g_vectorized(x) , label="approx16_faster")

# discrete versions
#g = lambda x: smooth_dither(partial(approx8, period=period)(x))
g = partial(approx16, period=period, lut=lut16)
g_vectorized = np.vectorize(g)
ax2.plot(x, g_vectorized(x) , label="approx16")

ax2.legend()

plt.show()
