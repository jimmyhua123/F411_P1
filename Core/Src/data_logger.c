#include "data_logger.h"
#include "audio_power.h"
#include "bsp_uart_log.h"

#ifndef DATA_LOGGER_USE_FATFS
#define DATA_LOGGER_USE_FATFS 1U
#endif

#if DATA_LOGGER_USE_FATFS != 0U
#include "fatfs.h"
#endif
#include <stdio.h>
#include <string.h>

#define DATA_LOGGER_PATH_SIZE 16U
#define DATA_LOGGER_MAX_FILE_INDEX 999U
#define DATA_LOGGER_ACTIVE_PERIOD_MS 200U
#define DATA_LOGGER_MUTED_PERIOD_MS 500U

#if DATA_LOGGER_USE_FATFS != 0U
static FIL logger_file;
static char logger_path[DATA_LOGGER_PATH_SIZE];
#endif
static uint8_t logger_ready = 0U;
static uint8_t logger_disabled = 0U;
static uint8_t logger_error = 0U;
static uint8_t logger_header_written = 0U;
static uint32_t logger_sequence = 0U;
static uint32_t logger_last_sample_tick = 0U;
static uint32_t logger_last_fault_flags = 0U;
static DataLogger_Snapshot_t logger_last_snapshot;

static DataLogger_Status_t DataLogger_WriteLine(const char *line);
static uint32_t DataLogger_NextSequence(void);
#if DATA_LOGGER_USE_FATFS != 0U
static FRESULT DataLogger_OpenNextFile(void);
static void DataLogger_WriteHeaderIfNeeded(void);
#endif
static void DataLogger_FormatSample(const DataLogger_Snapshot_t *snapshot,
                                    const char *mode_text,
                                    char *line,
                                    uint32_t line_size);
static int32_t DataLogger_RoundFloatToX10(float value);
static int32_t DataLogger_RoundFloatToX100(float value);
static void DataLogger_FormatSignedFixed(char *buffer,
                                         uint32_t buffer_size,
                                         int32_t value,
                                         uint32_t scale,
                                         uint8_t decimals);
static const char *DataLogger_HeadToString(head_state_t state);
static const char *DataLogger_RailToString(uint8_t audio_rail_on);
#if DATA_LOGGER_USE_FATFS != 0U
static void DataLogger_MarkError(FRESULT result);
#else
static void DataLogger_MarkError(void);
#endif

DataLogger_Status_t DataLogger_Init(void)
{
#if DATA_LOGGER_USE_FATFS != 0U
  FRESULT result;
#endif

  logger_ready = 0U;
  logger_disabled = 0U;
  logger_error = 0U;
  logger_header_written = 0U;
  logger_sequence = 0U;
  logger_last_sample_tick = 0U;
  logger_last_fault_flags = 0U;
  memset(&logger_last_snapshot, 0, sizeof(logger_last_snapshot));

#if DATA_LOGGER_USE_FATFS == 0U
  logger_disabled = 1U;
  LOG_Printf("[SD] init SKIP FatFS not enabled\r\n");
  return DATA_LOGGER_DISABLED;
#else
  result = f_mount(&USERFatFS, USERPath, 1);
  if (result != FR_OK)
  {
    DataLogger_MarkError(result);
    LOG_Printf("[SD] init FAIL mount=%u\r\n", (unsigned int)result);
    return DATA_LOGGER_ERROR;
  }

  result = DataLogger_OpenNextFile();
  if (result != FR_OK)
  {
    DataLogger_MarkError(result);
    LOG_Printf("[SD] init FAIL open=%u\r\n", (unsigned int)result);
    return DATA_LOGGER_ERROR;
  }

  DataLogger_WriteHeaderIfNeeded();
  if (logger_error == 0U)
  {
    logger_ready = 1U;
    LOG_Printf("[SD] init OK file=%s\r\n", logger_path);
    return DATA_LOGGER_OK;
  }

  return DATA_LOGGER_ERROR;
#endif
}

void DataLogger_Process(const DataLogger_Snapshot_t *snapshot)
{
  uint32_t period_ms;
  char line[160];

  if ((snapshot == 0) || (logger_ready == 0U))
  {
    return;
  }

  logger_last_snapshot = *snapshot;
  if ((snapshot->fault_flags != 0U) &&
      (snapshot->fault_flags != logger_last_fault_flags))
  {
    DataLogger_LogFault(snapshot);
    logger_last_fault_flags = snapshot->fault_flags;
  }

  if (snapshot->mode == SYS_MODE_ACTIVE)
  {
    period_ms = DATA_LOGGER_ACTIVE_PERIOD_MS;
  }
  else if (snapshot->mode == SYS_MODE_MUTED)
  {
    period_ms = DATA_LOGGER_MUTED_PERIOD_MS;
  }
  else
  {
    return;
  }

  if ((snapshot->time_ms - logger_last_sample_tick) < period_ms)
  {
    return;
  }
  logger_last_sample_tick = snapshot->time_ms;

  DataLogger_FormatSample(snapshot, SystemMode_ToString(snapshot->mode), line, sizeof(line));
  (void)DataLogger_WriteLine(line);
}

