#ifndef __ATTITUDE_H__
#define __ATTITUDE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "imu_mpu6500.h"

typedef struct
{
  float roll_deg;
  float pitch_deg;
} attitude_t;

void Attitude_UpdateAccelOnly(const imu_raw_t *raw, attitude_t *attitude);
int32_t Attitude_ToCentidegrees(float angle_deg);

#ifdef __cplusplus
}
#endif

#endif /* __ATTITUDE_H__ */
