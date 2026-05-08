/* USER CODE BEGIN Header */
/**
 ******************************************************************************
  * @file    user_diskio.c
  * @brief   This file includes a diskio driver skeleton to be completed by the user.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
 /* USER CODE END Header */

#ifdef USE_OBSOLETE_USER_CODE_SECTION_0
/*
 * Warning: the user section 0 is no more in use (starting from CubeMx version 4.16.0)
 * To be suppressed in the future.
 * Kept to ensure backward compatibility with previous CubeMx versions when
 * migrating projects.
 * User code previously added there should be copied in the new user sections before
 * the section contents can be deleted.
 */
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */
#endif

/* USER CODE BEGIN DECL */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "ff_gen_drv.h"
#include "bsp_uart_log.h"
#include "main.h"
#include "spi.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define SD_BLOCK_SIZE          512U
#define SD_SPI_TIMEOUT_MS      100U
#define SD_INIT_TIMEOUT_MS     1000U
#define SD_READY_TIMEOUT_MS    500U
#define SD_CMD0_RETRY_COUNT    20U
#define SD_DEBUG_INIT          1U

#define SD_CMD0                0U
#define SD_CMD1                1U
#define SD_CMD8                8U
#define SD_CMD9                9U
#define SD_CMD12               12U
#define SD_CMD16               16U
#define SD_CMD17               17U
#define SD_CMD18               18U
#define SD_CMD24               24U
#define SD_CMD25               25U
#define SD_CMD55               55U
#define SD_CMD58               58U
#define SD_ACMD23              23U
#define SD_ACMD41              41U

#define SD_TOKEN_START_BLOCK   0xFEU
#define SD_TOKEN_MULTI_WRITE   0xFCU
#define SD_TOKEN_STOP_TRAN     0xFDU

#define SD_CARD_TYPE_SDHC      0x04U

/* Private variables ---------------------------------------------------------*/
/* Disk status */
static volatile DSTATUS Stat = STA_NOINIT;
static BYTE CardType = 0U;

static void SD_Select(void);
static void SD_Deselect(void);
static uint8_t SD_SPI_TxRx(uint8_t data);
static uint8_t SD_WaitReady(uint32_t timeout_ms);
static uint8_t SD_SendCommand(uint8_t cmd, uint32_t arg);
static uint8_t SD_SendAppCommand(uint8_t acmd, uint32_t arg);
static uint8_t SD_ReadBlock(uint8_t *buff, uint32_t size);
static uint8_t SD_WriteBlock(const uint8_t *buff, uint8_t token);
static uint8_t SD_StopMultiWrite(void);

/* USER CODE END DECL */

/* Private function prototypes -----------------------------------------------*/
DSTATUS USER_initialize (BYTE pdrv);
DSTATUS USER_status (BYTE pdrv);
DRESULT USER_read (BYTE pdrv, BYTE *buff, DWORD sector, UINT count);
#if _USE_WRITE == 1
  DRESULT USER_write (BYTE pdrv, const BYTE *buff, DWORD sector, UINT count);
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
  DRESULT USER_ioctl (BYTE pdrv, BYTE cmd, void *buff);
#endif /* _USE_IOCTL == 1 */

