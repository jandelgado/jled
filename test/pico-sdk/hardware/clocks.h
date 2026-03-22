// Minimal Pico SDK clocks mock for testing JLed
#ifndef PICO_SDK_HARDWARE_CLOCKS_H_
#define PICO_SDK_HARDWARE_CLOCKS_H_

#include <stdint.h>

typedef enum { clk_sys = 0 } clock_index_t;

uint32_t clock_get_hz(clock_index_t clk);

#endif  // PICO_SDK_HARDWARE_CLOCKS_H_
