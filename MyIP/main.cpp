
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
#include "spi.hpp"
#include "enc28j60.hpp"
#include "exti.hpp"


static void LanTask();


Enc* Lan = nullptr;


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
    
    /* Create and initialisation class SPI for nRF24L01 */
    SPI_InitTypeDef initStruct;
    initStruct.SPI_Mode = SPI_Mode_Master;
    initStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
    initStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    initStruct.SPI_CPOL = SPI_CPOL_Low;
    initStruct.SPI_CPHA = SPI_CPHA_1Edge;
    initStruct.SPI_DataSize = SPI_DataSize_8b;
    initStruct.SPI_FirstBit = SPI_FirstBit_MSB;
    initStruct.SPI_NSS = SPI_NSS_Soft;
    initStruct.SPI_CRCPolynomial = 7;
    
    Spi Spi2(SPI2, &initStruct);
    VirtualPort* const VPortSpi = &Spi2;
    
    Lan = new Enc(VPortSpi, GPIOA, GPIO_Pin_6, GPIOA, GPIO_Pin_7);
    
    uint8_t myMac[] = {0x00,0x2F,0x68,0x12,0xAC,0x30};
    uint8_t myIp[] = {192, 168, 0, 110};
    const uint16_t tcpPort = 80;
    
    Lan->Init(myMac, myIp, tcpPort);
        
    Exti* Interrupt = new Exti(GPIOC, GPIO_Pin_6, LanTask);
    Interrupt->SetTypeTrigger(EXTI_Trigger_Falling);
    Interrupt->SetPriority(0, 0);
    Interrupt->Enable();
    
    while(true)
    {
        
        IWDG_ReloadCounter();
    }
}


static void LanTask()
{
    Lan->Task();
}