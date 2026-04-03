// Minimal Pico SDK PWM mock for testing JLed
#ifndef PICO_SDK_HARDWARE_PWM_H_
#define PICO_SDK_HARDWARE_PWM_H_

#include <stdint.h>

typedef unsigned int uint;

#define PWM_CHAN_A 0u
#define PWM_CHAN_B 1u

typedef enum { GPIO_FUNC_PWM = 4 } gpio_function_t;

uint pwm_gpio_to_slice_num(uint gpio);
uint pwm_gpio_to_channel(uint gpio);

void gpio_set_function(uint gpio, gpio_function_t fn);
void pwm_set_wrap(uint slice_num, uint16_t wrap);
void pwm_set_clkdiv_int_frac(uint slice_num, uint8_t integer, uint8_t fract);
void pwm_set_enabled(uint slice_num, bool enabled);
void pwm_set_chan_level(uint slice_num, uint chan, uint16_t level);

#endif  // PICO_SDK_HARDWARE_PWM_H_
