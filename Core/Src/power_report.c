#include "power_report.h"
#include "bsp_uart_log.h"

#define POWER_REPORT_CUTOFF_PASS_MA_X100 100L
#define POWER_REPORT_LOW_POWER_MIN_SAMPLES 8U
#define POWER_REPORT_SAMPLE_PERIOD_MS 100U

typedef enum
{
  POWER_REPORT_ACTIVE = 0,
  POWER_REPORT_HEAD_DOWN,
  POWER_REPORT_MUTED,
  POWER_REPORT_LOW_POWER,
  POWER_REPORT_DIAGNOSTIC,
  POWER_REPORT_FAULT,
  POWER_REPORT_COUNT
} PowerReportCategory_t;

typedef struct
{
  int32_t current_min_ma_x100;
  int32_t current_max_ma_x100;
  int64_t current_sum_ma_x100;
  uint32_t sample_count;
  uint32_t first_tick_ms;
  uint32_t last_tick_ms;
} PowerReportStats_t;

static PowerReportStats_t report_stats[POWER_REPORT_COUNT];
static uint8_t low_power_cutoff_reported = 0U;

static PowerReportCategory_t PowerReport_GetCategory(SystemMode_t mode, head_state_t head_state);
static const char *PowerReport_CategoryName(PowerReportCategory_t category);
static int32_t PowerReport_GetAverage(const PowerReportStats_t *stats);
static uint32_t PowerReport_GetDuration(const PowerReportStats_t *stats);
static uint8_t PowerReport_IsCutoffPass(void);
static uint8_t PowerReport_IsHeadDownMutePass(void);
static void PowerReport_PrintStats(PowerReportCategory_t category);
static void PowerReport_PrintCurrentValue(PowerReportCategory_t category);
static void PowerReport_PrintFixedSignedX100(int32_t value_x100);

void PowerReport_Init(void)
{
  uint32_t index;

  for (index = 0U; index < (uint32_t)POWER_REPORT_COUNT; index++)
  {
    report_stats[index].current_min_ma_x100 = 0L;
    report_stats[index].current_max_ma_x100 = 0L;
    report_stats[index].current_sum_ma_x100 = 0LL;
    report_stats[index].sample_count = 0U;
    report_stats[index].first_tick_ms = 0U;
    report_stats[index].last_tick_ms = 0U;
  }

  low_power_cutoff_reported = 0U;
}

void PowerReport_Record(SystemMode_t mode,
                        head_state_t head_state,
                        int32_t current_ma_x100,
                        uint32_t now_ms)
{
  PowerReportCategory_t category;
  PowerReportStats_t *stats;

  category = PowerReport_GetCategory(mode, head_state);
  stats = &report_stats[(uint32_t)category];

  if (stats->sample_count == 0U)
  {
    stats->current_min_ma_x100 = current_ma_x100;
    stats->current_max_ma_x100 = current_ma_x100;
    stats->first_tick_ms = now_ms;
  }
  else
  {
    if (current_ma_x100 < stats->current_min_ma_x100)
    {
      stats->current_min_ma_x100 = current_ma_x100;
    }
    if (current_ma_x100 > stats->current_max_ma_x100)
    {
      stats->current_max_ma_x100 = current_ma_x100;
    }
  }

  stats->current_sum_ma_x100 += current_ma_x100;
  stats->sample_count++;
  stats->last_tick_ms = now_ms;

  if ((category == POWER_REPORT_LOW_POWER) &&
      (low_power_cutoff_reported == 0U) &&
      (stats->sample_count >= POWER_REPORT_LOW_POWER_MIN_SAMPLES))
  {
    PowerReport_PrintPowerSummary();
    low_power_cutoff_reported = 1U;
  }
}

void PowerReport_PrintPowerSummary(void)
{
  PowerReport_PrintStats(POWER_REPORT_ACTIVE);
  PowerReport_PrintStats(POWER_REPORT_HEAD_DOWN);
  PowerReport_PrintStats(POWER_REPORT_MUTED);
  PowerReport_PrintStats(POWER_REPORT_LOW_POWER);
  PowerReport_PrintStats(POWER_REPORT_DIAGNOSTIC);
  PowerReport_PrintStats(POWER_REPORT_FAULT);

  if (report_stats[(uint32_t)POWER_REPORT_LOW_POWER].sample_count == 0U)
  {
    LOG_Printf("[PWR] AUDIO_RAIL_CUTOFF=UNKNOWN\r\n");
  }
  else
  {
    LOG_Printf("[PWR] AUDIO_RAIL_CUTOFF=%s\r\n",
               (PowerReport_IsCutoffPass() != 0U) ? "PASS" : "FAIL");
  }
}

