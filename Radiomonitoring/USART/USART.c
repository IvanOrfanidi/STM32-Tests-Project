#include "includes.h"
#include "USART.h"

uint16_t index_read_offset_uart1;
uint16_t index_read_offset_uart2;
uint16_t index_read_offset_uart3;

_Bool bGPS_Buffer_Data_Full_Half;

_Bool GpsBufNumber(void)
{
    return bGPS_Buffer_Data_Full_Half;
}

void InitUSART1(uint32_t baud_rate)
{
    // Init Structure
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_APB2PeriphClockCmd(USART1_PORT_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(USART1_CLK, ENABLE);

    // Config GPIO USART1
    /* Configure USARTy Rx as input floating */
    GPIO_InitStructure.GPIO_Pin = USART1_RxPin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(USART1_PORT, &GPIO_InitStructure);

    /* Configure USARTy Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = USART1_TxPin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(USART1_PORT, &GPIO_InitStructure);

    // USART1
    USART_InitStructure.USART_BaudRate = baud_rate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
}

void InitUSART2(uint32_t baud_rate)
{
    // Init Structure
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_APB2PeriphClockCmd(USART2_PORT_CLK, ENABLE);
    RCC_APB1PeriphClockCmd(USART2_CLK, ENABLE);

    // Config GPIO USART2
    /* Configure USARTy Rx as input floating */
    GPIO_InitStructure.GPIO_Pin = USART2_RxPin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(USART2_PORT, &GPIO_InitStructure);

    /* Configure USARTy Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = USART2_TxPin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(USART2_PORT, &GPIO_InitStructure);

    // USART2
    USART_InitStructure.USART_BaudRate = baud_rate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART2, &USART_InitStructure);
    USART_Cmd(USART2, ENABLE);
}

void InitUSART3(uint32_t baud_rate)
{
    // Init Structure
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_APB2PeriphClockCmd(USART3_PORT_CLK, ENABLE);
    RCC_APB1PeriphClockCmd(USART3_CLK, ENABLE);

    // Config GPIO USART3
    /* Configure USARTy Rx as input floating */
    GPIO_InitStructure.GPIO_Pin = USART3_RxPin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(USART3_PORT, &GPIO_InitStructure);

    /* Configure USARTy Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = USART3_TxPin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(USART3_PORT, &GPIO_InitStructure);

    // USART3
    USART_InitStructure.USART_BaudRate = baud_rate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART3, &USART_InitStructure);

    USART_Cmd(USART3, ENABLE);
}

void DMA_USART1_Configuration(void)
{
    DMA_InitTypeDef DMA_InitStructure;
    /* DMA clock enable */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    /* DISABLE USART1 TX DMA1 Channel */
    DMA_Cmd(USART1_Tx_DMA_Channel, DISABLE);
    /* DISABLE USART1 RX DMA1 Channel */
    DMA_Cmd(USART1_Rx_DMA_Channel, DISABLE);

    /* USART1 TX DMA1 Channel Config */
    DMA_DeInit(USART1_Tx_DMA_Channel);
    DMA_InitStructure.DMA_PeripheralBaseAddr = USART1_DR_Base;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)g_aucTxBufferUSART1;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = TX_BUFFER_SIZE1;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(USART1_Tx_DMA_Channel, &DMA_InitStructure);

    /* USART1 RX DMA1 Channel Config */
    DMA_DeInit(USART1_Rx_DMA_Channel);
    DMA_InitStructure.DMA_PeripheralBaseAddr = USART1_DR_Base;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)g_aucRxBufferUSART1;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = RX_BUFFER_SIZE1;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(USART1_Rx_DMA_Channel, &DMA_InitStructure);

    /* Enable USART1 DMA Rx and TX request */
    USART_DMACmd(USART1, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);

    DMA_SetCurrDataCounter(USART1_Tx_DMA_Channel, TX_BUFFER_SIZE1);
    DMA_SetCurrDataCounter(USART1_Tx_DMA_Channel, RX_BUFFER_SIZE1);

    /* Enable USART1 TX DMA1 Channel */
    DMA_Cmd(USART1_Tx_DMA_Channel, ENABLE);
    /* Enable USART1 RX DMA1 Channel */
    DMA_Cmd(USART1_Rx_DMA_Channel, ENABLE);

    // Interrupts
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    DMA_ITConfig(USART1_Rx_DMA_Channel, DMA_IT_TC | DMA_IT_TE, ENABLE);

    index_read_offset_uart1 = 0;
}

