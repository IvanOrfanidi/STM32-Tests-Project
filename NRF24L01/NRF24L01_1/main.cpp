
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


Nrf* txSingle = nullptr;
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
    
    /* Initialisation Watchdog Timer */
    Board::InitIWDG();
    
    /* Create and initialisation class UART for debug */
    Uart Debug(USART2, 115200, 256, 0);
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
    txSingle = new Nrf(VPortSpi, GPIOB, GPIO_Pin_0, GPIOB, GPIO_Pin_2);
    if((txSingle == nullptr) || (txSingle->CreateClass() == false)) {
        cout << "Class Nrf was not created!\r";
        while(true);
    }
    
    // Check radio
    if(!(txSingle->Check())) {
        cout << "nRF fail!\r";
        while(true);
    }
        
    // Init radio for listening
    Nrf::Settings_t  settingsNrf;
    settingsNrf.OperationalMode = Nrf::MODE_RX;
    settingsNrf.Channel = 40;                       // RF channel 40
    settingsNrf.Pipe = Nrf::PIPE1;                  // Work pipe
    settingsNrf.DataRate = Nrf::DR_250kbps;         // data rate
    settingsNrf.RfPower = Nrf::RF24_PA_MAX;         // TX power (maximum)
    settingsNrf.CrcScheme = Nrf::CRC_off;           // CRC scheme
    settingsNrf.StateAutoAck = Nrf::AA_OFF;         // Auto-ACK
    settingsNrf.AutoRetransmitDelay = Nrf::ARD_2500us;  // pause of 2500s in between
    settingsNrf.CountAutoRetransmits = 10;          // Count retransmissions
    settingsNrf.AddrWidth = sizeof(nRF_ADDR)/sizeof(nRF_ADDR[0]);        // Address width, its common for all pipes (RX and TX)
    memcpy(settingsNrf.Addr, nRF_ADDR, sizeof(nRF_ADDR)/sizeof(nRF_ADDR[0]));
    txSingle->Init(settingsNrf);
    
    /* Test Channel */
    txSingle->SetRFChannel(40);     // 55 Channel is busy
    txSingle->Enable();
    Board::DelayMS(5);
    txSingle->RxOn();
    for(uint16_t i = 0; i < 500; i++) {
        if(txSingle->GetReceivedPowerDetector()) {
            cout << "Channel is busy!\r";
            while(true) {
                IWDG_ReloadCounter();
            }
        }
        IWDG_ReloadCounter();
        Board::DelayMS(1);
    }
    txSingle->RxOff();
    txSingle->Disable();
    Board::DelayMS(5);
    
    
    IWDG_ReloadCounter();
    
    // Init radio for transmitter
    settingsNrf.OperationalMode = Nrf::MODE_TX;
    settingsNrf.Channel = 40;                       // RF channel 40
    settingsNrf.Pipe = Nrf::PIPE0;                  // Work pipe
    settingsNrf.DataRate = Nrf::DR_250kbps;         // data rate
    settingsNrf.RfPower = Nrf::RF24_PA_MAX;         // TX power (maximum)
    settingsNrf.CrcScheme = Nrf::CRC_2byte;         // CRC scheme
    settingsNrf.StateAutoAck = Nrf::AA_ON;          // Auto-ACK
    settingsNrf.AutoRetransmitDelay = Nrf::ARD_2500us;  // pause of 2500s in between
    settingsNrf.CountAutoRetransmits = 10;          // Count retransmissions
    settingsNrf.AddrWidth = sizeof(nRF_ADDR)/sizeof(nRF_ADDR[0]);        // Address width, its common for all pipes (RX and TX)
    memcpy(settingsNrf.Addr, nRF_ADDR, sizeof(nRF_ADDR)/sizeof(nRF_ADDR[0]));
    txSingle->Init(settingsNrf);  
    
    // Wake the transceiver
    txSingle->Enable();
    Board::DelayMS(5);
    
    
    /* Creating an external interrupt */
    Exti* Interrupt = new Exti(GPIOA, GPIO_Pin_15, NrfTask);
    Interrupt->SetTypeTrigger(EXTI_Trigger_Falling);
    Interrupt->SetPriority(0, 0);
    Interrupt->Enable();
    
    // Buffer to store a payload of maximum width
    uint8_t buffer[32];
    memset(buffer, 0, sizeof(buffer));
    const uint8_t length = 32;
    uint32_t j = 0;
    
    uint32_t packetsLost = 0;
    uint32_t otxPlosCnt = 0;
    volatile uint32_t otxArcCnt = 0;
    IWDG_ReloadCounter();
    
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
        
        if(res != Nrf::TX_SUCCESS) {
            packetsLost += otxPlosCnt;
        }
        
        Board::DelayMS(100);
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