Diskio_drvTypeDef  USER_Driver =
{
  USER_initialize,
  USER_status,
  USER_read,
#if  _USE_WRITE
  USER_write,
#endif  /* _USE_WRITE == 1 */
#if  _USE_IOCTL == 1
  USER_ioctl,
#endif /* _USE_IOCTL == 1 */
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes a Drive
  * @param  pdrv: Physical drive number (0..)
  * @retval DSTATUS: Operation status
  */
DSTATUS USER_initialize (
	BYTE pdrv           /* Physical drive nmuber to identify the drive */
)
{
  /* USER CODE BEGIN INIT */
  uint8_t response;
  uint8_t ocr[4];
  uint8_t index;
  uint8_t retry;
  uint32_t start_tick;

  (void)pdrv;
  Stat = STA_NOINIT;
  CardType = 0U;

  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);
#if SD_DEBUG_INIT != 0U
  LOG_Printf("[SD] probe cs_high=%u idle=0x%02X\r\n",
             (unsigned int)HAL_GPIO_ReadPin(SD_CS_GPIO_Port, SD_CS_Pin),
             SD_SPI_TxRx(0xFFU));
  SD_Select();
  LOG_Printf("[SD] probe cs_low=%u idle=0x%02X\r\n",
             (unsigned int)HAL_GPIO_ReadPin(SD_CS_GPIO_Port, SD_CS_Pin),
             SD_SPI_TxRx(0xFFU));
  SD_Deselect();
#endif
  for (index = 0U; index < 20U; index++)
  {
    (void)SD_SPI_TxRx(0xFFU);
  }

  response = 0xFFU;
  for (retry = 0U; retry < SD_CMD0_RETRY_COUNT; retry++)
  {
    response = SD_SendCommand(SD_CMD0, 0U);
#if SD_DEBUG_INIT != 0U
    LOG_Printf("[SD] CMD0 try=%u r=0x%02X cs=%u\r\n",
               (unsigned int)(retry + 1U),
               response,
               (unsigned int)HAL_GPIO_ReadPin(SD_CS_GPIO_Port, SD_CS_Pin));
#endif
    if (response == 0x01U)
    {
      break;
    }
    HAL_Delay(5U);
  }
  if (response != 0x01U)
  {
    LOG_Printf("[SD] init CMD0 fail r=0x%02X tries=%u\r\n",
               response,
               (unsigned int)SD_CMD0_RETRY_COUNT);
    SD_Deselect();
    return Stat;
  }

  response = SD_SendCommand(SD_CMD8, 0x1AAU);
  LOG_Printf("[SD] init CMD8 r=0x%02X\r\n", response);
  if (response == 0x01U)
  {
    for (index = 0U; index < 4U; index++)
    {
      ocr[index] = SD_SPI_TxRx(0xFFU);
    }

    if ((ocr[2] != 0x01U) || (ocr[3] != 0xAAU))
    {
      LOG_Printf("[SD] init CMD8 echo fail %02X %02X %02X %02X\r\n",
                 ocr[0], ocr[1], ocr[2], ocr[3]);
      SD_Deselect();
      return Stat;
    }

    start_tick = HAL_GetTick();
    do
    {
      response = SD_SendAppCommand(SD_ACMD41, 0x40000000UL);
      if (response == 0x00U)
      {
        break;
      }
    } while ((HAL_GetTick() - start_tick) < SD_INIT_TIMEOUT_MS);

    if (response != 0x00U)
    {
      LOG_Printf("[SD] init ACMD41 timeout r=0x%02X\r\n", response);
      SD_Deselect();
      return Stat;
    }

    response = SD_SendCommand(SD_CMD58, 0U);
    if (response != 0x00U)
    {
      LOG_Printf("[SD] init CMD58 fail r=0x%02X\r\n", response);
      SD_Deselect();
      return Stat;
    }
    for (index = 0U; index < 4U; index++)
    {
      ocr[index] = SD_SPI_TxRx(0xFFU);
    }
    if ((ocr[0] & 0x40U) != 0U)
    {
      CardType = SD_CARD_TYPE_SDHC;
    }
  }
  else
  {
    start_tick = HAL_GetTick();
    do
    {
      response = SD_SendAppCommand(SD_ACMD41, 0U);
      if (response == 0x00U)
      {
        break;
      }
    } while ((HAL_GetTick() - start_tick) < SD_INIT_TIMEOUT_MS);

    if (response != 0x00U)
    {
      start_tick = HAL_GetTick();
      do
      {
        response = SD_SendCommand(SD_CMD1, 0U);
        if (response == 0x00U)
        {
          break;
        }
      } while ((HAL_GetTick() - start_tick) < SD_INIT_TIMEOUT_MS);
    }

    if (response != 0x00U)
    {
      LOG_Printf("[SD] init legacy timeout r=0x%02X\r\n", response);
      SD_Deselect();
      return Stat;
    }

    response = SD_SendCommand(SD_CMD16, SD_BLOCK_SIZE);
    if (response != 0x00U)
    {
      LOG_Printf("[SD] init CMD16 fail r=0x%02X\r\n", response);
      SD_Deselect();
      return Stat;
    }
  }

  SD_Deselect();
  Stat = 0U;
  LOG_Printf("[SD] disk ready type=%s\r\n",
             ((CardType & SD_CARD_TYPE_SDHC) != 0U) ? "SDHC" : "SDSC");
  return Stat;
  /* USER CODE END INIT */
}