void DMA_USART2_Configuration(void)
{
    DMA_InitTypeDef DMA_InitStructure;
    /* DMA clock enable */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    /* DISABLE USART2 TX DMA1 Channel */
    DMA_Cmd(USART2_Tx_DMA_Channel, DISABLE);
    /* DISABLE USART2 RX DMA1 Channel */
    DMA_Cmd(USART2_Rx_DMA_Channel, DISABLE);

    /* USART2 TX DMA1 Channel Config */
    DMA_DeInit(USART2_Tx_DMA_Channel);
    DMA_InitStructure.DMA_PeripheralBaseAddr = USART2_DR_Base;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)g_aucTxBufferUSART2;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = TX_BUFFER_SIZE2;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(USART2_Tx_DMA_Channel, &DMA_InitStructure);

    /* USART2 RX DMA1 Channel Config */
    DMA_DeInit(USART2_Rx_DMA_Channel);
    DMA_InitStructure.DMA_PeripheralBaseAddr = USART2_DR_Base;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)g_aucRxBufferUSART2;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = RX_BUFFER_SIZE2;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(USART2_Rx_DMA_Channel, &DMA_InitStructure);

    /* Enable USART2 DMA Rx and TX request */
    USART_DMACmd(USART2, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);

    DMA_SetCurrDataCounter(USART2_Tx_DMA_Channel, TX_BUFFER_SIZE2);

    DMA_SetCurrDataCounter(USART2_Rx_DMA_Channel, RX_BUFFER_SIZE2);

    /* Enable USART2 TX DMA1 Channel */
    DMA_Cmd(USART2_Tx_DMA_Channel, ENABLE);
    /* Enable USART2 RX DMA1 Channel */
    DMA_Cmd(USART2_Rx_DMA_Channel, ENABLE);

    // Interrupts
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    DMA_ITConfig(USART2_Rx_DMA_Channel, DMA_IT_TC | DMA_IT_TE, ENABLE);

    index_read_offset_uart2 = 0;
}

void DMA_USART3_Configuration(void)
{
    DMA_InitTypeDef DMA_InitStructure;

    /* DMA clock enable */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    /* USART3 TX DMA1 Channel Config */
    DMA_DeInit(USART3_Tx_DMA_Channel);
    DMA_InitStructure.DMA_PeripheralBaseAddr = USART3_DR_Base;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)g_aucTxBufferUSART3;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = TX_BUFFER_SIZE3;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(USART3_Tx_DMA_Channel, &DMA_InitStructure);

    /* USART3 RX DMA1 Channel Config */
    DMA_DeInit(USART3_Rx_DMA_Channel);
    DMA_InitStructure.DMA_PeripheralBaseAddr = USART3_DR_Base;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)g_aucRxBufferUSART3;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = RX_BUFFER_SIZE3;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(USART3_Rx_DMA_Channel, &DMA_InitStructure);

    /* Enable USART3 DMA Rx and TX request */
    USART_DMACmd(USART3, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);

    DMA_SetCurrDataCounter(USART3_Tx_DMA_Channel, TX_BUFFER_SIZE3);
    DMA_SetCurrDataCounter(USART3_Tx_DMA_Channel, RX_BUFFER_SIZE3);

    /* Enable USART3 TX DMA1 Channel */
    DMA_Cmd(USART3_Tx_DMA_Channel, ENABLE);
    /* Enable USART3 RX DMA1 Channel */
    DMA_Cmd(USART3_Rx_DMA_Channel, ENABLE);

    // Interrupts
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    DMA_ITConfig(USART3_Rx_DMA_Channel, DMA_IT_TC | DMA_IT_HT, ENABLE);

    index_read_offset_uart3 = 0;
}

