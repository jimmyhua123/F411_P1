#include "button_event.h"

#define BUTTON_EVENT_DEBOUNCE_MS 50U
#define BUTTON_EVENT_LONG_1S_MS  1000U
#define BUTTON_EVENT_LONG_3S_MS  3000U

static uint8_t ButtonEvent_StateIsPressed(const button_event_t *button, GPIO_PinState state);

void ButtonEvent_Init(button_event_t *button,
                      GPIO_TypeDef *port,
                      uint16_t pin,
                      GPIO_PinState active_state,
                      uint32_t now)
{
  GPIO_PinState state;

  if (button == 0)
  {
    return;
  }

  state = HAL_GPIO_ReadPin(port, pin);
  button->port = port;
  button->pin = pin;
  button->active_state = active_state;
  button->stable_state = state;
  button->last_sample_state = state;
  button->last_sample_change_tick = now;
  button->press_start_tick = ButtonEvent_StateIsPressed(button, state) ? now : 0U;
}

ButtonEvent_t ButtonEvent_Update(button_event_t *button, uint32_t now)
{
  GPIO_PinState sample_state;
  GPIO_PinState previous_stable_state;
  uint32_t press_duration_ms;

  if ((button == 0) || (button->port == 0))
  {
    return BTN_EVENT_NONE;
  }

  sample_state = HAL_GPIO_ReadPin(button->port, button->pin);
  if (sample_state != button->last_sample_state)
  {
    button->last_sample_state = sample_state;
    button->last_sample_change_tick = now;
    return BTN_EVENT_NONE;
  }

  if ((now - button->last_sample_change_tick) < BUTTON_EVENT_DEBOUNCE_MS)
  {
    return BTN_EVENT_NONE;
  }

  if (sample_state == button->stable_state)
  {
    return BTN_EVENT_NONE;
  }

  previous_stable_state = button->stable_state;
  button->stable_state = sample_state;

  if (!ButtonEvent_StateIsPressed(button, previous_stable_state) &&
      ButtonEvent_StateIsPressed(button, button->stable_state))
  {
    button->press_start_tick = now;
    return BTN_EVENT_NONE;
  }

  if (ButtonEvent_StateIsPressed(button, previous_stable_state) &&
      !ButtonEvent_StateIsPressed(button, button->stable_state))
  {
    press_duration_ms = now - button->press_start_tick;
    button->press_start_tick = 0U;

    if (press_duration_ms >= BUTTON_EVENT_LONG_3S_MS)
    {
      return BTN_EVENT_LONG_PRESS_3S;
    }

    if (press_duration_ms >= BUTTON_EVENT_LONG_1S_MS)
    {
      return BTN_EVENT_LONG_PRESS_1S;
    }

    return BTN_EVENT_SHORT_PRESS;
  }

  return BTN_EVENT_NONE;
}

uint8_t ButtonEvent_IsPressed(const button_event_t *button)
{
  if (button == 0)
  {
    return 0U;
  }

  return ButtonEvent_StateIsPressed(button, button->stable_state);
}

uint32_t ButtonEvent_GetPressedDuration(const button_event_t *button, uint32_t now)
{
  if ((button == 0) || (ButtonEvent_IsPressed(button) == 0U) || (button->press_start_tick == 0U))
  {
    return 0U;
  }

  return now - button->press_start_tick;
}

const char *ButtonEvent_ToString(ButtonEvent_t event)
{
  switch (event)
  {
    case BTN_EVENT_SHORT_PRESS:
      return "SHORT";
    case BTN_EVENT_LONG_PRESS_1S:
      return "LONG_1S";
    case BTN_EVENT_LONG_PRESS_3S:
      return "LONG_3S";
    case BTN_EVENT_NONE:
    default:
      return "NONE";
  }
}

static uint8_t ButtonEvent_StateIsPressed(const button_event_t *button, GPIO_PinState state)
{
  return (state == button->active_state) ? 1U : 0U;
}
