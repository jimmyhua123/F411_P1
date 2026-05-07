#include "audio_volume_smoother.h"

#define AUDIO_VOLUME_SMOOTHER_DEFAULT_STEP 0.05f

static float AudioVolumeSmoother_Approach(float current, float target, float step);
static float AudioVolumeSmoother_Clamp(float volume);

void AudioVolumeSmoother_Init(audio_volume_smoother_t *smoother)
{
  if (smoother == 0)
  {
    return;
  }

  smoother->target_left = 0.0f;
  smoother->target_right = 0.0f;
  smoother->smooth_left = 0.0f;
  smoother->smooth_right = 0.0f;
  smoother->step = AUDIO_VOLUME_SMOOTHER_DEFAULT_STEP;
}

void AudioVolumeSmoother_SetStep(audio_volume_smoother_t *smoother, float step)
{
  if (smoother == 0)
  {
    return;
  }

  if (step < 0.0f)
  {
    step = 0.0f;
  }

  if (step > 1.0f)
  {
    step = 1.0f;
  }

  smoother->step = step;
}

void AudioVolumeSmoother_SetTarget(audio_volume_smoother_t *smoother, float left, float right)
{
  if (smoother == 0)
  {
    return;
  }

  smoother->target_left = AudioVolumeSmoother_Clamp(left);
  smoother->target_right = AudioVolumeSmoother_Clamp(right);
}

uint8_t AudioVolumeSmoother_Update(audio_volume_smoother_t *smoother)
{
  float next_left;
  float next_right;

  if (smoother == 0)
  {
    return 0U;
  }

  next_left = AudioVolumeSmoother_Approach(smoother->smooth_left,
                                           smoother->target_left,
                                           smoother->step);
  next_right = AudioVolumeSmoother_Approach(smoother->smooth_right,
                                            smoother->target_right,
                                            smoother->step);

  if ((next_left == smoother->smooth_left) && (next_right == smoother->smooth_right))
  {
    return 0U;
  }

  smoother->smooth_left = next_left;
  smoother->smooth_right = next_right;
  return 1U;
}

float AudioVolumeSmoother_GetLeft(const audio_volume_smoother_t *smoother)
{
  if (smoother == 0)
  {
    return 0.0f;
  }

  return smoother->smooth_left;
}

float AudioVolumeSmoother_GetRight(const audio_volume_smoother_t *smoother)
{
  if (smoother == 0)
  {
    return 0.0f;
  }

  return smoother->smooth_right;
}

static float AudioVolumeSmoother_Approach(float current, float target, float step)
{
  if (current < target)
  {
    current += step;
    if (current > target)
    {
      current = target;
    }
  }
  else if (current > target)
  {
    current -= step;
    if (current < target)
    {
      current = target;
    }
  }

  return AudioVolumeSmoother_Clamp(current);
}

static float AudioVolumeSmoother_Clamp(float volume)
{
  if (volume < 0.0f)
  {
    return 0.0f;
  }

  if (volume > 1.0f)
  {
    return 1.0f;
  }

  return volume;
}