void DMA1_Channel3_IRQHandler(void)
{
    USART1->CR1 &= ~USART_CR1_RE;
    USART1->CR3 &= ~USART_CR3_DMAR;
    if(DMA_GetITStatus(DMA1_IT_TC3))    // Full
    {
        DMA_ClearITPendingBit(DMA1_IT_TC3);
        DMA_ClearITPendingBit(DMA1_IT_GL3);
    }

    if(DMA_GetITStatus(DMA1_IT_TE3)) {
        DMA_ClearITPendingBit(DMA1_IT_TE3);
    }
}

void DMA1_Channel6_IRQHandler(void)
{
    USART2->CR1 &= ~USART_CR1_RE;
    USART2->CR3 &= ~USART_CR3_DMAR;
    if(DMA_GetITStatus(DMA1_IT_TC6))    // Full
    {
        DMA_ClearITPendingBit(DMA1_IT_TC6);
        DMA_ClearITPendingBit(DMA1_IT_GL6);
    }

    if(DMA_GetITStatus(DMA1_IT_TE6)) {
        DMA_ClearITPendingBit(DMA1_IT_TE6);
    }
}

void DMA1_Channel5_IRQHandler(void)
{
    if(DMA_GetITStatus(DMA1_IT_TC5))    // Full
    {
        bGPS_Buffer_Data_Full_Half = 1;
        DMA_ClearITPendingBit(DMA1_IT_TC5);
    }

    if(DMA_GetITStatus(DMA1_IT_HT5))    // Half
    {
        bGPS_Buffer_Data_Full_Half = 0;
        DMA_ClearITPendingBit(DMA1_IT_HT5);
    }
}

// Работа по DMA //
void USART1_Read(char* pData_Usart, uint16_t Len)
{
    uint16_t index_read;

    for(index_read = 0; index_read < Len; index_read++) {
        pData_Usart[index_read] = g_aucRxBufferUSART1[index_read_offset_uart1];
        g_aucRxBufferUSART1[index_read] = 0;

        index_read_offset_uart1++;
    }

    if((RX_BUFFER_SIZE1 - DMA_GetCurrDataCounter(USART1_Rx_DMA_Channel)) <= index_read_offset_uart1) {
        DMA_Cmd(USART1_Rx_DMA_Channel, DISABLE);
        DMA_SetCurrDataCounter(USART1_Rx_DMA_Channel, RX_BUFFER_SIZE1);
        index_read_offset_uart1 = 0;
        DMA_Cmd(USART1_Rx_DMA_Channel, ENABLE);
    }
}

void USART1_Write(const char* pData_Usart, uint16_t Len)
{
    uint16_t index_write;
    uint32_t wd = 0xFFFFF;

    while(wd && DMA_GetCurrDataCounter(USART1_Tx_DMA_Channel)) {
        wd--;    // freeze protection
    }

    DMA_Cmd(USART1_Tx_DMA_Channel, DISABLE);

    for(index_write = 0; index_write < Len; index_write++) {
        g_aucTxBufferUSART1[index_write] = pData_Usart[index_write];
    }
    DMA_SetCurrDataCounter(USART1_Tx_DMA_Channel, Len);
    DMA_Cmd(USART1_Tx_DMA_Channel, ENABLE);
}

uint16_t USART1_Rx_Len(void)
{
    return (RX_BUFFER_SIZE1 - DMA_GetCurrDataCounter(USART1_Rx_DMA_Channel));
}

//*********************************************//

void USART2_Read(char* pData_Usart, uint16_t Len)
{
    uint16_t index_read;
    for(index_read = 0; index_read < Len; index_read++) {
        pData_Usart[index_read] = g_aucRxBufferUSART2[index_read_offset_uart2];
        g_aucRxBufferUSART2[index_read_offset_uart2] = 0;

        index_read_offset_uart2++;
    }

    if((RX_BUFFER_SIZE2 - DMA_GetCurrDataCounter(USART2_Rx_DMA_Channel)) <= index_read_offset_uart2) {
        DMA_Cmd(USART2_Rx_DMA_Channel, DISABLE);
        DMA_SetCurrDataCounter(USART2_Rx_DMA_Channel, RX_BUFFER_SIZE2);
        index_read_offset_uart2 = 0;
        DMA_Cmd(USART2_Rx_DMA_Channel, ENABLE);
    }
}

