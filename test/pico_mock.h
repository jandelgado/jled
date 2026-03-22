// Minimal Pico SDK mock for testing JLed hardware accessing functions
#ifndef TEST_PICO_MOCK_H_
#define TEST_PICO_MOCK_H_

#include <stdint.h>

void picoMockInit();
void picoMockSetClockHz(uint32_t hz);
void picoMockSetBootTimeUs(uint64_t us);

uint32_t picoMockGetGpioFunction(uint32_t gpio);
uint16_t picoMockGetPwmWrap(uint32_t slice);
uint8_t  picoMockGetPwmDivInt(uint32_t slice);
uint8_t  picoMockGetPwmDivFrac(uint32_t slice);
bool     picoMockGetPwmEnabled(uint32_t slice);
uint16_t picoMockGetPwmChanLevel(uint32_t slice, uint32_t chan);

#endif  // TEST_PICO_MOCK_H_
