#ifndef __AUDIO_VOLUME_SMOOTHER_H__
#define __AUDIO_VOLUME_SMOOTHER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct
{
  float target_left;
  float target_right;
  float smooth_left;
  float smooth_right;
  float step;
} audio_volume_smoother_t;

void AudioVolumeSmoother_Init(audio_volume_smoother_t *smoother);
void AudioVolumeSmoother_SetStep(audio_volume_smoother_t *smoother, float step);
void AudioVolumeSmoother_SetTarget(audio_volume_smoother_t *smoother, float left, float right);
uint8_t AudioVolumeSmoother_Update(audio_volume_smoother_t *smoother);
float AudioVolumeSmoother_GetLeft(const audio_volume_smoother_t *smoother);
float AudioVolumeSmoother_GetRight(const audio_volume_smoother_t *smoother);

#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_VOLUME_SMOOTHER_H__ */