void DataLogger_LogModeEvent(const DataLogger_Snapshot_t *snapshot,
                             SystemMode_t previous_mode,
                             SystemMode_t next_mode)
{
  char line[176];
  char mode_text[48];

  if ((snapshot == 0) || (logger_ready == 0U))
  {
    return;
  }

  if ((previous_mode == SYS_MODE_LOW_POWER) || (next_mode == SYS_MODE_LOW_POWER))
  {
    (void)snprintf(mode_text,
                   sizeof(mode_text),
                   "EVENT_%s_%s",
                   (next_mode == SYS_MODE_LOW_POWER) ? "ENTER" : "EXIT",
                   SystemMode_ToString(SYS_MODE_LOW_POWER));
    DataLogger_FormatSample(snapshot, mode_text, line, sizeof(line));
    (void)DataLogger_WriteLine(line);
  }
}

void DataLogger_LogFault(const DataLogger_Snapshot_t *snapshot)
{
  char line[176];

  if ((snapshot == 0) || (logger_ready == 0U))
  {
    return;
  }

  DataLogger_FormatSample(snapshot, "EVENT_FAULT", line, sizeof(line));
  (void)DataLogger_WriteLine(line);
}

void DataLogger_LogDiagnosticSummary(uint8_t imu_ok,
                                     uint8_t i2s_left_ok,
                                     uint8_t i2s_right_ok,
                                     uint8_t ina219_ok)
{
  char line[160];

  if (logger_ready == 0U)
  {
    return;
  }

  (void)snprintf(line,
                 sizeof(line),
                 "%lu,%lu,DEMO_SUMMARY,RESULT,0.0,0.0,0.00,0.00,0.000,0.00,%s,0x%02lX,"
                 "imu=%s;i2s_left=%s;i2s_right=%s;ina219=%s\r\n",
                 (unsigned long)DataLogger_NextSequence(),
                 (unsigned long)HAL_GetTick(),
                 DataLogger_RailToString(AudioPower_IsOn()),
                 (unsigned long)logger_last_snapshot.fault_flags,
                 (imu_ok != 0U) ? "OK" : "FAIL",
                 (i2s_left_ok != 0U) ? "OK" : "FAIL",
                 (i2s_right_ok != 0U) ? "OK" : "FAIL",
                 (ina219_ok != 0U) ? "OK" : "FAIL");
  (void)DataLogger_WriteLine(line);
}

uint8_t DataLogger_IsReady(void)
{
  return logger_ready;
}

uint8_t DataLogger_HasError(void)
{
  return logger_error;
}

static DataLogger_Status_t DataLogger_WriteLine(const char *line)
{
#if DATA_LOGGER_USE_FATFS != 0U
  UINT bytes_written = 0U;
  UINT line_length;
  FRESULT result;

  if ((line == 0) || (logger_ready == 0U && logger_header_written != 0U))
  {
    return DATA_LOGGER_ERROR;
  }

  line_length = (UINT)strlen(line);
  result = f_write(&logger_file, line, line_length, &bytes_written);
  if ((result != FR_OK) || (bytes_written != line_length))
  {
    DataLogger_MarkError(result);
    return DATA_LOGGER_ERROR;
  }

  result = f_sync(&logger_file);
  if (result != FR_OK)
  {
    DataLogger_MarkError(result);
    return DATA_LOGGER_ERROR;
  }

  return DATA_LOGGER_OK;
#else
  (void)line;
  DataLogger_MarkError();
  return DATA_LOGGER_ERROR;
#endif
}

static uint32_t DataLogger_NextSequence(void)
{
  logger_sequence++;
  return logger_sequence;
}

#if DATA_LOGGER_USE_FATFS != 0U
static FRESULT DataLogger_OpenNextFile(void)
{
  FRESULT result;
  uint32_t index;

  for (index = 0U; index <= DATA_LOGGER_MAX_FILE_INDEX; index++)
  {
    (void)snprintf(logger_path,
                   sizeof(logger_path),
                   "%sLOG%03lu.CSV",
                   USERPath,
                   (unsigned long)index);
    result = f_open(&logger_file, logger_path, FA_CREATE_NEW | FA_WRITE);
    if (result == FR_OK)
    {
      logger_header_written = 0U;
      return FR_OK;
    }

    if (result != FR_EXIST)
    {
      return result;
    }
  }

  return FR_DENIED;
}

