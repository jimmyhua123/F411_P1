#include "status_led.h"
#include "main.h"

#define STATUS_LED_ON           GPIO_PIN_SET
#define STATUS_LED_OFF          GPIO_PIN_RESET
#define LED_SELF_TEST_ON_MS     300U
#define LED_SELF_TEST_OFF_MS    150U
#define LOW_POWER_LED_BLINK_MS  500U

static void StatusLed_Set(GPIO_PinState red_state,
                          GPIO_PinState green_state,
                          GPIO_PinState yellow_state);

static uint32_t last_blink_tick = 0U;
static uint8_t low_power_blink_phase = 0U;
static SystemMode_t previous_mode = SYS_MODE_ACTIVE;

void StatusLed_Init(void)
{
  StatusLed_Set(STATUS_LED_ON, STATUS_LED_ON, STATUS_LED_ON);
  HAL_Delay(LED_SELF_TEST_ON_MS);
  StatusLed_Set(STATUS_LED_OFF, STATUS_LED_OFF, STATUS_LED_OFF);
  HAL_Delay(LED_SELF_TEST_OFF_MS);

  last_blink_tick = HAL_GetTick();
  low_power_blink_phase = 0U;
  previous_mode = SYS_MODE_ACTIVE;
}

void StatusLed_Update(SystemMode_t mode, uint32_t fault_flags, uint32_t now_ms)
{
  GPIO_PinState red_state = STATUS_LED_OFF;
  GPIO_PinState green_state = STATUS_LED_OFF;
  GPIO_PinState yellow_state = STATUS_LED_OFF;

  if (mode != previous_mode)
  {
    previous_mode = mode;
    last_blink_tick = now_ms;
    low_power_blink_phase = 0U;
  }

  if (fault_flags != 0U)
  {
    StatusLed_Set(STATUS_LED_ON, STATUS_LED_OFF, STATUS_LED_OFF);
    return;
  }

  switch (mode)
  {
    case SYS_MODE_DIAGNOSTIC:
      red_state = STATUS_LED_ON;
      green_state = STATUS_LED_ON;
      yellow_state = STATUS_LED_ON;
      break;

    case SYS_MODE_LOW_POWER:
      if ((now_ms - last_blink_tick) >= LOW_POWER_LED_BLINK_MS)
      {
        last_blink_tick = now_ms;
        low_power_blink_phase = (low_power_blink_phase == 0U) ? 1U : 0U;
      }
      yellow_state = (low_power_blink_phase != 0U) ? STATUS_LED_ON : STATUS_LED_OFF;
      break;

    case SYS_MODE_MUTED:
      yellow_state = STATUS_LED_ON;
      break;

    case SYS_MODE_ACTIVE:
      green_state = STATUS_LED_ON;
      break;

    case SYS_MODE_FAULT:
    default:
      break;
  }

  StatusLed_Set(red_state, green_state, yellow_state);
}

static void StatusLed_Set(GPIO_PinState red_state,
                          GPIO_PinState green_state,
                          GPIO_PinState yellow_state)
{
  HAL_GPIO_WritePin(STATUS_LED_RED_GPIO_Port, STATUS_LED_RED_Pin, red_state);
  HAL_GPIO_WritePin(STATUS_LED_GREEN_GPIO_Port, STATUS_LED_GREEN_Pin, green_state);
  HAL_GPIO_WritePin(STATUS_LED_YELLOW_GPIO_Port, STATUS_LED_YELLOW_Pin, yellow_state);
}
