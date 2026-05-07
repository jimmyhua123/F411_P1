#ifndef __DATA_LOGGER_H__
#define __DATA_LOGGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "head_state_machine.h"
#include "system_mode.h"
#include <stdint.h>

typedef struct
{
  uint32_t time_ms;
  SystemMode_t mode;
  head_state_t head_state;
  float roll_deg;
  float pitch_deg;
  float left_volume;
  float right_volume;
  uint32_t bus_mv;
  int32_t current_ma_x100;
  uint8_t audio_rail_on;
  uint32_t fault_flags;
} DataLogger_Snapshot_t;

typedef enum
{
  DATA_LOGGER_OK = 0,
  DATA_LOGGER_ERROR
} DataLogger_Status_t;

DataLogger_Status_t DataLogger_Init(void);
void DataLogger_Process(const DataLogger_Snapshot_t *snapshot);
void DataLogger_LogModeEvent(const DataLogger_Snapshot_t *snapshot,
                             SystemMode_t previous_mode,
                             SystemMode_t next_mode);
void DataLogger_LogFault(const DataLogger_Snapshot_t *snapshot);
void DataLogger_LogDiagnosticSummary(uint8_t imu_ok,
                                     uint8_t i2s_left_ok,
                                     uint8_t i2s_right_ok,
                                     uint8_t ina219_ok);
uint8_t DataLogger_IsReady(void);
uint8_t DataLogger_HasError(void);

#ifdef __cplusplus
}
#endif

#endif /* __DATA_LOGGER_H__ */
