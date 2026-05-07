#ifndef __FAULT_MANAGER_H__
#define __FAULT_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdint.h>

typedef enum
{
  FAULT_NONE = 0,
  FAULT_IMU_LOST = 1U << 0,
  FAULT_I2S_LEFT = 1U << 1,
  FAULT_I2S_RIGHT = 1U << 2,
  FAULT_AUDIO_CLIP = 1U << 3,
  FAULT_BUTTON_STUCK = 1U << 4,
  FAULT_SD_LOG = 1U << 5
} FaultFlags_t;

typedef struct
{
  volatile uint32_t flags;
  uint32_t last_reported_flags;
  uint32_t last_log_tick;
  uint8_t consecutive_imu_failures;
} fault_manager_t;

void FaultManager_Init(fault_manager_t *manager);
void FaultManager_SetFlags(fault_manager_t *manager, uint32_t flags);
void FaultManager_RecordImuRead(fault_manager_t *manager, HAL_StatusTypeDef status);
void FaultManager_RecordI2sErrors(fault_manager_t *manager,
                                  uint32_t left_error,
                                  uint32_t right_error);
void FaultManager_RecordButtonHeld(fault_manager_t *manager, uint32_t held_ms);
uint32_t FaultManager_GetFlags(const fault_manager_t *manager);
void FaultManager_Clear(fault_manager_t *manager);
void FaultManager_UpdateLog(fault_manager_t *manager, uint32_t now_ms);
void FaultManager_PrintClearRequest(void);

#ifdef __cplusplus
}
#endif

#endif /* __FAULT_MANAGER_H__ */
