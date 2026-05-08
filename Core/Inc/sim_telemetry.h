#ifndef __SIM_TELEMETRY_H__
#define __SIM_TELEMETRY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "head_state_machine.h"
#include "power_policy.h"
#include "stm32f4xx_hal.h"
#include "system_mode.h"

void SimTelemetry_PrintFormat(void);
void SimTelemetry_Print(uint32_t tick_ms,
                        SystemMode_t mode,
                        uint32_t fault_flags,
                        const PowerPolicy_t *policy,
                        float roll_deg,
                        float pitch_deg,
                        head_state_t head_state,
                        float target_left_volume,
                        float target_right_volume,
                        float smooth_left_volume,
                        float smooth_right_volume);
int32_t SimTelemetry_DegToCentiDeg(float deg);
uint32_t SimTelemetry_VolumeToPercent(float volume);

#ifdef __cplusplus
}
#endif

#endif /* __SIM_TELEMETRY_H__ */
