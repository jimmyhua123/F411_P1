#ifndef __AUDIO_POWER_H__
#define __AUDIO_POWER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void AudioPower_Init(void);
void AudioPower_Set(uint8_t on);
uint8_t AudioPower_IsOn(void);

#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_POWER_H__ */
