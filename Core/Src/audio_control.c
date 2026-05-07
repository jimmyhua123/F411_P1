#include "audio_control.h"
#include "bsp_uart_log.h"

#define PAN_MAX_ROLL_DEG    30.0f
#define PAN_MIN_SIDE_VOLUME 0.30f

static void AudioControl_ApplyPan(head_state_t state, float roll_deg);
static void AudioControl_ApplyModeOverride(void);
static float AudioControl_Clamp(float value, float min_value, float max_value);
static void AudioControl_PrintVolume(float left, float right);

static head_state_t current_head_state = HEAD_CENTER;
static SystemMode_t current_mode = SYS_MODE_ACTIVE;
static float base_left_volume = 1.0f;
static float base_right_volume = 1.0f;
static float left_volume = 1.0f;
static float right_volume = 1.0f;

void AudioControl_Init(void)
{
  current_head_state = HEAD_CENTER;
  current_mode = SYS_MODE_ACTIVE;
  AudioControl_ApplyPan(current_head_state, 0.0f);
  AudioControl_ApplyModeOverride();
}

void AudioControl_SetMode(SystemMode_t mode)
{
  current_mode = mode;
  AudioControl_ApplyModeOverride();
}

uint8_t AudioControl_Update(head_state_t state, float roll_deg, float pitch_deg)
{
  head_state_t previous_state;
  uint8_t state_changed = 0U;

  (void)pitch_deg;

  if (state != current_head_state)
  {
    previous_state = current_head_state;
    current_head_state = state;
    state_changed = 1U;

    LOG_Printf("[HEAD] %s -> %s\r\n",
               HeadSM_ToString(previous_state),
               HeadSM_ToString(current_head_state));
  }

  AudioControl_ApplyPan(current_head_state, roll_deg);
  AudioControl_ApplyModeOverride();

  if (state_changed != 0U)
  {
    if ((left_volume == 0.0f) && (right_volume == 0.0f))
    {
      LOG_Printf("[AUDIO] mute\r\n");
    }
    else
    {
      AudioControl_PrintVolume(left_volume, right_volume);
    }
  }

  return state_changed;
}

float AudioControl_GetLeftVolume(void)
{
  return left_volume;
}

float AudioControl_GetRightVolume(void)
{
  return right_volume;
}

static void AudioControl_ApplyPan(head_state_t state, float roll_deg)
{
  float pan;
  float side_reduction;

  if (state == HEAD_DOWN)
  {
    base_left_volume = 0.0f;
    base_right_volume = 0.0f;
    return;
  }

  pan = AudioControl_Clamp(roll_deg / PAN_MAX_ROLL_DEG, -1.0f, 1.0f);
  side_reduction = 1.0f - PAN_MIN_SIDE_VOLUME;

  if (pan < 0.0f)
  {
    base_left_volume = 1.0f;
    base_right_volume = 1.0f - ((0.0f - pan) * side_reduction);
  }
  else
  {
    base_left_volume = 1.0f - (pan * side_reduction);
    base_right_volume = 1.0f;
  }
}

static void AudioControl_ApplyModeOverride(void)
{
  left_volume = base_left_volume;
  right_volume = base_right_volume;

  if ((current_mode == SYS_MODE_MUTED) ||
      (current_mode == SYS_MODE_LOW_POWER) ||
      (current_mode == SYS_MODE_FAULT))
  {
    left_volume = 0.0f;
    right_volume = 0.0f;
  }
  else if (current_mode == SYS_MODE_DIAGNOSTIC)
  {
    left_volume = 1.0f;
    right_volume = 1.0f;
  }
}

static float AudioControl_Clamp(float value, float min_value, float max_value)
{
  if (value < min_value)
  {
    return min_value;
  }

  if (value > max_value)
  {
    return max_value;
  }

  return value;
}

static void AudioControl_PrintVolume(float left, float right)
{
  uint32_t left_percent = (uint32_t)((left * 100.0f) + 0.5f);
  uint32_t right_percent = (uint32_t)((right * 100.0f) + 0.5f);

  LOG_Printf("[AUDIO] L=%lu.%02lu R=%lu.%02lu\r\n",
             (unsigned long)(left_percent / 100U),
             (unsigned long)(left_percent % 100U),
             (unsigned long)(right_percent / 100U),
             (unsigned long)(right_percent % 100U));
}