void PowerReport_PrintDemoSummary(uint8_t imu_ok,
                                  uint8_t i2s_left_ok,
                                  uint8_t i2s_right_ok,
                                  uint8_t ina219_ok)
{
  uint8_t cutoff_pass;
  uint8_t head_down_mute_pass;
  uint8_t result_pass;

  cutoff_pass = PowerReport_IsCutoffPass();
  head_down_mute_pass = PowerReport_IsHeadDownMutePass();
  result_pass = ((imu_ok != 0U) &&
                 (i2s_left_ok != 0U) &&
                 (i2s_right_ok != 0U) &&
                 (ina219_ok != 0U) &&
                 (cutoff_pass != 0U)) ? 1U : 0U;

  PowerReport_PrintPowerSummary();
  LOG_Printf("[DEMO_SUMMARY]\r\n");
  LOG_Printf("HEAD_TRACKING=%s\r\n", (imu_ok != 0U) ? "PASS" : "FAIL");
  if ((report_stats[(uint32_t)POWER_REPORT_ACTIVE].sample_count == 0U) ||
      (report_stats[(uint32_t)POWER_REPORT_HEAD_DOWN].sample_count == 0U))
  {
    LOG_Printf("HEAD_DOWN_MUTE=UNKNOWN\r\n");
  }
  else
  {
    LOG_Printf("HEAD_DOWN_MUTE=%s\r\n", (head_down_mute_pass != 0U) ? "PASS" : "FAIL");
  }
  if (report_stats[(uint32_t)POWER_REPORT_LOW_POWER].sample_count == 0U)
  {
    LOG_Printf("LOW_POWER_AUDIO_RAIL_CUTOFF=UNKNOWN\r\n");
  }
  else
  {
    LOG_Printf("LOW_POWER_AUDIO_RAIL_CUTOFF=%s\r\n", (cutoff_pass != 0U) ? "PASS" : "FAIL");
  }
  LOG_Printf("ACTIVE_20_CURRENT=");
  PowerReport_PrintCurrentValue(POWER_REPORT_ACTIVE);
  LOG_Printf("\r\n");
  LOG_Printf("HEAD_DOWN_CURRENT=");
  PowerReport_PrintCurrentValue(POWER_REPORT_HEAD_DOWN);
  LOG_Printf("\r\n");
  LOG_Printf("LOW_POWER_CURRENT=");
  PowerReport_PrintCurrentValue(POWER_REPORT_LOW_POWER);
  LOG_Printf("\r\n");
  LOG_Printf("RESULT=%s\r\n", (result_pass != 0U) ? "PASS" : "FAIL");
  LOG_Printf("IMU=%s\r\n", (imu_ok != 0U) ? "OK" : "FAIL");
  LOG_Printf("I2S_LEFT=%s\r\n", (i2s_left_ok != 0U) ? "OK" : "FAIL");
  LOG_Printf("I2S_RIGHT=%s\r\n", (i2s_right_ok != 0U) ? "OK" : "FAIL");
  LOG_Printf("INA219=%s\r\n", (ina219_ok != 0U) ? "OK" : "FAIL");
}

static PowerReportCategory_t PowerReport_GetCategory(SystemMode_t mode, head_state_t head_state)
{
  switch (mode)
  {
    case SYS_MODE_MUTED:
      return POWER_REPORT_MUTED;

    case SYS_MODE_LOW_POWER:
      return POWER_REPORT_LOW_POWER;

    case SYS_MODE_DIAGNOSTIC:
      return POWER_REPORT_DIAGNOSTIC;

    case SYS_MODE_FAULT:
      return POWER_REPORT_FAULT;

    case SYS_MODE_ACTIVE:
    default:
      if (head_state == HEAD_DOWN)
      {
        return POWER_REPORT_HEAD_DOWN;
      }
      return POWER_REPORT_ACTIVE;
  }
}

