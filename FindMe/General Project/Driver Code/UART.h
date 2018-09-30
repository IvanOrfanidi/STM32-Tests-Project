
#ifndef __USART_H
#define __USART_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdint.h>

void DMA_USART1_Configuration(void);
void DMA_USART2_Configuration(void);
void DMA_USART3_Configuration(void);

void InitUSART1(uint32_t baud_rate);
void InitDMA(uint8_t NumUSART);

void USART1_Write(const char* pData_Usart, uint16_t Len);
void USART1_Read(char* pData_Usart, uint16_t Len);
uint16_t USART1_Rx_Len(void);

void InitUSART2(uint32_t baud_rate);
void USART2_Write(const char* pData_Usart, uint16_t Len);
void USART2_Read(char* pData_Usart, uint16_t Len);
uint16_t USART2_Rx_Len(void);

void InitUSART3(uint32_t baud_rate);
void USART3_Write(const char* pData_Usart, uint16_t Len);
void USART3_Read(char* pData_Usart, uint16_t Len);
uint16_t USART3_Rx_Len(void);

void InitUSART(uint8_t NumUSART, uint32_t baud_rate);
void USART_Write(uint8_t NumUSART, const char* pData_Usart, uint16_t Len);
void USART_Read(uint8_t NumUSART, char* pData_Usart, uint16_t Len);
uint16_t USART_Rx_Len(uint8_t NumUSART);

void DeInitUSART(uint8_t NumUSART);

void DeInitUSART1(void);
void DeInitUSART2(void);
void DeInitUSART3(void);

void DeInitDMA1(void);
void DeInitDMA2(void);
void DeInitDMA3(void);

void ReStartDmaUsart2(void);
void ReStartDmaUsart1(void);
void ReStartDmaUsart3(void);

void DeInitDMA(uint8_t NumDMA);

_Bool GpsBufNumber(void);
_Bool CanBufNumber(void);
#endif
