
#ifndef __USART_H
#define __USART_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdint.h>

//// ----- USART ---- ////
#define RX_BUFFER_SIZE1 8
#define TX_BUFFER_SIZE1 8

#define RX_BUFFER_SIZE2 8
#define TX_BUFFER_SIZE2 8

#define RX_BUFFER_SIZE3 8
#define TX_BUFFER_SIZE3 8
//**********************//

// USART1 //
#define USART1_PORT GPIOA
#define USART1_TxPin GPIO_Pin_9
#define USART1_RxPin GPIO_Pin_10
#define USART1_CLK RCC_APB2Periph_USART1
#define USART1_PORT_CLK RCC_APB2Periph_GPIOA

// USART2 //
#define USART2_PORT GPIOA
#define USART2_TxPin GPIO_Pin_2
#define USART2_RxPin GPIO_Pin_3
#define USART2_CLK RCC_APB1Periph_USART2
#define USART2_PORT_CLK RCC_APB2Periph_GPIOA

// USART3 //
#define USART3_PORT GPIOB
#define USART3_TxPin GPIO_Pin_10
#define USART3_RxPin GPIO_Pin_11
#define USART3_CLK RCC_APB1Periph_USART3
#define USART3_PORT_CLK RCC_APB2Periph_GPIOB

// -=USARTS DMA CONFIG=- //
#define USART1_Tx_DMA_Channel DMA1_Channel4
#define USART1_Rx_DMA_Channel DMA1_Channel5
#define USART1_DR_Base USART1_BASE + 0x04

#define USART2_Tx_DMA_Channel DMA1_Channel7
#define USART2_Rx_DMA_Channel DMA1_Channel6
#define USART2_DR_Base USART2_BASE + 0x04

#define USART3_Tx_DMA_Channel DMA1_Channel2
#define USART3_Rx_DMA_Channel DMA1_Channel3
#define USART3_DR_Base USART3_BASE + 0x04
//********************************//

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

void DeInitDMA(uint8_t NumDMA);
_Bool GpsBufNumber(void);

#endif
