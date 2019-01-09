
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
    initStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
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
    Nrf* txSingle = new Nrf(VPortSpi, GPIOB, GPIO_Pin_0, GPIOB, GPIO_Pin_2);
    if((txSingle == nullptr) || (txSingle->CreateClass() == false)) {
        cout << "Class Nrf was not created!\r";
        while(true);
    }
    
    // Check radio
    if(!(txSingle->Check())) {
        cout << "nRF fail!\r";
        while(true);
    }
    
    // Init radio
    txSingle->Init();
    
    // Set RF channel
    txSingle->SetRFChannel(40);
    
    // Set data rate
    txSingle->SetDataRate(Nrf::DR_250kbps);
    
    // Set CRC scheme
    txSingle->SetCrcScheme(Nrf::CRC_2byte);
    
    // Set address width, its common for all pipes (RX and TX)
    txSingle->SetAddrWidth(sizeof(nRF_ADDR)/sizeof(nRF_ADDR[0]));
    
    // Configure TX PIPE
    txSingle->SetAddr(Nrf::PIPETX, nRF_ADDR);  // program TX address
    txSingle->SetAddr(Nrf::PIPE0, nRF_ADDR);   // program address for pipe#0, must be same as TX (for Auto-ACK)
    
    // Set TX power (maximum)
    txSingle->SetTxPower(Nrf::RF24_PA_MAX);

    // Configure auto retransmit: 10 retransmissions with pause of 2500s in between
    txSingle->SetAutoRetr(Nrf::ARD_2500us, 10);
    
    // Enable Auto-ACK for pipe#0 (for ACK packets)
    txSingle->EnableAA(Nrf::PIPE0);
    
    // Set operational mode (PTX == transmitter)
    txSingle->SetOperationalMode(Nrf::MODE_TX);
    
    // Clear any pending IRQ flags
    txSingle->ClearIRQFlags();
    
    // Wake the transceiver
    txSingle->SetPowerMode(Nrf::PWR_UP);
    Board::DelayMS(5);
    
    
    /* Creating an external interrupt */
    Exti* Interrupt = new Exti(GPIOA, GPIO_Pin_15, NrfTask);
    Interrupt->SetTypeTrigger(EXTI_Trigger_Falling);
    Interrupt->SetPriority(0, 0);
    Interrupt->Enable();
    
    // Buffer to store a payload of maximum width
    uint8_t buffer[32];
    memset(buffer, 0, sizeof(buffer));
    const uint8_t length = 10;
    uint32_t j = 0;
    
    uint32_t packetsLost = 0;
    uint32_t otxPlosCnt = 0;
    volatile uint32_t otxArcCnt = 0;
    
    /* General loop */
    while(true)
    {
        // Prepare data packet
    	for(size_t i = 0; i < length; i++) {
    		buffer[i] = j++;
    		if(j > 0x000000FF) {
                j = 0;
            }
    	}
        
        // Transmit a packet
        const Nrf::TXResult_t res = txSingle->TransmitPacket(buffer, length);
        uint8_t otx = txSingle->GetRetransmitCounters();
        otxPlosCnt = (otx & Nrf::MASK_PLOS_CNT) >> 4; // packets lost counter
        otxArcCnt  = (otx & Nrf::MASK_ARC_CNT); // auto retransmissions counter
        
        if(res == Nrf::TX_MAXRT) {
            packetsLost += otxPlosCnt;
        }
        
        Board::DelayMS(1000);
        IWDG_ReloadCounter();
    }
}


static void NrfTask()
{
    __NOP;
}



/* Private function prototypes -----------------------------------------------*/
#ifdef __GNUC__
    /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
       set to 'Yes') calls __io_putchar() */
    #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
    #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */


/**
 * Переопределение функции putchar
 * Для работы должна быть включена полная поддержка всех библиотечных функций 
*/
PUTCHAR_PROTOTYPE
{
    VPortUart->Transmit((uint8_t *)&ch, sizeof(uint8_t)); 
    return ch;
}


#ifdef USE_FULL_ASSERT

/**
 * @brief  Reports the name of the source file and the source line number
 *   where the assert_param error has occurred.
 * @param file: pointer to the source file name
 * @param line: assert_param error line source number
 * @retval : None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line
       number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line)
     */

    /* Infinite loop */
    while(1)
    {
    }
}
#endif