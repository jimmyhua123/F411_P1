#include "bsp_uart_log.h"
#include "usart.h"
#include <stdarg.h>
#include <stdio.h>

#define LOG_PRINTF_BUFFER_SIZE 192U

int LOG_Printf(const char *format, ...)
{
  int written;
  uint16_t tx_len;
  va_list args;
  static char log_buffer[LOG_PRINTF_BUFFER_SIZE];

  va_start(args, format);
  written = vsnprintf(log_buffer, sizeof(log_buffer), format, args);
  va_end(args);

  if (written <= 0)
  {
    return written;
  }

  if ((uint32_t)written >= sizeof(log_buffer))
  {
    tx_len = (uint16_t)(sizeof(log_buffer) - 1U);
  }
  else
  {
    tx_len = (uint16_t)written;
  }

  (void)HAL_UART_Transmit(&huart2, (uint8_t *)log_buffer, tx_len, 50);

  return written;
}

int __io_putchar(int ch)
{
  uint8_t data = (uint8_t)ch;
  (void)HAL_UART_Transmit(&huart2, &data, 1, 10);
  return ch;
}
