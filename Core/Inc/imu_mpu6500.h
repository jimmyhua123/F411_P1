#ifndef __IMU_MPU6500_H__
#define __IMU_MPU6500_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

typedef struct
{
  int16_t ax;
  int16_t ay;
  int16_t az;
  int16_t gx;
  int16_t gy;
  int16_t gz;
} imu_raw_t;

HAL_StatusTypeDef MPU6500_Init(uint8_t *who_am_i);
HAL_StatusTypeDef MPU6500_ReadWhoAmI(uint8_t *who_am_i);
HAL_StatusTypeDef MPU6500_ReadRaw(imu_raw_t *raw);

#ifdef __cplusplus
}
#endif

#endif /* __IMU_MPU6500_H__ */