/**
  * @brief  Gets Disk Status
  * @param  pdrv: Physical drive number (0..)
  * @retval DSTATUS: Operation status
  */
DSTATUS USER_status (
	BYTE pdrv       /* Physical drive number to identify the drive */
)
{
  /* USER CODE BEGIN STATUS */
  (void)pdrv;
  return Stat;
  /* USER CODE END STATUS */
}

/**
  * @brief  Reads Sector(s)
  * @param  pdrv: Physical drive number (0..)
  * @param  *buff: Data buffer to store read data
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to read (1..128)
  * @retval DRESULT: Operation result
  */
DRESULT USER_read (
	BYTE pdrv,      /* Physical drive nmuber to identify the drive */
	BYTE *buff,     /* Data buffer to store read data */
	DWORD sector,   /* Sector address in LBA */
	UINT count      /* Number of sectors to read */
)
{
  /* USER CODE BEGIN READ */
  DWORD read_sector;

  (void)pdrv;
  if ((buff == 0) || (count == 0U))
  {
    return RES_PARERR;
  }

  if ((Stat & STA_NOINIT) != 0U)
  {
    return RES_NOTRDY;
  }

  read_sector = sector;
  if ((CardType & SD_CARD_TYPE_SDHC) == 0U)
  {
    read_sector *= SD_BLOCK_SIZE;
  }

  if (count == 1U)
  {
    if ((SD_SendCommand(SD_CMD17, read_sector) == 0x00U) &&
        (SD_ReadBlock(buff, SD_BLOCK_SIZE) != 0U))
    {
      SD_Deselect();
      return RES_OK;
    }
  }
  else
  {
    if (SD_SendCommand(SD_CMD18, read_sector) == 0x00U)
    {
      do
      {
        if (SD_ReadBlock(buff, SD_BLOCK_SIZE) == 0U)
        {
          break;
        }
        buff += SD_BLOCK_SIZE;
      } while (--count != 0U);

      (void)SD_SendCommand(SD_CMD12, 0U);
      SD_Deselect();
      return (count == 0U) ? RES_OK : RES_ERROR;
    }
  }

  SD_Deselect();
  return RES_ERROR;
  /* USER CODE END READ */
}

/**
  * @brief  Writes Sector(s)
  * @param  pdrv: Physical drive number (0..)
  * @param  *buff: Data to be written
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to write (1..128)
  * @retval DRESULT: Operation result
  */