static const char *PowerReport_CategoryName(PowerReportCategory_t category)
{
  switch (category)
  {
    case POWER_REPORT_HEAD_DOWN:
      return "HEAD_DOWN";

    case POWER_REPORT_MUTED:
      return "MUTED";

    case POWER_REPORT_LOW_POWER:
      return "LOW_POWER";

    case POWER_REPORT_DIAGNOSTIC:
      return "DIAGNOSTIC";

    case POWER_REPORT_FAULT:
      return "FAULT";

    case POWER_REPORT_ACTIVE:
    default:
      return "ACTIVE";
  }
}

static int32_t PowerReport_GetAverage(const PowerReportStats_t *stats)
{
  if ((stats == 0) || (stats->sample_count == 0U))
  {
    return 0L;
  }

  return (int32_t)(stats->current_sum_ma_x100 / (int64_t)stats->sample_count);
}

static uint32_t PowerReport_GetDuration(const PowerReportStats_t *stats)
{
  if ((stats == 0) || (stats->sample_count == 0U))
  {
    return 0U;
  }

  return stats->sample_count * POWER_REPORT_SAMPLE_PERIOD_MS;
}

static uint8_t PowerReport_IsCutoffPass(void)
{
  int32_t low_power_avg;

  if (report_stats[(uint32_t)POWER_REPORT_LOW_POWER].sample_count == 0U)
  {
    return 0U;
  }

  low_power_avg = PowerReport_GetAverage(&report_stats[(uint32_t)POWER_REPORT_LOW_POWER]);
  return (low_power_avg < POWER_REPORT_CUTOFF_PASS_MA_X100) ? 1U : 0U;
}

static uint8_t PowerReport_IsHeadDownMutePass(void)
{
  int32_t active_avg;
  int32_t head_down_avg;

  if ((report_stats[(uint32_t)POWER_REPORT_ACTIVE].sample_count == 0U) ||
      (report_stats[(uint32_t)POWER_REPORT_HEAD_DOWN].sample_count == 0U))
  {
    return 0U;
  }

  active_avg = PowerReport_GetAverage(&report_stats[(uint32_t)POWER_REPORT_ACTIVE]);
  head_down_avg = PowerReport_GetAverage(&report_stats[(uint32_t)POWER_REPORT_HEAD_DOWN]);
  return (head_down_avg < active_avg) ? 1U : 0U;
}

static void PowerReport_PrintStats(PowerReportCategory_t category)
{
  PowerReportStats_t *stats;

  stats = &report_stats[(uint32_t)category];
  if (stats->sample_count == 0U)
  {
    return;
  }

  LOG_Printf("[PWR] %s avg=", PowerReport_CategoryName(category));
  PowerReport_PrintFixedSignedX100(PowerReport_GetAverage(stats));
  LOG_Printf("mA min=");
  PowerReport_PrintFixedSignedX100(stats->current_min_ma_x100);
  LOG_Printf("mA max=");
  PowerReport_PrintFixedSignedX100(stats->current_max_ma_x100);
  LOG_Printf("mA n=%lu duration=%lums\r\n",
             (unsigned long)stats->sample_count,
             (unsigned long)PowerReport_GetDuration(stats));
}

static void PowerReport_PrintCurrentValue(PowerReportCategory_t category)
{
  PowerReportStats_t *stats;

  stats = &report_stats[(uint32_t)category];
  if (stats->sample_count == 0U)
  {
    LOG_Printf("NA");
    return;
  }

  PowerReport_PrintFixedSignedX100(PowerReport_GetAverage(stats));
  LOG_Printf("mA");
}

static void PowerReport_PrintFixedSignedX100(int32_t value_x100)
{
  uint32_t magnitude_x100;
  uint32_t rounded_x10;

  if (value_x100 < 0L)
  {
    LOG_Printf("-");
    magnitude_x100 = (uint32_t)(0L - value_x100);
  }
  else
  {
    magnitude_x100 = (uint32_t)value_x100;
  }

  rounded_x10 = (magnitude_x100 + 5U) / 10U;
  LOG_Printf("%lu.%lu",
             (unsigned long)(rounded_x10 / 10U),
             (unsigned long)(rounded_x10 % 10U));
}
