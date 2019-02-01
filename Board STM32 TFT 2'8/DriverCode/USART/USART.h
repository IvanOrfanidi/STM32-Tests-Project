
#ifndef __USART_H
#define __USART_H

#include "includes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdint.h>

///// ----- USART ---- /////

#define RX_BUFFER_SIZE1 128
#define TX_BUFFER_SIZE1 128

#define USART_RXD GPIO_Pin_9
#define USART_TXD GPIO_Pin_10

#define USART1_NVIC_PRIORITY 0x02
#define USART1_NVIC_GROUP NVIC_PriorityGroup_2

#define USART1_Tx_DMA_Channel DMA1_Channel4
#define USART1_Rx_DMA_Channel DMA1_Channel5
#define USART1_DR_Base 0x40013804

void InitUSART1(uint32_t baud_rate);
void USART1_Send_STR(unsigned char* pData_Usart);
void DMA_USART_Configuration(void);

#endif
