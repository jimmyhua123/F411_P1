#ifndef __INA219_POWER_H__
#define __INA219_POWER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdint.h>

typedef struct
{
  uint32_t bus_mv;
  int32_t shunt_uv;
  int32_t current_ma_x100;
  uint32_t power_mw;
} ina219_sample_t;

HAL_StatusTypeDef INA219_Init(void);
HAL_StatusTypeDef INA219_ReadSample(ina219_sample_t *sample);

#ifdef __cplusplus
}
#endif

#endif /* __INA219_POWER_H__ */
