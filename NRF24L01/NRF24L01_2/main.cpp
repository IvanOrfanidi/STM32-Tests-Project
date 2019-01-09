
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
#include "exti.hpp"
#include "nrf24l01.hpp"


const uint8_t nRF_ADDR[] = { 'E', 'S', 'B' };


static void NrfTask();

volatile Nrf::RXResult_t pipe;

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
    
    Spi Spi2(SPI2, &initStruct);
    VirtualPort* const VPortSpi = &Spi2;
       
    /* Create radio */
    Nrf* rxSingle = new Nrf(VPortSpi, GPIOB, GPIO_Pin_0, GPIOB, GPIO_Pin_1);
    if((rxSingle == nullptr) || (rxSingle->CreateClass() == false)) {
        while(true);
    }
    
    if(!(rxSingle->Check())) {
        while(true);
    }
    
    // Init radio
    rxSingle->Init();
    
    // Set RF channel
    rxSingle->SetRFChannel(40);
    
    // Set data rate
    rxSingle->SetDataRate(Nrf::DR_250kbps);
    
    // Set CRC scheme
    rxSingle->SetCrcScheme(Nrf::CRC_2byte);
    
    // Set address width, its common for all pipes (RX and TX)
    rxSingle->SetAddrWidth(sizeof(nRF_ADDR)/sizeof(nRF_ADDR[0]));
    
    // Configure RX PIPE
    rxSingle->SetAddr(Nrf::PIPE1, nRF_ADDR); // program address for pipe
    rxSingle->SetRxPipe(Nrf::PIPE1, Nrf::AA_ON, 10); // Auto-ACK: enabled, payload length: 10 bytes
    
    // Set TX power (maximum)
    rxSingle->SetTxPower(Nrf::RF24_PA_MAX);
    
    // Set operational mode (PRX == receiver)
    rxSingle->SetOperationalMode(Nrf::MODE_RX);
    
    // Clear any pending IRQ flags
    rxSingle->ClearIRQFlags();
    
    // Wake the transceiver
    rxSingle->SetPowerMode(Nrf::PWR_UP);
    Board::DelayMS(5);
    
    // Put the transceiver to the RX mode
    rxSingle->RxOn();
    
    /* Creating an external interrupt */
    Exti* Interrupt = new Exti(GPIOA, GPIO_Pin_15, NrfTask);
    Interrupt->SetTypeTrigger(EXTI_Trigger_Falling);
    Interrupt->SetPriority(0, 0);
    Interrupt->Enable();
    
    // Buffer to store a payload of maximum width
    uint8_t buffer[32];
    memset(buffer, 0, sizeof(buffer));
    uint8_t length = 10;
    
    
    /* General loop */
    while(true)
    {
        if(rxSingle->GetStatus_RXFIFO() != Nrf::STATUS_RXFIFO_EMPTY) {
    		// Get a payload from the transceiver
            memset(buffer, 0, sizeof(buffer));
    		pipe = rxSingle->ReadPayload(buffer, &length);
            IWDG_ReloadCounter();
        }
        IWDG_ReloadCounter();
    }
}


static void NrfTask()
{
    __NOP;
}

