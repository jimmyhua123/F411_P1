#ifndef __SYSTEM_MODE_H__
#define __SYSTEM_MODE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "button_event.h"
#include <stdint.h>

typedef enum
{
  SYS_MODE_ACTIVE = 0,
  SYS_MODE_MUTED,
  SYS_MODE_LOW_POWER,
  SYS_MODE_DIAGNOSTIC,
  SYS_MODE_FAULT
} SystemMode_t;

typedef struct
{
  SystemMode_t mode;
} system_mode_t;

void SystemMode_Init(system_mode_t *system_mode);
SystemMode_t SystemMode_Get(const system_mode_t *system_mode);
uint8_t SystemMode_Set(system_mode_t *system_mode, SystemMode_t mode);
uint8_t SystemMode_HandleButtonEvent(system_mode_t *system_mode, ButtonEvent_t event);
const char *SystemMode_ToString(SystemMode_t mode);

#ifdef __cplusplus
}
#endif

#endif /* __SYSTEM_MODE_H__ */
