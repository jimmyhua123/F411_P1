#ifndef __POWER_REPORT_H__
#define __POWER_REPORT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "head_state_machine.h"
#include "system_mode.h"
#include <stdint.h>

void PowerReport_Init(void);
void PowerReport_Record(SystemMode_t mode,
                        head_state_t head_state,
                        int32_t current_ma_x100,
                        uint32_t now_ms);
void PowerReport_PrintPowerSummary(void);
void PowerReport_PrintDemoSummary(uint8_t imu_ok,
                                  uint8_t i2s_left_ok,
                                  uint8_t i2s_right_ok,
                                  uint8_t ina219_ok);

#ifdef __cplusplus
}
#endif

#endif /* __POWER_REPORT_H__ */
