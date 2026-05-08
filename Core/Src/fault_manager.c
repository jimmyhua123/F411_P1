#include "fault_manager.h"
#include "bsp_uart_log.h"

#define FAULT_IMU_LOST_LIMIT       5U
#define FAULT_BUTTON_STUCK_MS      10000U
#define FAULT_LOG_PERIOD_MS        1000U
#define FAULT_SD_LOG_PERIOD_MS     10000U

static void FaultManager_PrintFlags(uint32_t flags);
static void FaultManager_PrintNames(uint32_t flags);

void FaultManager_Init(fault_manager_t *manager)
{
  if (manager == 0)
  {
    return;
  }

  manager->flags = FAULT_NONE;
  manager->last_reported_flags = FAULT_NONE;
  manager->last_log_tick = 0U;
  manager->consecutive_imu_failures = 0U;
}

void FaultManager_SetFlags(fault_manager_t *manager, uint32_t flags)
{
  if (manager == 0)
  {
    return;
  }

  manager->flags |= flags;
}

void FaultManager_RecordImuRead(fault_manager_t *manager, HAL_StatusTypeDef status)
{
  if (manager == 0)
  {
    return;
  }

  if (status == HAL_OK)
  {
    manager->consecutive_imu_failures = 0U;
    return;
  }

  if (manager->consecutive_imu_failures < FAULT_IMU_LOST_LIMIT)
  {
    manager->consecutive_imu_failures++;
  }

  if (manager->consecutive_imu_failures >= FAULT_IMU_LOST_LIMIT)
  {
    FaultManager_SetFlags(manager, FAULT_IMU_LOST);
  }
}

void FaultManager_RecordI2sErrors(fault_manager_t *manager,
                                  uint32_t left_error,
                                  uint32_t right_error)
{
  if (manager == 0)
  {
    return;
  }

  if (left_error != 0U)
  {
    FaultManager_SetFlags(manager, FAULT_I2S_LEFT);
  }

  if (right_error != 0U)
  {
    FaultManager_SetFlags(manager, FAULT_I2S_RIGHT);
  }
}

void FaultManager_RecordButtonHeld(fault_manager_t *manager, uint32_t held_ms)
{
  if (held_ms >= FAULT_BUTTON_STUCK_MS)
  {
    FaultManager_SetFlags(manager, FAULT_BUTTON_STUCK);
  }
}

uint32_t FaultManager_GetFlags(const fault_manager_t *manager)
{
  if (manager == 0)
  {
    return FAULT_NONE;
  }

  return manager->flags;
}

void FaultManager_Clear(fault_manager_t *manager)
{
  if (manager == 0)
  {
    return;
  }

  manager->flags = FAULT_NONE;
  manager->last_reported_flags = FAULT_NONE;
  manager->last_log_tick = 0U;
  manager->consecutive_imu_failures = 0U;
  LOG_Printf("[FAULT] cleared\r\n");
}

void FaultManager_UpdateLog(fault_manager_t *manager, uint32_t now_ms)
{
  uint32_t flags;

  if (manager == 0)
  {
    return;
  }

  flags = manager->flags;
  if (flags == FAULT_NONE)
  {
    manager->last_reported_flags = FAULT_NONE;
    return;
  }

  if ((flags == manager->last_reported_flags) &&
      ((now_ms - manager->last_log_tick) <
       (((flags & ~((uint32_t)FAULT_SD_LOG)) == 0U) ? FAULT_SD_LOG_PERIOD_MS : FAULT_LOG_PERIOD_MS)))
  {
    return;
  }

  manager->last_reported_flags = flags;
  manager->last_log_tick = now_ms;
  FaultManager_PrintFlags(flags);
}

void FaultManager_PrintClearRequest(void)
{
  LOG_Printf("[FAULT] clear request\r\n");
}

static void FaultManager_PrintFlags(uint32_t flags)
{
  LOG_Printf("[FAULT] flags=0x%08lX ", (unsigned long)flags);
  FaultManager_PrintNames(flags);
  LOG_Printf("\r\n");
}

static void FaultManager_PrintNames(uint32_t flags)
{
  uint8_t need_separator = 0U;

  if ((flags & FAULT_IMU_LOST) != 0U)
  {
    LOG_Printf("IMU_LOST");
    need_separator = 1U;
  }

  if ((flags & FAULT_I2S_LEFT) != 0U)
  {
    LOG_Printf("%sI2S_LEFT", (need_separator != 0U) ? "|" : "");
    need_separator = 1U;
  }

  if ((flags & FAULT_I2S_RIGHT) != 0U)
  {
    LOG_Printf("%sI2S_RIGHT", (need_separator != 0U) ? "|" : "");
    need_separator = 1U;
  }

  if ((flags & FAULT_AUDIO_CLIP) != 0U)
  {
    LOG_Printf("%sAUDIO_CLIP", (need_separator != 0U) ? "|" : "");
    need_separator = 1U;
  }

  if ((flags & FAULT_BUTTON_STUCK) != 0U)
  {
    LOG_Printf("%sBUTTON_STUCK", (need_separator != 0U) ? "|" : "");
    need_separator = 1U;
  }

  if ((flags & FAULT_SD_LOG) != 0U)
  {
    LOG_Printf("%sSD_LOG", (need_separator != 0U) ? "|" : "");
    need_separator = 1U;
  }

  if (need_separator == 0U)
  {
    LOG_Printf("UNKNOWN");
  }
}
