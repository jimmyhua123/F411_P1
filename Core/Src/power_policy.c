#include "power_policy.h"

void PowerPolicy_Get(SystemMode_t mode, PowerPolicy_t *policy)
{
  if (policy == 0)
  {
    return;
  }

  policy->imu_period_ms = 20U;
  policy->telemetry_period_ms = 50U;
  policy->audio_enabled = 1U;
  policy->allow_sleep = 0U;

  switch (mode)
  {
    case SYS_MODE_ACTIVE:
      break;

    case SYS_MODE_MUTED:
      policy->telemetry_period_ms = 100U;
      policy->audio_enabled = 0U;
      break;

    case SYS_MODE_LOW_POWER:
      policy->imu_period_ms = 100U;
      policy->telemetry_period_ms = 500U;
      policy->audio_enabled = 0U;
      policy->allow_sleep = 1U;
      break;

    case SYS_MODE_DIAGNOSTIC:
      break;

    case SYS_MODE_FAULT:
      policy->imu_period_ms = 100U;
      policy->telemetry_period_ms = 500U;
      policy->audio_enabled = 0U;
      break;

    default:
      break;
  }
}

uint32_t PowerPolicy_PeriodToHz(uint32_t period_ms)
{
  if (period_ms == 0U)
  {
    return 0U;
  }

  return 1000U / period_ms;
}
