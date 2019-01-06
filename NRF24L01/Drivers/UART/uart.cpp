/**
 ******************************************************************************
 * @file    uart.c
 * @author  Ivan Orfanidi
 * @version V1.0.0
 * @date    28-March-2018
 * @brief   This file provides all the USART firmware method.
 ******************************************************************************
 * @attention
 *
 *
 * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "uart.hpp"

/*
 * Main array pointers of classes Uarts
 * Use MAX_COUNT_UART max quantity Uarts
 */
Uart* Uart::Uarts[Uart::MAX_COUNT_UART];


/**
 * Member functions definitions including constructor.
 */
Uart::Uart(USART_TypeDef* const usart, uint32_t baudrate, size_t tx_size, size_t rx_size, uint8_t preemption_priority, uint8_t sub_priority)
{
    USARTx = nullptr;
    
    if(usart == USART1) {
        if((Uarts[UART1]) || (!(InitDataBuf(tx_size, rx_size)))) {
            return;
        }
        Uarts[UART1] = this;
    }
    else if(usart == USART2) {
        if((Uarts[UART2]) || (!(InitDataBuf(tx_size, rx_size)))) {
            return;
        }
        Uarts[UART2] = this;
    }
    else if(usart == USART3) {
        if((Uarts[UART3]) || (!(InitDataBuf(tx_size, rx_size)))) {
            return;
        }
        
        Uarts[UART3] = this;
    }
    else {
        return;
    }
    
    USARTx = usart;    // Assign work USART

    // Init GPIO USART
    InitGpioUart();

    // Init Clock Uart
    InitClkUart();

    // Init USART
    InitUart(baudrate);

    // Init USART Interrupt
    InitNvic(preemption_priority, sub_priority);
}

/**
 * @brief Deconstructor class Uart
 */
Uart::~Uart()
{
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    
    if(USARTx == USART1) {
        if(Uarts[UART1] == nullptr) {
            return;
        }
        
        NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
        
        USART_Cmd(USARTx, DISABLE);
        USART_DeInit(USARTx);
        RCC_APB2PeriphClockCmd(USART1_CLK, DISABLE);
        
        Uarts[UART1] = nullptr;
    }
    else if(USARTx == USART2) {
        if(Uarts[UART2] == nullptr) {
            return;
        }
        
        NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
        
        USART_Cmd(USARTx, DISABLE);
        USART_DeInit(USARTx);
        RCC_APB1PeriphClockCmd(USART2_CLK, DISABLE);
        
        Uarts[UART2] = nullptr;
    }
    else if(USARTx == USART3) {
        if(Uarts[UART3] == nullptr) {
            return;
        }
        
        NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
        
        USART_Cmd(USARTx, DISABLE);
        USART_DeInit(USARTx);
        RCC_APB1PeriphClockCmd(USART3_CLK, DISABLE);
        
        Uarts[UART3] = nullptr;
    }
    else {
        return;
    }
    
    // Waiting for complete the transfer of
    WaitingCompleteTransfer();

    USART_ITConfig(USARTx, USART_IT_TXE, DISABLE);    // Disable the USARTx Receive interrupt
    USART_ITConfig(USARTx, USART_IT_RXNE, DISABLE);    // Disable the USARTx Transmit interrupt

    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);

    DeInitGpioUart();

    if(pTxBuf) {
        delete[] pTxBuf;
    }

    if(pRxBuf) {
        delete[] pRxBuf;
    }
}


bool Uart::InitDataBuf(size_t tx_size, size_t rx_size)
{
    /* Create memory */
    pTxBuf = new uint8_t[tx_size];
    if(!(pTxBuf)) {
        return false;
    }
    pRxBuf = new uint8_t[rx_size];
    if(!(pRxBuf)) {
        return false;
    }
    
    TxBufSize = tx_size;
    RxBufSize = rx_size;

    TxWrIndex = 0;
    TxRrIndex = 0;
    TxCount = 0;

    RxWrIndex = 0;
    RxRrIndex = 0;
    RxCount = 0;
    BufOverflow = false;
    
    return true;
}


/**
 * @brief Wait complete transfer data/
 */
void Uart::WaitingCompleteTransfer()
{
    while(TxCount);
    while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
    while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);
}


/**
 * @brief  Initialization GPIO for UART.
 * @retval None.
 */
