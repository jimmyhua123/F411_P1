#include "imu_mpu6500.h"
#include "i2c.h"

#define MPU6500_ADDR             (0x68 << 1)
#define MPU6500_REG_ACCEL_XOUT_H 0x3B
#define MPU6500_REG_PWR_MGMT_1   0x6B
#define MPU6500_REG_WHO_AM_I     0x75

static int16_t MPU6500_MakeInt16(uint8_t high, uint8_t low);
static HAL_StatusTypeDef MPU6500_ReadReg(uint8_t reg, uint8_t *value);
static HAL_StatusTypeDef MPU6500_ReadRegs(uint8_t reg, uint8_t *data, uint16_t len);
static HAL_StatusTypeDef MPU6500_WriteReg(uint8_t reg, uint8_t value);

HAL_StatusTypeDef MPU6500_Init(uint8_t *who_am_i)
{
  HAL_StatusTypeDef status;
  uint8_t id = 0;

  HAL_Delay(100);
  status = MPU6500_WriteReg(MPU6500_REG_PWR_MGMT_1, 0x00);
  if (status != HAL_OK)
  {
    return status;
  }

  HAL_Delay(100);
  status = MPU6500_ReadReg(MPU6500_REG_WHO_AM_I, &id);
  if (who_am_i != NULL)
  {
    *who_am_i = id;
  }

  return status;
}

HAL_StatusTypeDef MPU6500_ReadWhoAmI(uint8_t *who_am_i)
{
  if (who_am_i == NULL)
  {
    return HAL_ERROR;
  }

  return MPU6500_ReadReg(MPU6500_REG_WHO_AM_I, who_am_i);
}

HAL_StatusTypeDef MPU6500_ReadRaw(imu_raw_t *raw)
{
  uint8_t data[14] = {0};
  HAL_StatusTypeDef status;

  if (raw == NULL)
  {
    return HAL_ERROR;
  }

  status = MPU6500_ReadRegs(MPU6500_REG_ACCEL_XOUT_H, data, sizeof(data));
  if (status != HAL_OK)
  {
    return status;
  }

  raw->ax = MPU6500_MakeInt16(data[0], data[1]);
  raw->ay = MPU6500_MakeInt16(data[2], data[3]);
  raw->az = MPU6500_MakeInt16(data[4], data[5]);
  raw->gx = MPU6500_MakeInt16(data[8], data[9]);
  raw->gy = MPU6500_MakeInt16(data[10], data[11]);
  raw->gz = MPU6500_MakeInt16(data[12], data[13]);

  return HAL_OK;
}

static HAL_StatusTypeDef MPU6500_ReadReg(uint8_t reg, uint8_t *value)
{
  return HAL_I2C_Mem_Read(&hi2c1, MPU6500_ADDR, reg, I2C_MEMADD_SIZE_8BIT, value, 1, 100);
}

static HAL_StatusTypeDef MPU6500_ReadRegs(uint8_t reg, uint8_t *data, uint16_t len)
{
  return HAL_I2C_Mem_Read(&hi2c1, MPU6500_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, len, 100);
}

static HAL_StatusTypeDef MPU6500_WriteReg(uint8_t reg, uint8_t value)
{
  return HAL_I2C_Mem_Write(&hi2c1, MPU6500_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, 100);
}

static int16_t MPU6500_MakeInt16(uint8_t high, uint8_t low)
{
  return (int16_t)((uint16_t)high << 8 | low);
}