void USART2_Write(const char* pData_Usart, uint16_t Len)
{
    uint16_t index_write;
    uint32_t wd = 0xFFFFF;

    while(wd && DMA_GetCurrDataCounter(USART2_Tx_DMA_Channel)) {
        wd--;    // freeze protection
    }

    // while(DMA_GetCurrDataCounter(USART2_Tx_DMA_Channel));

    DMA_Cmd(USART2_Tx_DMA_Channel, DISABLE);

    for(index_write = 0; index_write < Len; index_write++) {
        g_aucTxBufferUSART2[index_write] = pData_Usart[index_write];
    }

    DMA_SetCurrDataCounter(USART2_Tx_DMA_Channel, Len);
    DMA_Cmd(USART2_Tx_DMA_Channel, ENABLE);
}

uint16_t USART2_Rx_Len(void)
{
    return (RX_BUFFER_SIZE2 - DMA_GetCurrDataCounter(USART2_Rx_DMA_Channel));
}

//*********************************************//

void USART3_Read(char* pData_Usart, uint16_t Len)
{
    uint16_t index_read;
    for(index_read = 0; index_read < Len; index_read++) {
        pData_Usart[index_read] = g_aucRxBufferUSART3[index_read_offset_uart3];
        g_aucRxBufferUSART3[index_read] = 0;

        index_read_offset_uart3++;
    }

    if((RX_BUFFER_SIZE3 - DMA_GetCurrDataCounter(USART3_Rx_DMA_Channel)) <= index_read_offset_uart3) {
        DMA_Cmd(USART3_Rx_DMA_Channel, DISABLE);
        DMA_SetCurrDataCounter(USART3_Rx_DMA_Channel, RX_BUFFER_SIZE3);
        index_read_offset_uart3 = 0;
        DMA_Cmd(USART3_Rx_DMA_Channel, ENABLE);
    }
}

void USART3_Write(const char* pData_Usart, uint16_t Len)
{
    uint16_t index_write;
    uint32_t wd = 0xFFFFF;

    while(wd && DMA_GetCurrDataCounter(USART3_Tx_DMA_Channel)) {
        wd--;    // freeze protection
    }

    // while(DMA_GetCurrDataCounter(USART3_Tx_DMA_Channel));

    DMA_Cmd(USART3_Tx_DMA_Channel, DISABLE);

    for(index_write = 0; index_write < Len; index_write++) {
        g_aucTxBufferUSART3[index_write] = pData_Usart[index_write];
    }
    DMA_SetCurrDataCounter(USART3_Tx_DMA_Channel, Len);
    DMA_Cmd(USART3_Tx_DMA_Channel, ENABLE);
}

uint16_t USART3_Rx_Len(void)
{
    return (RX_BUFFER_SIZE3 - DMA_GetCurrDataCounter(USART3_Rx_DMA_Channel));
}

//*********************************************//

// Доп. функции облегчающие работу с USART //

void USART_Write(uint8_t NumUSART, const char* pData_Usart, uint16_t Len)
{
    switch(NumUSART) {
        case 1:
            USART1_Write(pData_Usart, Len);
            break;
        case 2:
            USART2_Write(pData_Usart, Len);
            break;
        case 3:
            USART3_Write(pData_Usart, Len);
            break;
    }
}

void USART_Read(uint8_t NumUSART, char* pData_Usart, uint16_t Len)
{
    switch(NumUSART) {
        case 1:
            USART1_Read(pData_Usart, Len);
            break;
        case 2:
            USART2_Read(pData_Usart, Len);
            break;
        case 3:
            USART3_Read(pData_Usart, Len);
            break;
    }
}

uint16_t USART_Rx_Len(uint8_t NumUSART)
{
    uint16_t Len;

    switch(NumUSART) {
        case 1:
            Len = USART1_Rx_Len();
            break;
        case 2:
            Len = USART2_Rx_Len();
            break;
        case 3:
            Len = USART3_Rx_Len();
            break;
    }

    return Len;
}

void InitUSART(uint8_t NumUSART, uint32_t baud_rate)
{
    switch(NumUSART) {
        case 1:
            InitUSART1(baud_rate);
            break;
        case 2:
            InitUSART2(baud_rate);
            break;
        case 3:
            InitUSART3(baud_rate);
            break;
    }
}

