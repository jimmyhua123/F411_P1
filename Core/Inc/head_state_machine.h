#ifndef __HEAD_STATE_MACHINE_H__
#define __HEAD_STATE_MACHINE_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
  HEAD_CENTER = 0,
  HEAD_LEFT,
  HEAD_RIGHT,
  HEAD_DOWN,
  HEAD_UP
} head_state_t;

typedef struct
{
  float enter_left_deg;
  float exit_left_deg;
  float enter_right_deg;
  float exit_right_deg;
  float enter_down_deg;
  float exit_down_deg;
  float enter_up_deg;
  float exit_up_deg;
  head_state_t state;
} head_sm_t;

void HeadSM_Init(head_sm_t *sm);
head_state_t HeadSM_Update(head_sm_t *sm, float roll_deg, float pitch_deg);
const char *HeadSM_ToString(head_state_t state);

#ifdef __cplusplus
}
#endif

#endif /* __HEAD_STATE_MACHINE_H__ */
