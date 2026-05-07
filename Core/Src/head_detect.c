#include "head_detect.h"

void HeadDetect_Init(head_detector_t *detector)
{
  HeadSM_Init(detector);
}

head_state_t HeadDetect_Update(head_detector_t *detector, float roll_deg, float pitch_deg)
{
  return HeadSM_Update(detector, roll_deg, pitch_deg);
}

const char *HeadDetect_ToString(head_state_t state)
{
  return HeadSM_ToString(state);
}