#if _USE_WRITE == 1
DRESULT USER_write (
	BYTE pdrv,          /* Physical drive nmuber to identify the drive */
	const BYTE *buff,   /* Data to be written */
	DWORD sector,       /* Sector address in LBA */
	UINT count          /* Number of sectors to write */
)
{
  /* USER CODE BEGIN WRITE */
  DWORD write_sector;

  (void)pdrv;
  if ((buff == 0) || (count == 0U))
  {
    return RES_PARERR;
  }

  if ((Stat & STA_NOINIT) != 0U)
  {
    return RES_NOTRDY;
  }

  write_sector = sector;
  if ((CardType & SD_CARD_TYPE_SDHC) == 0U)
  {
    write_sector *= SD_BLOCK_SIZE;
  }

  if (count == 1U)
  {
    if ((SD_SendCommand(SD_CMD24, write_sector) == 0x00U) &&
        (SD_WriteBlock(buff, SD_TOKEN_START_BLOCK) != 0U))
    {
      SD_Deselect();
      return RES_OK;
    }
  }
  else
  {
    if ((CardType & SD_CARD_TYPE_SDHC) != 0U)
    {
      (void)SD_SendAppCommand(SD_ACMD23, count);
    }

    if (SD_SendCommand(SD_CMD25, write_sector) == 0x00U)
    {
      do
      {
        if (SD_WriteBlock(buff, SD_TOKEN_MULTI_WRITE) == 0U)
        {
          break;
        }
        buff += SD_BLOCK_SIZE;
      } while (--count != 0U);

      if (SD_StopMultiWrite() == 0U)
      {
        count = 1U;
      }
      SD_Deselect();
      return (count == 0U) ? RES_OK : RES_ERROR;
    }
  }

  SD_Deselect();
  return RES_ERROR;
  /* USER CODE END WRITE */
}
#endif /* _USE_WRITE == 1 */

/**
  * @brief  I/O control operation
  * @param  pdrv: Physical drive number (0..)
  * @param  cmd: Control code
  * @param  *buff: Buffer to send/receive control data
  * @retval DRESULT: Operation result
  */
#if _USE_IOCTL == 1
DRESULT USER_ioctl (
	BYTE pdrv,      /* Physical drive nmuber (0..) */
	BYTE cmd,       /* Control code */
	void *buff      /* Buffer to send/receive control data */
)
{
  /* USER CODE BEGIN IOCTL */
  BYTE csd[16];
  DWORD csize;

  (void)pdrv;
  if ((Stat & STA_NOINIT) != 0U)
  {
    return RES_NOTRDY;
  }

  switch (cmd)
  {
    case CTRL_SYNC:
      return (SD_WaitReady(SD_READY_TIMEOUT_MS) != 0U) ? RES_OK : RES_ERROR;

    case GET_SECTOR_SIZE:
      *(WORD *)buff = SD_BLOCK_SIZE;
      return RES_OK;

    case GET_BLOCK_SIZE:
      *(DWORD *)buff = 1U;
      return RES_OK;

    case GET_SECTOR_COUNT:
      if ((buff == 0) ||
          (SD_SendCommand(SD_CMD9, 0U) != 0x00U) ||
          (SD_ReadBlock(csd, sizeof(csd)) == 0U))
      {
        SD_Deselect();
        return RES_ERROR;
      }
      SD_Deselect();

      if ((csd[0] & 0xC0U) == 0x40U)
      {
        csize = ((DWORD)(csd[7] & 0x3FU) << 16) |
                ((DWORD)csd[8] << 8) |
                (DWORD)csd[9];
        *(DWORD *)buff = (csize + 1U) * 1024U;
      }
      else
      {
        BYTE read_bl_len = csd[5] & 0x0FU;
        DWORD c_size = ((DWORD)(csd[6] & 0x03U) << 10) |
                       ((DWORD)csd[7] << 2) |
                       ((DWORD)(csd[8] & 0xC0U) >> 6);
        BYTE c_size_mult = ((csd[9] & 0x03U) << 1) |
                           ((csd[10] & 0x80U) >> 7);
        *(DWORD *)buff = (c_size + 1U) << (c_size_mult + read_bl_len - 7U);
      }
      return RES_OK;

    default:
      return RES_PARERR;
  }
  /* USER CODE END IOCTL */
}
#endif /* _USE_IOCTL == 1 */

/* USER CODE BEGIN 1 */
static void SD_Select(void)
{
  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET);
}

static void SD_Deselect(void)
{
  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);
  (void)SD_SPI_TxRx(0xFFU);
}

static uint8_t SD_SPI_TxRx(uint8_t data)
{
  uint8_t rx_data = 0xFFU;

  if (HAL_SPI_TransmitReceive(&hspi1, &data, &rx_data, 1U, SD_SPI_TIMEOUT_MS) != HAL_OK)
  {
    return 0xFFU;
  }

  return rx_data;
}

