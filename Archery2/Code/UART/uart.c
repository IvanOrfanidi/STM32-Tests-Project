
#include "includes.h"
#include "uart.h"

void configDbgGpioUart();
void configDbgUart();
void configDbgNvicUart();

/* Config UART Debug */
void initDebugUart(void)
{
    configDbgGpioUart();    // GPIO UART Config

    configDbgUart();    // UART Config

    configDbgNvicUart();    // NVIC UART Config
}

/* Конфигурация GPIO UART Debug */
void configDbgGpioUart(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

#ifndef DBG_RX_BUFFER_SIZE
    DBG_RX_BUFFER_SIZE NULL
#endif
#if(DBG_RX_BUFFER_SIZE)
        /* RX Config ------------------------------------------- */
        /* Port Rx Clock */
        RCC_APB2PeriphClockCmd(UART_DBG_RX_PORT_CLK, ENABLE);

    /* Configure USARTy Rx as input floating */
    GPIO_InitStructure.GPIO_Pin = UART_DBG_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(UART_DBG_RX_PORT, &GPIO_InitStructure);
    /* ---------------------------------------------------- */
#endif

#ifndef DBG_TX_BUFFER_SIZE
    DBG_TX_BUFFER_SIZE NULL
#endif
#if(DBG_TX_BUFFER_SIZE)
        /* TX Config ------------------------------------------- */
        /* Port Tx Clock */
        RCC_APB2PeriphClockCmd(UART_DBG_TX_PORT_CLK, ENABLE);

    /* Configure USARTy Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = UART_DBG_TX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(UART_DBG_TX_PORT, &GPIO_InitStructure);
    /* ---------------------------------------------------- */
#endif
}

/* Конфигурация UART Debug */
void configDbgUart(void)
{
    /* UART Config ------------------------------------------- */
    /* UART Clock */
    RCC_APB2PeriphClockCmd(USART_DBG_CLK, ENABLE);

    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = DBG_BAUDRATE;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = NULL;
#if(DBG_RX_BUFFER_SIZE)
    USART_InitStructure.USART_Mode |= USART_Mode_Rx;
#endif
#if(DBG_TX_BUFFER_SIZE)
    USART_InitStructure.USART_Mode |= USART_Mode_Tx;
#endif
    USART_Init(UART_DBG_INTRERFACE, &USART_InitStructure);
    USART_Cmd(UART_DBG_INTRERFACE, ENABLE);
}

/* Конфигурация NVIC UART Debug */
void configDbgNvicUart(void)
{
    /* NVIC Configuration ------------------------------------------- */
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = UART_DBG_IRQ_CHANNEL;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = UART_DBG_PREMP_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = UART_DBG_SUB_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    NVIC_EnableIRQ(UART_DBG_IRQ_CHANNEL);    //Прерывания USART

#if(DBG_RX_BUFFER_SIZE)
    USART_ITConfig(UART_DBG_INTRERFACE, USART_IT_RXNE, ENABLE);
#endif
}