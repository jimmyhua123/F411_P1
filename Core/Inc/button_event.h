#ifndef __BUTTON_EVENT_H__
#define __BUTTON_EVENT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

typedef enum
{
  BTN_EVENT_NONE = 0,
  BTN_EVENT_SHORT_PRESS,
  BTN_EVENT_LONG_PRESS_1S,
  BTN_EVENT_LONG_PRESS_3S
} ButtonEvent_t;

typedef struct
{
  GPIO_TypeDef *port;
  uint16_t pin;
  GPIO_PinState active_state;
  GPIO_PinState stable_state;
  GPIO_PinState last_sample_state;
  uint32_t last_sample_change_tick;
  uint32_t press_start_tick;
} button_event_t;

void ButtonEvent_Init(button_event_t *button,
                      GPIO_TypeDef *port,
                      uint16_t pin,
                      GPIO_PinState active_state,
                      uint32_t now);
ButtonEvent_t ButtonEvent_Update(button_event_t *button, uint32_t now);
uint8_t ButtonEvent_IsPressed(const button_event_t *button);
uint32_t ButtonEvent_GetPressedDuration(const button_event_t *button, uint32_t now);
const char *ButtonEvent_ToString(ButtonEvent_t event);

#ifdef __cplusplus
}
#endif

#endif /* __BUTTON_EVENT_H__ */
