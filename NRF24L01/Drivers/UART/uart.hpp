/**
 ******************************************************************************
 * @file    uart.hpp
 * @author  Ivan Orfanidi
 * @version V1.0.1
 * @date    06/01/2019
 * @brief   This file contains all the methods prototypes for the USART
 *          firmware library.
 ******************************************************************************
 * @attention
 *
 *
 * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UART_HPP
#define __UART_HPP

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_usart.h"
#include "virtual_port.hpp"
#include <stdint.h>
#include <stdlib.h>

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

#ifdef __cplusplus

/*
 * @brief Class RTC
 */
class Uart : public VirtualPort {
  public:
    enum Default_t {
        PREEMPTION_PRIORITY = 0,
        SUB_PRIORITY = 0,

        TX_BUFFER_SIZE = 256,
        RX_BUFFER_SIZE = 256,

        BAUDRATE = 9600,
    };

    /// Ñonstructor
    Uart(USART_TypeDef* const usart,
        uint32_t baudrate = BAUDRATE,
        size_t tx_size = TX_BUFFER_SIZE,
        size_t rx_size = RX_BUFFER_SIZE,
        uint8_t preemption_priority = PREEMPTION_PRIORITY,
        uint8_t sub_priority = SUB_PRIORITY);

    virtual ~Uart();    /// Destructor

    void InitUart(uint32_t baudrate = BAUDRATE);    ///< Initialization hard UART

    void SetData(uint8_t);

    bool GetData(uint8_t* const);

    virtual size_t GetLen() override;

    virtual void WaitingCompleteTransfer() override;

    virtual void Transmit(const uint8_t*, size_t) override;

    virtual void ClearTransmit() override;

    virtual size_t Receive(uint8_t*, size_t) override;

    virtual void ClearReceive() override;

    bool GetStatusBufOverflow();

    bool Handler(USART_TypeDef* uart);

    USART_TypeDef* GetUSART();

    bool CreateClass();    ///< Returns the status of class creation

  private:
    friend void USART_HandlerCallback(USART_TypeDef*);

    void InitGpioUart();

    void DeInitGpioUart();    ///< Deinitialization GPIO for UART

    bool InitDataBuf(size_t, size_t);

    void InitClkUart();    ///< Enable Periph Clock APB for UART. APB2 for USART1
                           ///< and APB1 for USART2, USART3.

    void InitNvic(uint8_t preemption_priority = PREEMPTION_PRIORITY,
        uint8_t sub_priority = SUB_PRIORITY);    ///< Init Nested Vectored
                                                 ///< Interrupt Controller (NVIC)

    /* Transmit val */
    size_t TxBufSize;

    size_t TxWrIndex;
    size_t TxRrIndex;
    size_t TxCount;

    uint8_t* pTxBuf;

    /* Recept val */
    size_t RxBufSize;

    size_t RxWrIndex;
    size_t RxRrIndex;
    size_t RxCount;
    bool BufOverflow;

    uint8_t* pRxBuf;

    USART_TypeDef* USARTx;    ///< Work USART

    enum Uarts_t {
        UART1,
        UART2,
        UART3,

        MAX_COUNT_UART
    };

    static Uart* Uarts[MAX_COUNT_UART];    ///< Main array pointers of classes Uarts
};

extern "C" {
void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
void USART3_IRQHandler(void);
}

#endif    //__cplusplus

#endif