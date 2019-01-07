
/* Standart lib */
#include <assert.h>
#include <ctype.h>
#include <intrinsics.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
using namespace std;


/* User lib */
#include "board.hpp"
#include "stm32f10x_conf.h"
#include "uart.hpp"
#include "spi.hpp"
#include "nrf24l01.hpp"


VirtualPort* VPortUart = nullptr;


/**
 * @brief General functions main
 */
int main()
{   
    /* Set NVIC Priority Group (4 bits for preemption priority, 0 bits for
    * subpriority) */
    Board::SetNvicPriorityGroup(NVIC_PriorityGroup_4);

    /* Update System clock Core */
    Board::ClockUpdate();

    /* Init System Timer */
    Board::InitSysTick(1000);

    /* Initialisation Backup */
    Board::InitBKP();

    /* Initialisation Led */
    Board::InitLed();
    Board::LedOn();
    
    /* Create and initialisation class UART for debug */
    Uart Debug(USART2, 9600, 256, 0, 0, 0);
    VPortUart = &Debug;
    
    /* Create and initialisation class SPI for nRF24L01 */
    SPI_InitTypeDef initStruct;
    initStruct.SPI_Mode = SPI_Mode_Master;
    initStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
    initStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    initStruct.SPI_CPOL = SPI_CPOL_Low;
    initStruct.SPI_CPHA = SPI_CPHA_1Edge;
    initStruct.SPI_DataSize = SPI_DataSize_8b;
    initStruct.SPI_FirstBit = SPI_FirstBit_MSB;
    initStruct.SPI_NSS = SPI_NSS_Soft;
    initStruct.SPI_CRCPolynomial = 7;
    
    Spi Spi1(SPI1, &initStruct);
    VirtualPort* const VPortSpi = &Spi1;
       
    /* Create radio */
    Nrf* nrf1 = new Nrf(VPortSpi, GPIOB, GPIO_Pin_0, GPIOB, GPIO_Pin_2);
    if((nrf1 == nullptr) || (nrf1->CreateClass() == false)) {
        while(true);
    }
    
    if(!(nrf1->Check())) {
        while(true);
    }
    
    /* General loop */
    while(true)
    {
        cout << "Hello\r" << endl;
        Board::DelayMS(1000);
        IWDG_ReloadCounter();
    }
}
