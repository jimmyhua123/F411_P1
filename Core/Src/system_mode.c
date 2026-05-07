#include "system_mode.h"

void SystemMode_Init(system_mode_t *system_mode)
{
  if (system_mode == 0)
  {
    return;
  }

  system_mode->mode = SYS_MODE_ACTIVE;
}

SystemMode_t SystemMode_Get(const system_mode_t *system_mode)
{
  if (system_mode == 0)
  {
    return SYS_MODE_ACTIVE;
  }

  return system_mode->mode;
}

uint8_t SystemMode_Set(system_mode_t *system_mode, SystemMode_t mode)
{
  if (system_mode == 0)
  {
    return 0U;
  }

  if (system_mode->mode == mode)
  {
    return 0U;
  }

  system_mode->mode = mode;
  return 1U;
}

uint8_t SystemMode_HandleButtonEvent(system_mode_t *system_mode, ButtonEvent_t event)
{
  SystemMode_t mode;

  if (system_mode == 0)
  {
    return 0U;
  }

  mode = system_mode->mode;

  switch (event)
  {
    case BTN_EVENT_LONG_PRESS_1S:
      if (mode == SYS_MODE_FAULT)
      {
        break;
      }

      if (mode == SYS_MODE_ACTIVE)
      {
        return SystemMode_Set(system_mode, SYS_MODE_MUTED);
      }

      if ((mode == SYS_MODE_MUTED) || (mode == SYS_MODE_LOW_POWER))
      {
        return SystemMode_Set(system_mode, SYS_MODE_ACTIVE);
      }
      break;

    case BTN_EVENT_LONG_PRESS_3S:
      if (mode == SYS_MODE_FAULT)
      {
        break;
      }
      return SystemMode_Set(system_mode, SYS_MODE_DIAGNOSTIC);

    case BTN_EVENT_SHORT_PRESS:
    case BTN_EVENT_NONE:
    default:
      break;
  }

  return 0U;
}

const char *SystemMode_ToString(SystemMode_t mode)
{
  switch (mode)
  {
    case SYS_MODE_ACTIVE:
      return "ACTIVE";
    case SYS_MODE_MUTED:
      return "MUTED";
    case SYS_MODE_LOW_POWER:
      return "LOW_POWER";
    case SYS_MODE_DIAGNOSTIC:
      return "DIAGNOSTIC";
    case SYS_MODE_FAULT:
      return "FAULT";
    default:
      return "UNKNOWN";
  }
}