static uint8_t SD_WaitReady(uint32_t timeout_ms)
{
  uint32_t start_tick = HAL_GetTick();

  do
  {
    if (SD_SPI_TxRx(0xFFU) == 0xFFU)
    {
      return 1U;
    }
  } while ((HAL_GetTick() - start_tick) < timeout_ms);

  return 0U;
}

static uint8_t SD_SendCommand(uint8_t cmd, uint32_t arg)
{
  uint8_t response;
  uint8_t crc = 0x01U;
  uint8_t retry;
  uint8_t packet[6];

  SD_Deselect();
  SD_Select();
  if ((cmd != SD_CMD0) && (SD_WaitReady(SD_READY_TIMEOUT_MS) == 0U))
  {
    return 0xFEU;
  }

  if (cmd == SD_CMD0)
  {
    crc = 0x95U;
  }
  else if (cmd == SD_CMD8)
  {
    crc = 0x87U;
  }

  packet[0] = (uint8_t)(0x40U | cmd);
  packet[1] = (uint8_t)(arg >> 24);
  packet[2] = (uint8_t)(arg >> 16);
  packet[3] = (uint8_t)(arg >> 8);
  packet[4] = (uint8_t)arg;
  packet[5] = crc;

  if (HAL_SPI_Transmit(&hspi1, packet, sizeof(packet), SD_SPI_TIMEOUT_MS) != HAL_OK)
  {
    return 0xFFU;
  }

  retry = 10U;
  do
  {
    response = SD_SPI_TxRx(0xFFU);
  } while (((response & 0x80U) != 0U) && (--retry != 0U));

  return response;
}

static uint8_t SD_SendAppCommand(uint8_t acmd, uint32_t arg)
{
  uint8_t response;

  response = SD_SendCommand(SD_CMD55, 0U);
  if (response > 0x01U)
  {
    return response;
  }

  return SD_SendCommand(acmd, arg);
}

static uint8_t SD_ReadBlock(uint8_t *buff, uint32_t size)
{
  uint8_t token;
  uint32_t index;
  uint32_t start_tick = HAL_GetTick();

  do
  {
    token = SD_SPI_TxRx(0xFFU);
    if (token == SD_TOKEN_START_BLOCK)
    {
      for (index = 0U; index < size; index++)
      {
        buff[index] = SD_SPI_TxRx(0xFFU);
      }
      (void)SD_SPI_TxRx(0xFFU);
      (void)SD_SPI_TxRx(0xFFU);
      return 1U;
    }
  } while ((HAL_GetTick() - start_tick) < SD_READY_TIMEOUT_MS);

  return 0U;
}

static uint8_t SD_WriteBlock(const uint8_t *buff, uint8_t token)
{
  uint8_t response;
  uint8_t dummy_crc[2] = {0xFFU, 0xFFU};

  if (SD_WaitReady(SD_READY_TIMEOUT_MS) == 0U)
  {
    return 0U;
  }

  (void)SD_SPI_TxRx(token);
  if (token == SD_TOKEN_STOP_TRAN)
  {
    return 1U;
  }

  if (HAL_SPI_Transmit(&hspi1, (uint8_t *)buff, SD_BLOCK_SIZE, SD_SPI_TIMEOUT_MS) != HAL_OK)
  {
    return 0U;
  }
  if (HAL_SPI_Transmit(&hspi1, dummy_crc, sizeof(dummy_crc), SD_SPI_TIMEOUT_MS) != HAL_OK)
  {
    return 0U;
  }

  response = SD_SPI_TxRx(0xFFU);
  if ((response & 0x1FU) != 0x05U)
  {
    return 0U;
  }

  return SD_WaitReady(SD_READY_TIMEOUT_MS);
}

static uint8_t SD_StopMultiWrite(void)
{
  return SD_WriteBlock(0, SD_TOKEN_STOP_TRAN);
}
/* USER CODE END 1 */

