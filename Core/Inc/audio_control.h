#ifndef __AUDIO_CONTROL_H__
#define __AUDIO_CONTROL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "head_state_machine.h"
#include "system_mode.h"
#include <stdint.h>

void AudioControl_Init(void);
void AudioControl_SetMode(SystemMode_t mode);
uint8_t AudioControl_Update(head_state_t state, float roll_deg, float pitch_deg);
float AudioControl_GetLeftVolume(void);
float AudioControl_GetRightVolume(void);

#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_CONTROL_H__ */