void InitDMA(uint8_t NumUSART)
{
    switch(NumUSART) {
        case 1:
            DMA_USART1_Configuration();
            break;
        case 2:
            DMA_USART2_Configuration();
            break;
        case 3:
            DMA_USART3_Configuration();
            break;
    }
}

void DeInitUSART(uint8_t NumUSART)
{
    switch(NumUSART) {
        case 1:
            DeInitUSART1();
            break;
        case 2:
            DeInitUSART2();
            break;
        case 3:
            DeInitUSART3();
            break;
    }
}

void DeInitUSART1(void)
{
    USART_DeInit(USART1);
    DeInitDMA(1);
}

void DeInitUSART2(void)
{
    USART_DeInit(USART2);
    DeInitDMA(2);
}

void DeInitUSART3(void)
{
    USART_DeInit(USART3);
    DeInitDMA(3);
}

void DeInitDMA(uint8_t NumDMA)
{
    switch(NumDMA) {
        case 1:
            DeInitDMA1();
            break;
        case 2:
            DeInitDMA2();
            break;
        case 3:
            DeInitDMA3();
            break;
    }
}

void DeInitDMA1(void)
{
    DMA_SetCurrDataCounter(USART1_Tx_DMA_Channel, TX_BUFFER_SIZE1);
    DMA_SetCurrDataCounter(USART1_Tx_DMA_Channel, RX_BUFFER_SIZE1);
    /* DISABLE USART3 TX DMA1 Channel */
    DMA_Cmd(USART1_Tx_DMA_Channel, DISABLE);
    /* Enable USART3 RX DMA1 Channel */
    DMA_Cmd(USART1_Rx_DMA_Channel, DISABLE);

    /* Enable USART3 DMA Rx and TX request */
    USART_DMACmd(USART1, USART_DMAReq_Rx | USART_DMAReq_Tx, DISABLE);

    DMA_DeInit(USART1_Tx_DMA_Channel);
    DMA_DeInit(USART1_Rx_DMA_Channel);
}

void DeInitDMA2(void)
{
    DMA_SetCurrDataCounter(USART2_Tx_DMA_Channel, TX_BUFFER_SIZE2);
    DMA_SetCurrDataCounter(USART2_Tx_DMA_Channel, RX_BUFFER_SIZE2);
    /* DISABLE USART3 TX DMA1 Channel */
    DMA_Cmd(USART2_Tx_DMA_Channel, DISABLE);
    /* Enable USART3 RX DMA1 Channel */
    DMA_Cmd(USART2_Rx_DMA_Channel, DISABLE);

    /* Enable USART3 DMA Rx and TX request */
    USART_DMACmd(USART2, USART_DMAReq_Rx | USART_DMAReq_Tx, DISABLE);

    DMA_DeInit(USART2_Tx_DMA_Channel);
    DMA_DeInit(USART2_Rx_DMA_Channel);
}

void DeInitDMA3(void)
{
    DMA_SetCurrDataCounter(USART3_Tx_DMA_Channel, TX_BUFFER_SIZE3);
    DMA_SetCurrDataCounter(USART3_Tx_DMA_Channel, RX_BUFFER_SIZE3);
    /* DISABLE USART3 TX DMA1 Channel */
    DMA_Cmd(USART3_Tx_DMA_Channel, DISABLE);
    /* Enable USART3 RX DMA1 Channel */
    DMA_Cmd(USART3_Rx_DMA_Channel, DISABLE);

    /* Enable USART3 DMA Rx and TX request */
    USART_DMACmd(USART3, USART_DMAReq_Rx | USART_DMAReq_Tx, DISABLE);

    DMA_DeInit(USART3_Tx_DMA_Channel);
    DMA_DeInit(USART3_Rx_DMA_Channel);
}

