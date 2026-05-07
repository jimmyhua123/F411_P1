#include "head_state_machine.h"

void HeadSM_Init(head_sm_t *sm)
{
  if (sm == 0)
  {
    return;
  }

  sm->enter_left_deg = -25.0f;
  sm->exit_left_deg = -18.0f;
  sm->enter_right_deg = 25.0f;
  sm->exit_right_deg = 18.0f;
  sm->enter_down_deg = -35.0f;
  sm->exit_down_deg = -25.0f;
  sm->enter_up_deg = 35.0f;
  sm->exit_up_deg = 25.0f;
  sm->state = HEAD_CENTER;
}

head_state_t HeadSM_Update(head_sm_t *sm, float roll_deg, float pitch_deg)
{
  head_state_t state;

  if (sm == 0)
  {
    return HEAD_CENTER;
  }

  state = sm->state;

  switch (state)
  {
    case HEAD_LEFT:
      if (roll_deg > sm->exit_left_deg)
      {
        state = HEAD_CENTER;
      }
      break;

    case HEAD_RIGHT:
      if (roll_deg < sm->exit_right_deg)
      {
        state = HEAD_CENTER;
      }
      break;

    case HEAD_DOWN:
      if (pitch_deg > sm->exit_down_deg)
      {
        state = HEAD_CENTER;
      }
      break;

    case HEAD_UP:
      if (pitch_deg < sm->exit_up_deg)
      {
        state = HEAD_CENTER;
      }
      break;

    case HEAD_CENTER:
    default:
      if (pitch_deg < sm->enter_down_deg)
      {
        state = HEAD_DOWN;
      }
      else if (pitch_deg > sm->enter_up_deg)
      {
        state = HEAD_UP;
      }
      else if (roll_deg < sm->enter_left_deg)
      {
        state = HEAD_LEFT;
      }
      else if (roll_deg > sm->enter_right_deg)
      {
        state = HEAD_RIGHT;
      }
      break;
  }

  sm->state = state;
  return state;
}

const char *HeadSM_ToString(head_state_t state)
{
  switch (state)
  {
    case HEAD_LEFT:
      return "LEFT";

    case HEAD_RIGHT:
      return "RIGHT";

    case HEAD_DOWN:
      return "DOWN";

    case HEAD_UP:
      return "UP";

    case HEAD_CENTER:
    default:
      return "CENTER";
  }
}