void Uart::DeInitGpioUart()
{
    /* Configure USARTy Rx Pin */
    GPIO_InitTypeDef GPIO_RxPin_InitStructure;
    GPIO_RxPin_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    /* Configure USARTy Tx Pin */
    GPIO_InitTypeDef GPIO_TxPin_InitStructure;
    GPIO_TxPin_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    /* Configure USARTy Port */
    GPIO_TypeDef *GPIO_Port;

    switch((uint32_t)USARTx)
    {
        case(uint32_t)USART1:
            RCC_APB1PeriphClockCmd(USART1_CLK, DISABLE);
            GPIO_Port = USART1_PORT;
            GPIO_RxPin_InitStructure.GPIO_Pin = USART1_RxPin;
            GPIO_TxPin_InitStructure.GPIO_Pin = USART1_TxPin;
            break;

        case(uint32_t)USART2:
            RCC_APB1PeriphClockCmd(USART2_CLK, DISABLE);
            GPIO_Port = USART2_PORT;
            GPIO_RxPin_InitStructure.GPIO_Pin = USART2_RxPin;
            GPIO_TxPin_InitStructure.GPIO_Pin = USART2_TxPin;
            break;

        case(uint32_t)USART3:
            RCC_APB1PeriphClockCmd(USART3_CLK, DISABLE);
            GPIO_Port = USART3_PORT;
            GPIO_RxPin_InitStructure.GPIO_Pin = USART3_RxPin;
            GPIO_TxPin_InitStructure.GPIO_Pin = USART3_TxPin;
            break;

        default:
            return;
    }

    GPIO_Init(GPIO_Port, &GPIO_RxPin_InitStructure);
    GPIO_Init(GPIO_Port, &GPIO_TxPin_InitStructure);
}


/**
 * @brief  Enable Periph Clock APB for UART. APB2 for USART1 and APB1 for
 * USART2, USART3.
 * @retval None.
 */
void Uart::InitClkUart()
{
    switch((uint32_t)USARTx)
    {
        case(uint32_t)USART1:
            RCC_APB2PeriphClockCmd(USART1_CLK, ENABLE);
            break;

        case(uint32_t)USART2:
            RCC_APB1PeriphClockCmd(USART2_CLK, ENABLE);
            break;

        case(uint32_t)USART3:
            RCC_APB1PeriphClockCmd(USART3_CLK, ENABLE);
            break;

        default:
            return;
    }
}


/**
 * @brief  Initialization GPIO for UART.
 * @retval None.
 */
void Uart::InitGpioUart()
{
    /* Configure USARTy Rx Pin */
    GPIO_InitTypeDef GPIO_RxPin_InitStructure;
    /* Configure USARTy Tx Pin */
    GPIO_InitTypeDef GPIO_TxPin_InitStructure;
    /* Configure USARTy Port */
    GPIO_TypeDef *GPIO_Port;

    switch((uint32_t)USARTx)
    {
        case(uint32_t)USART1:
            RCC_APB2PeriphClockCmd(USART1_PORT_CLK, ENABLE);
            GPIO_Port = USART1_PORT;
            GPIO_RxPin_InitStructure.GPIO_Pin = USART1_RxPin;
            GPIO_TxPin_InitStructure.GPIO_Pin = USART1_TxPin;
            break;

        case(uint32_t)USART2:
            RCC_APB2PeriphClockCmd(USART2_PORT_CLK, ENABLE);
            GPIO_Port = USART2_PORT;
            GPIO_RxPin_InitStructure.GPIO_Pin = USART2_RxPin;
            GPIO_TxPin_InitStructure.GPIO_Pin = USART2_TxPin;
            break;

        case(uint32_t)USART3:
            RCC_APB2PeriphClockCmd(USART3_PORT_CLK, ENABLE);
            GPIO_Port = USART3_PORT;
            GPIO_RxPin_InitStructure.GPIO_Pin = USART3_RxPin;
            GPIO_TxPin_InitStructure.GPIO_Pin = USART3_TxPin;
            break;

        default:
            return;
    }

    /* Configure USARTy Rx as input floating */
    GPIO_RxPin_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIO_Port, &GPIO_RxPin_InitStructure);

    /* Configure USARTy Tx as alternate function push-pull */
    GPIO_TxPin_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_TxPin_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIO_Port, &GPIO_TxPin_InitStructure);
}


/*
 * @brief  Initialization hard UART.
 * @retval None.
 */
void Uart::InitUart(uint32_t baudrate)
{
    USART_Cmd(USARTx, DISABLE);

    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = 0;
    if(pTxBuf) {
        USART_InitStructure.USART_Mode |= USART_Mode_Tx;
    }
    if(pRxBuf) {
        USART_InitStructure.USART_Mode |= USART_Mode_Rx;
    }
    
    USART_Init(USARTx, &USART_InitStructure);

    USART_Cmd(USARTx, ENABLE);
}


/**
 * @brief  Initialization Nested Vectored Interrupt Controller (NVIC).
 * @retval None.
 */
void Uart::InitNvic(uint8_t preemption_priority, uint8_t sub_priority)
{
    /* Enable the USARTx Interrupt */
    NVIC_InitTypeDef NVIC_InitStructure;

    switch((uint32_t)USARTx)
    {
        case(uint32_t)USART1:
            NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
            break;

        case(uint32_t)USART2:
            NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
            break;

        case(uint32_t)USART3:
            NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
            break;

        default:
            return;
    }
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = preemption_priority;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = sub_priority;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Enable the USART1 Receive interrupt: this interrupt is generated when the
       USARTx receive data register is not empty */
    USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);
}


/**
 * @brief  Transmits single data through the USARTx peripheral.
 * @param  Data: the data to transmit.
 * @retval None
 */
