

#include "USART.h"

unsigned char RxBufferUSART1[RX_BUFFER_SIZE1];    //Буфер Rx USART
unsigned char TxBufferUSART1[TX_BUFFER_SIZE1];    //Буфер Tx USART

uint16_t rx_counter1;

DMA_InitTypeDef DMA_InitStructure;

void InitUSART1(uint32_t baud_rate)
{
    // Init Structure
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    //Config GPIO USART1
    /* Configure USARTy Rx as input floating */
    GPIO_InitStructure.GPIO_Pin = USART_TXD;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure USARTy Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = USART_RXD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //USART1
    USART_InitStructure.USART_BaudRate = baud_rate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART1, &USART_InitStructure);

    /* NVIC configuration USAR1*/
    /*
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_PriorityGroupConfig(USART1_NVIC_GROUP);    
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = USART1_NVIC_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	
    NVIC_Init(&NVIC_InitStructure);
    NVIC_EnableIRQ (USART1_IRQn);                        //Прерывания USART1
    
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);  
    USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
    */
    /* Enable USART1 */
    USART_Cmd(USART1, ENABLE);
}

void DMA_USART_Configuration(void)
{
    /* DMA clock enable */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    /* USARTy TX DMA1 Channel (triggered by USARTy Tx event) Config */
    DMA_DeInit(USART1_Tx_DMA_Channel);
    DMA_InitStructure.DMA_PeripheralBaseAddr = USART1_DR_Base;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)TxBufferUSART1;
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

    /* USARTy RX DMA1 Channel (triggered by USARTy Rx event) Config */
    DMA_DeInit(USART1_Rx_DMA_Channel);
    DMA_InitStructure.DMA_PeripheralBaseAddr = USART1_DR_Base;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)RxBufferUSART1;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = RX_BUFFER_SIZE1;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(USART1_Rx_DMA_Channel, &DMA_InitStructure);

    /* Enable USARTy DMA Rx and TX request */
    USART_DMACmd(USART1, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);

    /* Enable USARTy TX DMA1 Channel */
    //DMA_Cmd(USART1_Tx_DMA_Channel, ENABLE);
    /* Enable USARTy RX DMA1 Channel */
    DMA_Cmd(USART1_Rx_DMA_Channel, ENABLE);
}

uint8_t USART1_Read(void)
{
    static uint16_t index_read;

    uint8_t DataRx = RxBufferUSART1[index_read];

    index_read++;
    rx_counter1--;
    if(rx_counter1 == 0) {
        index_read = 0;
        DMA_Cmd(USART1_Rx_DMA_Channel, DISABLE);
        DMA_SetCurrDataCounter(USART1_Rx_DMA_Channel, RX_BUFFER_SIZE1);
        DMA_Cmd(USART1_Rx_DMA_Channel, ENABLE);
    }

    return DataRx;
}

void USART1_Write(char DataTx)
{
    DMA_Cmd(USART1_Tx_DMA_Channel, DISABLE);
    TxBufferUSART1[0] = DataTx;
    DMA_SetCurrDataCounter(USART1_Tx_DMA_Channel, 1);
    DMA_Cmd(USART1_Tx_DMA_Channel, ENABLE);
    while(!(USART1->SR & USART_SR_TXE))
        ;
}

void USART1_Send_STR(unsigned char* pData_Usart)
{
    uint16_t i = 0;
    DMA_Cmd(USART1_Tx_DMA_Channel, DISABLE);
    while(pData_Usart[i] != 0) {
        USART1_Write(pData_Usart[i]);
        i++;
    }

    // DMA_SetCurrDataCounter(USART1_Tx_DMA_Channel, i);
    // DMA_Cmd(USART1_Tx_DMA_Channel, ENABLE);
}

uint16_t USART1_Rx_Len(void)
{
    rx_counter1 = (RX_BUFFER_SIZE1 - DMA_GetCurrDataCounter(USART1_Rx_DMA_Channel));
    return rx_counter1;
}

void USART1_IRQHandler(void)
{
    if(USART1->SR & USART_SR_RXNE) {
        if((USART1->SR & (USART_SR_NE | USART_SR_FE | USART_SR_PE | USART_SR_ORE)) == 0)    //If Error No
        {
        }

        USART1->SR &= ~USART_SR_RXNE;
    }

    if(USART1->SR & USART_SR_TXE) {
        USART1->SR &= ~USART_SR_TXE;
    }
}
