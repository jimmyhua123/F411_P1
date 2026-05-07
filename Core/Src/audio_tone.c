#include "audio_tone.h"
#include <math.h>
#include <stdint.h>

#define AUDIO_TONE_SAMPLE_RATE_HZ 16000.0f
#define AUDIO_TONE_FREQ_HZ        440.0f
#define AUDIO_TONE_FRAMES         800U
#define AUDIO_TONE_CHANNELS       2U
#define AUDIO_TONE_BUFFER_SAMPLES (AUDIO_TONE_FRAMES * AUDIO_TONE_CHANNELS)
#define AUDIO_TONE_AMPLITUDE      12000.0f
#define AUDIO_TONE_PI             3.14159265f

static void AudioTone_FillBuffer(void);
static float AudioTone_ClampVolume(float volume);

static int16_t left_audio_buffer[AUDIO_TONE_BUFFER_SAMPLES];
static int16_t right_audio_buffer[AUDIO_TONE_BUFFER_SAMPLES];
static float left_audio_volume = 0.1f;
static float right_audio_volume = 0.1f;
static uint8_t audio_muted = 0U;

#ifdef HAL_I2S_MODULE_ENABLED
static I2S_HandleTypeDef *left_audio_i2s = 0;
static I2S_HandleTypeDef *right_audio_i2s = 0;

void AudioTone_Init(I2S_HandleTypeDef *left_i2s, I2S_HandleTypeDef *right_i2s)
{
  left_audio_i2s = left_i2s;
  right_audio_i2s = right_i2s;
  AudioTone_FillBuffer();
}

HAL_StatusTypeDef AudioTone_Start(void)
{
  HAL_StatusTypeDef status;

  if ((left_audio_i2s == 0) || (right_audio_i2s == 0))
  {
    return HAL_ERROR;
  }

  status = HAL_I2S_Transmit_DMA(left_audio_i2s,
                                (uint16_t *)left_audio_buffer,
                                (uint16_t)AUDIO_TONE_BUFFER_SAMPLES);
  if (status != HAL_OK)
  {
    return status;
  }

  status = HAL_I2S_Transmit_DMA(right_audio_i2s,
                                (uint16_t *)right_audio_buffer,
                                (uint16_t)AUDIO_TONE_BUFFER_SAMPLES);
  if (status != HAL_OK)
  {
    (void)HAL_I2S_DMAStop(left_audio_i2s);
  }

  return status;
}

HAL_StatusTypeDef AudioTone_Stop(void)
{
  HAL_StatusTypeDef left_status;
  HAL_StatusTypeDef right_status;

  if ((left_audio_i2s == 0) || (right_audio_i2s == 0))
  {
    return HAL_ERROR;
  }

  left_status = HAL_I2S_DMAStop(left_audio_i2s);
  right_status = HAL_I2S_DMAStop(right_audio_i2s);

  if (left_status != HAL_OK)
  {
    return left_status;
  }

  return right_status;
}

uint32_t AudioTone_GetError(void)
{
  return AudioTone_GetLeftError() | AudioTone_GetRightError();
}

uint32_t AudioTone_GetState(void)
{
  return AudioTone_GetLeftState();
}

uint32_t AudioTone_GetLeftError(void)
{
  if (left_audio_i2s == 0)
  {
    return 0xFFFFFFFFU;
  }

  return HAL_I2S_GetError(left_audio_i2s);
}

uint32_t AudioTone_GetLeftState(void)
{
  if (left_audio_i2s == 0)
  {
    return 0xFFFFFFFFU;
  }

  return (uint32_t)HAL_I2S_GetState(left_audio_i2s);
}

uint32_t AudioTone_GetRightError(void)
{
  if (right_audio_i2s == 0)
  {
    return 0xFFFFFFFFU;
  }

  return HAL_I2S_GetError(right_audio_i2s);
}

uint32_t AudioTone_GetRightState(void)
{
  if (right_audio_i2s == 0)
  {
    return 0xFFFFFFFFU;
  }

  return (uint32_t)HAL_I2S_GetState(right_audio_i2s);
}
#else
void AudioTone_Init(void *left_i2s, void *right_i2s)
{
  (void)left_i2s;
  (void)right_i2s;
  AudioTone_FillBuffer();
}

HAL_StatusTypeDef AudioTone_Start(void)
{
  return HAL_ERROR;
}

HAL_StatusTypeDef AudioTone_Stop(void)
{
  return HAL_ERROR;
}

uint32_t AudioTone_GetError(void)
{
  return 0xFFFFFFFFU;
}

uint32_t AudioTone_GetState(void)
{
  return 0xFFFFFFFFU;
}

uint32_t AudioTone_GetLeftError(void)
{
  return 0xFFFFFFFFU;
}

uint32_t AudioTone_GetLeftState(void)
{
  return 0xFFFFFFFFU;
}

uint32_t AudioTone_GetRightError(void)
{
  return 0xFFFFFFFFU;
}

uint32_t AudioTone_GetRightState(void)
{
  return 0xFFFFFFFFU;
}
#endif

void AudioTone_SetVolume(float volume)
{
  AudioTone_SetStereoVolume(volume, volume);
}

void AudioTone_SetStereoVolume(float left, float right)
{
  left_audio_volume = AudioTone_ClampVolume(left);
  right_audio_volume = AudioTone_ClampVolume(right);
  AudioTone_FillBuffer();
}

void AudioTone_SetMute(uint8_t mute)
{
  audio_muted = mute != 0U ? 1U : 0U;
  AudioTone_FillBuffer();
}

static void AudioTone_FillBuffer(void)
{
  uint32_t frame;
  uint32_t index;
  float phase;
  int16_t sample;
  int16_t left_sample;
  int16_t right_sample;

  for (frame = 0U; frame < AUDIO_TONE_FRAMES; frame++)
  {
    phase = (2.0f * AUDIO_TONE_PI * AUDIO_TONE_FREQ_HZ * (float)frame) / AUDIO_TONE_SAMPLE_RATE_HZ;
    sample = (int16_t)(sinf(phase) * AUDIO_TONE_AMPLITUDE);

    if (audio_muted != 0U)
    {
      left_sample = 0;
      right_sample = 0;
    }
    else
    {
      left_sample = (int16_t)((float)sample * left_audio_volume);
      right_sample = (int16_t)((float)sample * right_audio_volume);
    }

    index = frame * AUDIO_TONE_CHANNELS;
    left_audio_buffer[index] = left_sample;
    left_audio_buffer[index + 1U] = left_sample;
    right_audio_buffer[index] = right_sample;
    right_audio_buffer[index + 1U] = right_sample;
  }
}

static float AudioTone_ClampVolume(float volume)
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
