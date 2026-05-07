#include "attitude_filter.h"

void AttitudeFilter_Init(attitude_filter_t *filter, float alpha)
{
  if (filter == 0)
  {
    return;
  }

  filter->alpha = alpha;
  filter->filtered.roll_deg = 0.0f;
  filter->filtered.pitch_deg = 0.0f;
  filter->initialized = 0U;
}

void AttitudeFilter_Update(attitude_filter_t *filter,
                           const attitude_angle_t *input,
                           attitude_angle_t *output)
{
  float alpha;

  if ((filter == 0) || (input == 0) || (output == 0))
  {
    return;
  }

  if (filter->initialized == 0U)
  {
    filter->filtered = *input;
    filter->initialized = 1U;
  }
  else
  {
    alpha = filter->alpha;
    filter->filtered.roll_deg = (alpha * input->roll_deg) + ((1.0f - alpha) * filter->filtered.roll_deg);
    filter->filtered.pitch_deg = (alpha * input->pitch_deg) + ((1.0f - alpha) * filter->filtered.pitch_deg);
  }

  *output = filter->filtered;
}
