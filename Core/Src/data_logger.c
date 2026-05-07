#include "data_logger.h"
#include "audio_power.h"
#include "bsp_uart_log.h"
#include "ff.h"
#include <stdio.h>
#include <string.h>

#define DATA_LOGGER_FILE_NAME "log.csv"
#define DATA_LOGGER_ACTIVE_PERIOD_MS 200U
#define DATA_LOGGER_MUTED_PERIOD_MS 500U

static FATFS logger_fs;
static FIL logger_file;
static uint8_t logger_ready = 0U;
static uint8_t logger_error = 0U;
static uint8_t logger_header_written = 0U;
static uint32_t logger_last_sample_tick = 0U;
static uint32_t logger_last_fault_flags = 0U;
static DataLogger_Snapshot_t logger_last_snapshot;

static DataLogger_Status_t DataLogger_WriteLine(const char *line);
static void DataLogger_WriteHeaderIfNeeded(void);
static void DataLogger_FormatSample(const DataLogger_Snapshot_t *snapshot,
                                    const char *mode_text,
                                    char *line,
                                    uint32_t line_size);
static const char *DataLogger_HeadToString(head_state_t state);
static const char *DataLogger_RailToString(uint8_t audio_rail_on);
static void DataLogger_MarkError(FRESULT result);

DataLogger_Status_t DataLogger_Init(void)
{
  FRESULT result;

  logger_ready = 0U;
  logger_error = 0U;
  logger_header_written = 0U;
  logger_last_sample_tick = 0U;
  logger_last_fault_flags = 0U;
  memset(&logger_last_snapshot, 0, sizeof(logger_last_snapshot));

  result = f_mount(&logger_fs, "", 1);
  if (result != FR_OK)
  {
    DataLogger_MarkError(result);
    LOG_Printf("[SD] init FAIL mount=%u\r\n", (unsigned int)result);
    return DATA_LOGGER_ERROR;
  }

  result = f_open(&logger_file, DATA_LOGGER_FILE_NAME, FA_OPEN_ALWAYS | FA_WRITE);
  if (result != FR_OK)
  {
    DataLogger_MarkError(result);
    LOG_Printf("[SD] init FAIL open=%u\r\n", (unsigned int)result);
    return DATA_LOGGER_ERROR;
  }

  if (f_size(&logger_file) == 0U)
  {
    logger_header_written = 0U;
  }
  else
  {
    logger_header_written = 1U;
    (void)f_lseek(&logger_file, f_size(&logger_file));
  }

  DataLogger_WriteHeaderIfNeeded();
  if (logger_error == 0U)
  {
    logger_ready = 1U;
    LOG_Printf("[SD] init OK file=%s\r\n", DATA_LOGGER_FILE_NAME);
    return DATA_LOGGER_OK;
  }

  return DATA_LOGGER_ERROR;
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
                 "%lu,DEMO_SUMMARY,RESULT,0.0,0.0,0.00,0.00,0.000,0.00,%s,0x%02lX,"
                 "imu=%s;i2s_left=%s;i2s_right=%s;ina219=%s\r\n",
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
}

static void DataLogger_WriteHeaderIfNeeded(void)
{
  if (logger_header_written != 0U)
  {
    return;
  }

  if (DataLogger_WriteLine("time_ms,mode,head,roll,pitch,lvol,rvol,bus_v,current_ma,audio_rail,fault\r\n") == DATA_LOGGER_OK)
  {
    logger_header_written = 1U;
  }
}

static void DataLogger_FormatSample(const DataLogger_Snapshot_t *snapshot,
                                    const char *mode_text,
                                    char *line,
                                    uint32_t line_size)
{
  (void)snprintf(line,
                 line_size,
                 "%lu,%s,%s,%.1f,%.1f,%.2f,%.2f,%lu.%03lu,%.2f,%s,0x%02lX\r\n",
                 (unsigned long)snapshot->time_ms,
                 mode_text,
                 DataLogger_HeadToString(snapshot->head_state),
                 (double)snapshot->roll_deg,
                 (double)snapshot->pitch_deg,
                 (double)snapshot->left_volume,
                 (double)snapshot->right_volume,
                 (unsigned long)(snapshot->bus_mv / 1000U),
                 (unsigned long)(snapshot->bus_mv % 1000U),
                 (double)snapshot->current_ma_x100 / 100.0,
                 DataLogger_RailToString(snapshot->audio_rail_on),
                 (unsigned long)snapshot->fault_flags);
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

static void DataLogger_MarkError(FRESULT result)
{
  (void)result;
  logger_ready = 0U;
  logger_error = 1U;
}
