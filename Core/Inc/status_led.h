#ifndef __STATUS_LED_H__
#define __STATUS_LED_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include "system_mode.h"
#include <stdint.h>

void StatusLed_Init(void);
void StatusLed_Update(SystemMode_t mode, uint32_t fault_flags, uint32_t now_ms);

#ifdef __cplusplus
}
#endif

#endif /* __STATUS_LED_H__ */
