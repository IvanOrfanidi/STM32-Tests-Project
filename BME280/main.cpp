
/* Standart lib */
#include <assert.h>
#include <ctype.h>
#include <intrinsics.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Standart lib */
#include <iostream>
using namespace std;

#include "board.hpp"
#include "stm32f10x_conf.h"
#include "bme280.hpp"

float temp = 0;
uint16_t pressure = 0;
float humidity = 0;


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
    
    /* Initialisation Watchdog timer */
    Board::InitIWDG();

    /* Create sensor type BME (pressure and temperature) */
    Bme* bme = new Bme(I2C1, I2C_SPEED);
    if((bme == nullptr) || (bme->CreateClass() == false)) {
        while(true);
    }

    /* Read calibration parameters */
    if(!(bme->ReadCalibration())) {
        while(true);
    }

    Board::DelayMS(500);

    /* General loop */
    while(true)
    {
        bme->GetTemperature(&temp);
        bme->GetPressureHg(&pressure);
        bme->GetHumidity(&humidity);

        Board::LedOn();
        Board::DelayMS(500);
        IWDG_ReloadCounter();
        Board::LedOff();
        Board::DelayMS(500);
        IWDG_ReloadCounter();
    }
}


#ifdef USE_FULL_ASSERT

/**
* @brief  Reports the name of the source file and the source line number
*   where the assert_param error has occurred.
* @param file: pointer to the source file name
* @param line: assert_param error line source number
* @retval : None
*/
void assert_failed(uint8_t* file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line
    number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line)
    */

    /* Infinite loop */
        while (1)
        {
        }
}
#endif
