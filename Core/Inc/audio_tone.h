#ifndef __AUDIO_TONE_H__
#define __AUDIO_TONE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

#ifdef HAL_I2S_MODULE_ENABLED
void AudioTone_Init(I2S_HandleTypeDef *left_i2s, I2S_HandleTypeDef *right_i2s);
#else
void AudioTone_Init(void *left_i2s, void *right_i2s);
#endif

HAL_StatusTypeDef AudioTone_Start(void);
HAL_StatusTypeDef AudioTone_Stop(void);
void AudioTone_SetVolume(float volume);
void AudioTone_SetStereoVolume(float left, float right);
void AudioTone_SetMute(uint8_t mute);
uint32_t AudioTone_GetError(void);
uint32_t AudioTone_GetState(void);
uint32_t AudioTone_GetLeftError(void);
uint32_t AudioTone_GetLeftState(void);
uint32_t AudioTone_GetRightError(void);
uint32_t AudioTone_GetRightState(void);

#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_TONE_H__ */