void ReStartDmaUsart1(void)
{
    DMA_Cmd(USART1_Rx_DMA_Channel, DISABLE);    // отключаем канал
    USART1_Rx_DMA_Channel->CCR &=
        ~DMA_CCR5_MINC;                                                // переключаем ля того чтобы сбросить внутреннее смещение дма по буферу
    DMA_SetCurrDataCounter(USART1_Rx_DMA_Channel, RX_BUFFER_SIZE1);    // сбрасываем содержимое буфера
    USART1_Rx_DMA_Channel->CCR |= DMA_CCR5_MINC;                       //включаем все как было
    memset(g_aucRxBufferUSART1, 0, sizeof(g_aucRxBufferUSART1));       // очищаем сам буфер на всякий пожарный
    DMA_Cmd(USART1_Rx_DMA_Channel, ENABLE);                            //включаем канал
    USART1->CR1 |= USART_CR1_RE;                                       //разрешаем прием символов от GSM модуля
    //_delay_ms(100);                          //ждем пока UART войдет в рабочее стостояние
    while(USART1->SR & USART_SR_RXNE) {    //если есть данные в аппаратном буфере UART то
        int i = USART1->DR;                //удаляем их - это старые данные пришедшие уже после отключения DMA
    }
    USART1->CR3 |= USART_CR3_DMAR;    //разрешаем передачу от uart к dma
    index_read_offset_uart1 = 0;      //начинаем с чистого буфера - устанавливаем смещение чтения на 0
}

void ReStartDmaUsart2(void)
{
    DMA_Cmd(USART2_Rx_DMA_Channel, DISABLE);    // отключаем канал
    USART2_Rx_DMA_Channel->CCR &=
        ~DMA_CCR6_MINC;                                                // переключаем ля того чтобы сбросить внутреннее смещение дма по буферу
    DMA_SetCurrDataCounter(USART2_Rx_DMA_Channel, RX_BUFFER_SIZE2);    // сбрасываем содержимое буфера
    USART2_Rx_DMA_Channel->CCR |= DMA_CCR6_MINC;                       //включаем все как было
    memset(g_aucRxBufferUSART2, 0, sizeof(g_aucRxBufferUSART2));       // очищаем сам буфер на всякий пожарный
    DMA_Cmd(USART2_Rx_DMA_Channel, ENABLE);                            //включаем канал
    USART2->CR1 |= USART_CR1_RE;                                       //разрешаем прием символов от GSM модуля
    //_delay_ms(100);                          //ждем пока UART войдет в рабочее стостояние
    while(USART2->SR & USART_SR_RXNE) {    //если есть данные в аппаратном буфере UART то
        int i = USART2->DR;                //удаляем их - это старые данные пришедшие уже после отключения DMA
    }
    USART2->CR3 |= USART_CR3_DMAR;    //разрешаем передачу от uart к dma
    index_read_offset_uart2 = 0;      //начинаем с чистого буфера - устанавливаем смещение чтения на 0
}

void ReStartDmaUsart3(void)
{
    DMA_Cmd(USART3_Rx_DMA_Channel, DISABLE);    // отключаем канал
    USART3_Rx_DMA_Channel->CCR &=
        ~DMA_CCR3_MINC;                                                // переключаем ля того чтобы сбросить внутреннее смещение дма по буферу
    DMA_SetCurrDataCounter(USART2_Rx_DMA_Channel, RX_BUFFER_SIZE3);    // сбрасываем содержимое буфера
    USART3_Rx_DMA_Channel->CCR |= DMA_CCR3_MINC;                       //включаем все как было
    memset(g_aucRxBufferUSART3, 0, sizeof(g_aucRxBufferUSART3));       // очищаем сам буфер на всякий пожарный
    DMA_Cmd(USART3_Rx_DMA_Channel, ENABLE);                            //включаем канал
    USART3->CR1 |= USART_CR1_RE;                                       //разрешаем прием символов от GSM модуля
    //_delay_ms(SLEEP_MS_100);                          //ждем пока UART войдет в рабочее стостояние
    while(USART3->SR & USART_SR_RXNE) {    //если есть данные в аппаратном буфере UART то
        int i = USART3->DR;                //удаляем их - это старые данные пришедшие уже после отключения DMA
    }
    USART3->CR3 |= USART_CR3_DMAR;    //разрешаем передачу от uart к dma
    index_read_offset_uart3 = 0;      //начинаем с чистого буфера - устанавливаем смещение чтения на 0
}
