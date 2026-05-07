#ifndef __POWER_POLICY_H__
#define __POWER_POLICY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "system_mode.h"
#include <stdint.h>

typedef struct
{
  uint32_t imu_period_ms;
  uint32_t telemetry_period_ms;
  uint8_t audio_enabled;
  uint8_t allow_sleep;
} PowerPolicy_t;

void PowerPolicy_Get(SystemMode_t mode, PowerPolicy_t *policy);
uint32_t PowerPolicy_PeriodToHz(uint32_t period_ms);

#ifdef __cplusplus
}
#endif

#endif /* __POWER_POLICY_H__ */
