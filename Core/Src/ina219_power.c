#include "ina219_power.h"
#include "i2c.h"

#define INA219_ADDR             (0x40U << 1)
#define INA219_REG_CONFIG       0x00U
#define INA219_REG_SHUNT_V      0x01U
#define INA219_REG_BUS_V        0x02U
#define INA219_REG_POWER        0x03U
#define INA219_REG_CURRENT      0x04U
#define INA219_REG_CALIBRATION  0x05U

#define INA219_CONFIG_32V_2A    0x399FU
#define INA219_CAL_32V_2A       4096U
#define INA219_I2C_TIMEOUT_MS   100U

static HAL_StatusTypeDef INA219_ReadReg(uint8_t reg, uint16_t *value);
static HAL_StatusTypeDef INA219_WriteReg(uint8_t reg, uint16_t value);

HAL_StatusTypeDef INA219_Init(void)
{
  HAL_StatusTypeDef status;
  uint16_t config = 0U;

  status = INA219_WriteReg(INA219_REG_CONFIG, INA219_CONFIG_32V_2A);
  if (status != HAL_OK)
  {
    return status;
  }

  status = INA219_WriteReg(INA219_REG_CALIBRATION, INA219_CAL_32V_2A);
  if (status != HAL_OK)
  {
    return status;
  }

  status = INA219_ReadReg(INA219_REG_CONFIG, &config);
  if (status != HAL_OK)
  {
    return status;
  }

  return (config == INA219_CONFIG_32V_2A) ? HAL_OK : HAL_ERROR;
}

HAL_StatusTypeDef INA219_ReadSample(ina219_sample_t *sample)
{
  HAL_StatusTypeDef status;
  uint16_t raw_bus = 0U;
  uint16_t raw_shunt = 0U;
  uint16_t raw_current = 0U;
  uint16_t raw_power = 0U;
  int16_t signed_shunt;
  int16_t signed_current;

  if (sample == 0)
  {
    return HAL_ERROR;
  }

  status = INA219_WriteReg(INA219_REG_CALIBRATION, INA219_CAL_32V_2A);
  if (status != HAL_OK)
  {
    return status;
  }

  status = INA219_ReadReg(INA219_REG_BUS_V, &raw_bus);
  if (status != HAL_OK)
  {
    return status;
  }

  status = INA219_ReadReg(INA219_REG_SHUNT_V, &raw_shunt);
  if (status != HAL_OK)
  {
    return status;
  }

  status = INA219_ReadReg(INA219_REG_CURRENT, &raw_current);
  if (status != HAL_OK)
  {
    return status;
  }

  status = INA219_ReadReg(INA219_REG_POWER, &raw_power);
  if (status != HAL_OK)
  {
    return status;
  }

  signed_shunt = (int16_t)raw_shunt;
  signed_current = (int16_t)raw_current;

  sample->bus_mv = (uint32_t)((raw_bus >> 3) * 4U);
  sample->shunt_uv = (int32_t)signed_shunt * 10L;
  sample->current_ma_x100 = (int32_t)signed_current * 10L;
  sample->power_mw = (uint32_t)raw_power * 2U;

  return HAL_OK;
}

static HAL_StatusTypeDef INA219_ReadReg(uint8_t reg, uint16_t *value)
{
  HAL_StatusTypeDef status;
  uint8_t data[2] = {0};

  if (value == 0)
  {
    return HAL_ERROR;
  }

  status = HAL_I2C_Mem_Read(&hi2c1,
                            INA219_ADDR,
                            reg,
                            I2C_MEMADD_SIZE_8BIT,
                            data,
                            sizeof(data),
                            INA219_I2C_TIMEOUT_MS);
  if (status != HAL_OK)
  {
    return status;
  }

  *value = ((uint16_t)data[0] << 8) | data[1];
  return HAL_OK;
}

static HAL_StatusTypeDef INA219_WriteReg(uint8_t reg, uint16_t value)
{
  uint8_t data[2];

  data[0] = (uint8_t)(value >> 8);
  data[1] = (uint8_t)(value & 0xFFU);

  return HAL_I2C_Mem_Write(&hi2c1,
                           INA219_ADDR,
                           reg,
                           I2C_MEMADD_SIZE_8BIT,
                           data,
                           sizeof(data),
                           INA219_I2C_TIMEOUT_MS);
}
