#include "audio_power.h"
#include "main.h"

#define AUDIO_POWER_ON_LEVEL GPIO_PIN_RESET
#define AUDIO_POWER_OFF_LEVEL GPIO_PIN_SET

static uint8_t audio_power_on = 0U;

void AudioPower_Init(void)
{
  AudioPower_Set(1U);
}

void AudioPower_Set(uint8_t on)
{
  GPIO_PinState level;

  if (on != 0U)
  {
    level = AUDIO_POWER_ON_LEVEL;
    audio_power_on = 1U;
  }
  else
  {
    level = AUDIO_POWER_OFF_LEVEL;
    audio_power_on = 0U;
  }

  HAL_GPIO_WritePin(AUDIO_PWR_EN_GPIO_Port, AUDIO_PWR_EN_Pin, level);
}

uint8_t AudioPower_IsOn(void)
{
  return audio_power_on;
}
