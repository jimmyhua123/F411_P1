#ifndef __ATTITUDE_FILTER_H__
#define __ATTITUDE_FILTER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct
{
  float roll_deg;
  float pitch_deg;
} attitude_angle_t;

typedef struct
{
  float alpha;
  attitude_angle_t filtered;
  uint8_t initialized;
} attitude_filter_t;

void AttitudeFilter_Init(attitude_filter_t *filter, float alpha);
void AttitudeFilter_Update(attitude_filter_t *filter,
                           const attitude_angle_t *input,
                           attitude_angle_t *output);

#ifdef __cplusplus
}
#endif

#endif /* __ATTITUDE_FILTER_H__ */