static void DataLogger_WriteHeaderIfNeeded(void)
{
  if (logger_header_written != 0U)
  {
    return;
  }

  if (DataLogger_WriteLine("seq,time_ms,mode,head,roll,pitch,lvol,rvol,bus_v,current_ma,audio_rail,fault\r\n") == DATA_LOGGER_OK)
  {
    logger_header_written = 1U;
  }
}
#endif

static void DataLogger_FormatSample(const DataLogger_Snapshot_t *snapshot,
                                    const char *mode_text,
                                    char *line,
                                    uint32_t line_size)
{
  char roll_text[16];
  char pitch_text[16];
  char left_text[16];
  char right_text[16];
  char current_text[16];

  DataLogger_FormatSignedFixed(roll_text,
                               sizeof(roll_text),
                               DataLogger_RoundFloatToX10(snapshot->roll_deg),
                               10U,
                               1U);
  DataLogger_FormatSignedFixed(pitch_text,
                               sizeof(pitch_text),
                               DataLogger_RoundFloatToX10(snapshot->pitch_deg),
                               10U,
                               1U);
  DataLogger_FormatSignedFixed(left_text,
                               sizeof(left_text),
                               DataLogger_RoundFloatToX100(snapshot->left_volume),
                               100U,
                               2U);
  DataLogger_FormatSignedFixed(right_text,
                               sizeof(right_text),
                               DataLogger_RoundFloatToX100(snapshot->right_volume),
                               100U,
                               2U);
  DataLogger_FormatSignedFixed(current_text,
                               sizeof(current_text),
                               snapshot->current_ma_x100,
                               100U,
                               2U);

  (void)snprintf(line,
                 line_size,
                 "%lu,%lu,%s,%s,%s,%s,%s,%s,%lu.%03lu,%s,%s,0x%02lX\r\n",
                 (unsigned long)DataLogger_NextSequence(),
                 (unsigned long)snapshot->time_ms,
                 mode_text,
                 DataLogger_HeadToString(snapshot->head_state),
                 roll_text,
                 pitch_text,
                 left_text,
                 right_text,
                 (unsigned long)(snapshot->bus_mv / 1000U),
                 (unsigned long)(snapshot->bus_mv % 1000U),
                 current_text,
                 DataLogger_RailToString(snapshot->audio_rail_on),
                 (unsigned long)snapshot->fault_flags);
}

static int32_t DataLogger_RoundFloatToX10(float value)
{
  if (value >= 0.0f)
  {
    return (int32_t)((value * 10.0f) + 0.5f);
  }

  return (int32_t)((value * 10.0f) - 0.5f);
}

static int32_t DataLogger_RoundFloatToX100(float value)
{
  if (value >= 0.0f)
  {
    return (int32_t)((value * 100.0f) + 0.5f);
  }

  return (int32_t)((value * 100.0f) - 0.5f);
}

static void DataLogger_FormatSignedFixed(char *buffer,
                                         uint32_t buffer_size,
                                         int32_t value,
                                         uint32_t scale,
                                         uint8_t decimals)
{
  uint32_t magnitude;
  uint32_t integer_part;
  uint32_t fractional_part;

  if ((buffer == 0) || (buffer_size == 0U) || (scale == 0U))
  {
    return;
  }

  if (value < 0L)
  {
    magnitude = (uint32_t)(0L - value);
  }
  else
  {
    magnitude = (uint32_t)value;
  }

  integer_part = magnitude / scale;
  fractional_part = magnitude % scale;

  if (decimals == 1U)
  {
    (void)snprintf(buffer,
                   buffer_size,
                   "%s%lu.%01lu",
                   (value < 0L) ? "-" : "",
                   (unsigned long)integer_part,
                   (unsigned long)fractional_part);
  }
  else
  {
    (void)snprintf(buffer,
                   buffer_size,
                   "%s%lu.%02lu",
                   (value < 0L) ? "-" : "",
                   (unsigned long)integer_part,
                   (unsigned long)fractional_part);
  }
}

static const char *DataLogger_HeadToString(head_state_t state)
{
  switch (state)
  {
    case HEAD_LEFT:
      return "LEFT";

    case HEAD_RIGHT:
      return "RIGHT";

    case HEAD_DOWN:
      return "DOWN";

    case HEAD_UP:
      return "UP";

    case HEAD_CENTER:
    default:
      return "CENTER";
  }
}

static const char *DataLogger_RailToString(uint8_t audio_rail_on)
{
  return (audio_rail_on != 0U) ? "ON" : "OFF";
}

#if DATA_LOGGER_USE_FATFS != 0U
static void DataLogger_MarkError(FRESULT result)
{
  (void)result;
  logger_ready = 0U;
  logger_error = 1U;
}
#else
static void DataLogger_MarkError(void)
{
  logger_ready = 0U;
  logger_error = 1U;
}
#endif
