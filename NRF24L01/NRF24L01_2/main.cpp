
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
#include "nrf24l01.hpp"


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
    Uart Debug(USART2, 9600, 256, 256, 0, 0);
    VirtualPort *VPortDebug = &Debug;
       
    /* Create radio */
    Nrf* nrf1 = new Nrf(SPI1, GPIOB, GPIO_Pin_0, GPIOB, GPIO_Pin_2);
    if((nrf1 == nullptr) || (nrf1->CreateClass() == false)) {
        while(true);
    }
    
    if(!(nrf1->Check())) {
        while(true);
    }
    
    /* General loop */
    while(true)
    {
        
        IWDG_ReloadCounter();
    }
}
