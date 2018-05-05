#ifndef _UART_H
#define _UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32l0xx_hal.h"

#define dbg_uart hlpuart1

void MX_USART_DEBUG_Init(void);
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart);
void HAL_UART_MspInit(UART_HandleTypeDef* huart);

#ifdef __cplusplus
}
#endif

#endif