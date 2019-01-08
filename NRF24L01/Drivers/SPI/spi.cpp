/**
 ******************************************************************************
 * @file    spi.cpp
 * @author  Ivan Orfanidi
 * @version V1.0.0
 * @date    06/01/2019
 * @brief   This file provides all the SPI firmware method.
 ******************************************************************************
 * @attention
 *
 *
 * <h2><center>&copy; </center></h2>
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "spi.hpp"

/*
 * Main array pointers of classes Spis
 * Use MAX_COUNT_UART max quantity Spis
 */
Spi* Spi::Spis[Spi::MAX_COUNT_SPI];


/**
 * @brief Сonstructor
 * @param [in] spi - pointer to work SPI
 * @param [in] initStruct - interface settings SPI
 */
Spi::Spi(SPI_TypeDef* const spi, SPI_InitTypeDef* initStruct, size_t rx_size)
{
    SPIx = nullptr;
    
    if(spi == SPI1) {
        if((Spis[SPI_1]) || (!(InitDataBuf(rx_size)))) {
            return;
        }
        Spis[SPI_1] = this;
    }
    else if(spi == SPI2) {
        if((Spis[SPI_2]) || (!(InitDataBuf(rx_size)))) {
            return;
        }
        Spis[SPI_2] = this;
    }
    else if(spi == SPI3) {
        if((Spis[SPI_3]) || (!(InitDataBuf(rx_size)))) {
            return;
        }
        Spis[SPI_3] = this;
    }
    else {
        return;
    }
    
    SPIx = spi; // Assign work SPI
    
    /* Config GPIO SPI */
    GPIO_TypeDef* portSpiInit;
    GPIO_InitTypeDef pinSpiInitStructure;
    pinSpiInitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    pinSpiInitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    
    if(SPIx == SPI1) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
        
        pinSpiInitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
        portSpiInit = GPIOA;
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    }
    else if (SPIx == SPI2) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
        
        pinSpiInitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
        portSpiInit = GPIOB;
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    }
    else if (SPIx == SPI3) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
        
        pinSpiInitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
        portSpiInit = GPIOB;
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    }
    
    GPIO_Init(portSpiInit, &pinSpiInitStructure);
    
    if(initStruct == nullptr) {
        InitDefSpi();       // Default config
    }
    else {        
        SPI_Init(SPIx, initStruct);
    }
    
    SPI_Cmd(SPIx, ENABLE);
}


bool Spi::InitDataBuf(size_t rx_size)
{
    /* Create memory */
    pRxBuf = new uint8_t[rx_size];
    if(!(pRxBuf)) {
        return false;
    }
    
    RxBufSize = rx_size;
    RxCount = 0;
    
    return true;
}


/**
 * @brief Destructor
 */
Spi::~Spi()
{    
    GPIO_TypeDef* port;
    GPIO_InitTypeDef pin;
    pin.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    pin.GPIO_Speed = GPIO_Speed_2MHz;
    
    if(SPIx == SPI1) {
        if(Spis[SPI_1] == nullptr) {
            return;
        }
        
        SPI_Cmd(SPIx, DISABLE);
        SPI_I2S_DeInit(SPIx);
        
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, DISABLE);
        pin.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
        port = GPIOA;
    
        Spis[SPI_1] = nullptr;
    }
    else if (SPIx == SPI2) {
        if(Spis[SPI_2] == nullptr) {
            return;
        }
        
        SPI_Cmd(SPIx, DISABLE);
        SPI_I2S_DeInit(SPIx);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, DISABLE);
        
        pin.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
        port = GPIOB;
    
        Spis[SPI_2] = nullptr;
    }
    else if (SPIx == SPI3) {
        if(Spis[SPI_3] == nullptr) {
            return;
        }
        
        SPI_Cmd(SPIx, DISABLE);
        SPI_I2S_DeInit(SPIx);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, DISABLE);
        
        pin.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
        port = GPIOB;
    
        Spis[SPI_3] = nullptr;
    }
    
    GPIO_Init(port, &pin);
    
    if(pRxBuf) {
        delete[] pRxBuf;
    }
}


/**
 * @brief Initialisation default SPI
 */
void Spi::InitDefSpi() const
{
    /* Config Interface SPI */
    SPI_InitTypeDef initDefStruct;
    initDefStruct.SPI_Mode = SPI_Mode_Master;
    initDefStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
    initDefStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    initDefStruct.SPI_CPOL = SPI_CPOL_Low;
    initDefStruct.SPI_CPHA = SPI_CPHA_1Edge;
    initDefStruct.SPI_DataSize = SPI_DataSize_8b;
    initDefStruct.SPI_FirstBit = SPI_FirstBit_MSB;
    initDefStruct.SPI_NSS = SPI_NSS_Soft;
    initDefStruct.SPI_CRCPolynomial = 7;
    
    SPI_Init(SPIx, &initDefStruct);
}


void Spi::Transmit(const uint8_t *ptr, size_t len)
{
    if(SPIx == nullptr) {
        return;
    }
    
    uint8_t byteRx;
    
    while(len--) {
        // Wait while spi will not busy
        while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_BSY) == SET);
        
         // Wait until TX buffer is empty
        while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET);
        
        // Send byte to SPI (TXE cleared)
        SPI_I2S_SendData(SPIx, *ptr++);

        // Wait while receive buffer is empty
        while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET);
        
        // Wait while spi will not busy
        while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_BSY) == SET);

        // Return received byte
        byteRx = SPI_I2S_ReceiveData(SPIx);
        
        pRxBuf[RxCount++] = byteRx;
        
        // Нехватает памяти, добавим еще
        if(RxCount > RxBufSize) {
            uint8_t* tempBuf  = new uint8_t[RxBufSize + RX_ADD_SIZE];
            memcpy(tempBuf, pRxBuf, RxCount);
            
            delete[] pRxBuf;
            
            pRxBuf = new uint8_t[RxBufSize + RX_ADD_SIZE];
            memcpy(pRxBuf, tempBuf, RxCount);
            
            delete[] tempBuf;
        }
    }
}


size_t Spi::Receive(uint8_t *ptr, size_t len)
{
    uint32_t real_len = 0;
    while(len--) {
        *ptr++ = pRxBuf[real_len++];
        if(!(RxCount--)) {
            break;
        }
    }
    return real_len;
}


size_t Spi::GetLen()
{
    return RxCount;
}


void Spi::ClearTransmit()
{
}


void Spi::ClearReceive()
{
    RxCount = 0;
}


/**
 * @brief Wait complete transfer data/
 */
void Spi::WaitingCompleteTransfer()
{
    // Wait while spi will not busy
    while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_BSY) == SET);

     // Wait until TX buffer is empty
    while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET);
}