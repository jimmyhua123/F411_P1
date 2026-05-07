#include "attitude.h"
#include <math.h>

#define RAD_TO_DEG 57.29578f

void Attitude_UpdateAccelOnly(const imu_raw_t *raw, attitude_t *attitude)
{
  if ((raw == NULL) || (attitude == NULL))
  {
    return;
  }

  attitude->roll_deg = atan2f((float)raw->ay, (float)raw->az) * RAD_TO_DEG;
  attitude->pitch_deg = atan2f((float)raw->ax,
                               sqrtf((float)raw->ay * raw->ay + (float)raw->az * raw->az)) * RAD_TO_DEG;
}

int32_t Attitude_ToCentidegrees(float angle_deg)
{
  if (angle_deg >= 0.0f)
  {
    return (int32_t)(angle_deg * 100.0f + 0.5f);
  }

  return (int32_t)(angle_deg * 100.0f - 0.5f);
}
