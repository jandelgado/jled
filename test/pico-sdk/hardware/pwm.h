// Minimal Pico SDK PWM mock for testing JLed
#ifndef PICO_SDK_HARDWARE_PWM_H_
#define PICO_SDK_HARDWARE_PWM_H_

#include <stdint.h>

typedef unsigned int uint;
typedef uint32_t io_rw_32;

#define PWM_CHAN_A 0u
#define PWM_CHAN_B 1u

// from hardware/regs/pwm.h (RP2040/RP2350, verified against the vendored
// pico-sdk): PWM_CH0_CSR_A_INV / PWM_CH0_CSR_B_INV fields. "CH0" in the macro
// name is the register template's name, not slice-specific; it applies
// uniformly to every slice's csr register.
#define PWM_CH0_CSR_A_INV_BITS 0x00000004u
#define PWM_CH0_CSR_A_INV_LSB 2u
#define PWM_CH0_CSR_B_INV_BITS 0x00000008u
#define PWM_CH0_CSR_B_INV_LSB 3u

// from pico/types.h
#define bool_to_bit(x) ((uint) !!(x))

typedef enum { GPIO_FUNC_PWM = 4 } gpio_function_t;

// from hardware/structs/pwm.h: one memory-mapped CSR register per slice. The
// mock only models the csr field, since that's all SetLowActive() touches.
typedef struct {
    io_rw_32 csr;
} pwm_slice_hw_t;

typedef struct {
    pwm_slice_hw_t slice[8];
} pwm_hw_t;

extern pwm_hw_t pwm_hw_mock;
#define pwm_hw (&pwm_hw_mock)

uint pwm_gpio_to_slice_num(uint gpio);
uint pwm_gpio_to_channel(uint gpio);

void gpio_set_function(uint gpio, gpio_function_t fn);
void pwm_set_wrap(uint slice_num, uint16_t wrap);
void pwm_set_clkdiv_int_frac(uint slice_num, uint8_t integer, uint8_t fract);
void pwm_set_enabled(uint slice_num, bool enabled);
void pwm_set_chan_level(uint slice_num, uint chan, uint16_t level);

// from hardware/address_mapped.h: masked register write (real signature).
void hw_write_masked(io_rw_32* addr, uint32_t values, uint32_t write_mask);

#endif  // PICO_SDK_HARDWARE_PWM_H_
