#ifndef _UART_H
#define _UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32l0xx_hal.h"

void MX_USART1_UART_Init(void);
void MX_LPUSART_DEBUG_Init(void);
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart);
void HAL_UART_MspInit(UART_HandleTypeDef* huart);
void sendRequest(uint8_t* pData, uint16_t Size);
uint16_t lenCommRxBuf(void);
int acceptAnswer(uint8_t* pData, uint16_t Size);

#ifdef __cplusplus
}
#endif

#endif