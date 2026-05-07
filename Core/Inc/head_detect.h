#ifndef __HEAD_DETECT_H__
#define __HEAD_DETECT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "head_state_machine.h"

typedef head_sm_t head_detector_t;

void HeadDetect_Init(head_detector_t *detector);
head_state_t HeadDetect_Update(head_detector_t *detector, float roll_deg, float pitch_deg);
const char *HeadDetect_ToString(head_state_t state);

#ifdef __cplusplus
}
#endif

#endif /* __HEAD_DETECT_H__ */
