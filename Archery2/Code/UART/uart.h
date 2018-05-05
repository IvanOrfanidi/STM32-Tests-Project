
#ifndef __USART_H
#define __USART_H

// USART1 //
#define USART1_PORT GPIOA
#define USART1_TX_PIN GPIO_Pin_9
#define USART1_RX_PIN GPIO_Pin_10
#define USART1_CLK RCC_APB2Periph_USART1
#define USART1_PORT_CLK RCC_APB2Periph_GPIOA

// USART2 //
#define USART2_PORT GPIOA
#define USART2_TX_PIN GPIO_Pin_2
#define USART2_RX_PIN GPIO_Pin_3
#define USART2_CLK RCC_APB1Periph_USART2
#define USART2_PORT_CLK RCC_APB2Periph_GPIOA

// USART3 //
#define USART3_PORT GPIOB
#define USART3_TX_PIN GPIO_Pin_10
#define USART3_RX_PIN GPIO_Pin_11
#define USART3_CLK RCC_APB1Periph_USART3
#define USART3_PORT_CLK RCC_APB2Periph_GPIOB

#define RUN_TRANSMIT_UART_DBG USART_ITConfig(UART_DBG_INTRERFACE, USART_IT_TXE, ENABLE)
#define STOP_TRANSMIT_UART_DBG USART_ITConfig(UART_DBG_INTRERFACE, USART_IT_TXE, DISABLE)

// -=USARTS DMA CONFIG=- //
#define USART1_Tx_DMA_Channel DMA1_Channel4
#define USART1_Rx_DMA_Channel DMA1_Channel5
#define USART1_DR_Base (USART1_BASE + 0x04)

#define USART2_Tx_DMA_Channel DMA1_Channel7
#define USART2_Rx_DMA_Channel DMA1_Channel6
#define USART2_DR_Base (USART2_BASE + 0x04)

//********************************//

void initDebugUart(void);
void initGpsUart(void);
#endif
