// Minimal Pico SDK time mock for testing JLed
#ifndef PICO_SDK_PICO_TIME_H_
#define PICO_SDK_PICO_TIME_H_

#include <stdint.h>

typedef uint64_t absolute_time_t;

absolute_time_t get_absolute_time();
uint32_t to_ms_since_boot(absolute_time_t t);

#endif  // PICO_SDK_PICO_TIME_H_