void Uart::SetData(uint8_t data)
{
    while(TxCount == TxBufSize);    // Buffer overflow. Waiting for clearing.

    USART_ITConfig(USARTx, USART_IT_TXE, DISABLE);

    if(TxCount || (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET))
    {
        pTxBuf[TxWrIndex++] = data;

        if(TxWrIndex == TxBufSize)
            TxWrIndex = 0;    //

        ++TxCount;

        USART_ITConfig(USARTx, USART_IT_TXE, ENABLE);    // Enable the USARTx Transmit interrupt
    }
    else
    {
        USART_SendData(USARTx, data);
        USART_ITConfig(USARTx, USART_IT_TXE, ENABLE);    // Enable the USARTx Transmit interrupt
    }
}


/**
 * @brief  Returns the most recent received data by the USARTx peripheral.
 * @param  pointer to data buffer.
 * @retval No data.
 */
bool Uart::GetData(uint8_t *const ptr)
{
    if(0 == RxCount) {    // No data.
        return false;
    }

    uint8_t data = pRxBuf[RxRrIndex++];
    if(RxRrIndex >= RxBufSize) {
        RxRrIndex = 0;
    }

    USART_ITConfig(USARTx, USART_IT_RXNE, DISABLE);    // Disable USART interrupt
    --RxCount;                                         // Decrement counter
    USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);     // Enable USART interrupt

    *ptr = data;
    return true;
}


size_t Uart::GetLen()
{
    return RxCount;
}


bool Uart::GetStatusBufOverflow()
{
    return BufOverflow;
}


void Uart::Transmit(const uint8_t *ptr, size_t len)
{
    if(USARTx == nullptr) {
        return;
    }
    
    while(len--) {
        SetData(*ptr++);
    }
}


void Uart::ClearTransmit()
{
    TxWrIndex = 0;
    TxRrIndex = 0;
    TxCount = 0;
}


size_t Uart::Receive(uint8_t *ptr, size_t len)
{
    uint32_t real_len = 0;
    while(len--) {
        if(GetData(ptr++) == false) {
            real_len++;
            break;
        }
    }
    return real_len;
}


void Uart::ClearReceive()
{
    RxWrIndex = 0;
    RxRrIndex = 0;
    RxCount = 0;
    BufOverflow = false;
}


/**
 * @brief  Returns the work USART.
 * @retval pointer to USARTx.
 */
USART_TypeDef *Uart::GetUSART()
{
    return USARTx;
}


/**
 * @brief  Returns the status of class creation.
 * @retval true -  Success;
 *         false - Fail.
 */
bool Uart::CreateClass()
{
    return (USARTx != nullptr);
}


/*
 * INTERRUPTS
 */
bool Uart::Handler(USART_TypeDef *uart)
{
    if((uint32_t)uart == (uint32_t)USARTx)
    {
        /* Transmit Data */
        if(USART_GetITStatus(USARTx, USART_IT_TXE) == SET)    // Interruption for empty transmit register
        {
            if(TxCount) {
                TxCount--;
                /* Write one byte to the Transmit data register */
                USART_SendData(USARTx, pTxBuf[TxRrIndex++]);

                /* Go around the circle */
                if(TxRrIndex >= TxBufSize) {
                    TxRrIndex = 0;
                }
            }
            else {
                /* Disable the USARTx Transmit interrupt */
                USART_ITConfig(USARTx, USART_IT_TXE, DISABLE);
            }
        }
        /*****************/

        /* Receive Data */
        if(USART_GetITStatus(USARTx, USART_IT_RXNE) == SET)    // The interrupt for data reception.
        {
            // Checking for data reception errors
            if(USART_GetFlagStatus(USARTx, USART_FLAG_NE) || USART_GetFlagStatus(USARTx, USART_FLAG_FE) ||
               USART_GetFlagStatus(USARTx, USART_FLAG_PE)) {
                USART_ReceiveData(USARTx);    // If was errors
            }
            else {
                uint8_t data = (uint8_t)(USART_ReceiveData(USARTx) & 0x00FF);
                pRxBuf[RxWrIndex++] = data;

                if(RxWrIndex >= RxBufSize) {                     // Checking go to begin
                    RxWrIndex = 0;    // Go to begin circle
                }

                if(++RxCount >= RxBufSize) {    // Checking for overflow
                    RxCount = 0;
                    BufOverflow = true;
                }
            }
        }

        return true;
    }
    return false;
}

/**
 * @brief This function must be called from all global interruptions USARTs.
 */
void USART_HandlerCallback(USART_TypeDef *uart)
{
    for(auto i = 0; i < Uart::MAX_COUNT_UART; i++) {
        if(Uart::Uarts[i]) {
            if(Uart::Uarts[i]->Handler(uart)) {
                break;
            }
        }
    }
}

/**
 * @brief  This function handles USARTs global interrupt request.
 * @param  None
 * @retval None
 */
void USART1_IRQHandler(void)
{
    USART_HandlerCallback(USART1);
}

void USART2_IRQHandler(void)
{
    USART_HandlerCallback(USART2);
}

void USART3_IRQHandler(void)
{
    USART_HandlerCallback(USART3);
}
