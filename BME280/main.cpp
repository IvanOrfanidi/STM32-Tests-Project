
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
#include "bme280.hpp"

/// I2C Baudrate
enum IicConfig_t {
    I2C_SPEED = 400000
};

/*
 * @brief General functions MAIN
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

    /* Initialisation Watchdog timer */
    Board::InitIWDG();

    /* Create sensor type BME (pressure, humidity and temperature) */
    Bme* bme = new Bme(I2C1, I2C_SPEED);
    if((bme == nullptr) || (bme->CreateClass() == false)) {
        while(true)
            ;
    }

    /* Read calibration parameters */
    if(!(bme->ReadCalibration())) {
        while(true)
            ;
    }

    float temp = 0;               // temperature
    volatile uint32_t qfe = 0;    // QFE pressure in mmHg(давление измеренное в точке измерения)
    volatile uint32_t qnh = 0;    // QNH pressure in mmHg(давление на уровне моря в точке измерения)
    float hum = 0;                // humidity
    volatile int alt = 0;         // altitude
    volatile float dew = 0;       // точка росы

    Board::DelayMS(500);

    /* General loop */
    while(true) {
        bme->GetTemperature(&temp);

        bme->GetHumidity(&hum);

        uint32_t ppa_qfe;
        bme->GetQfePressure(&ppa_qfe);
        qfe = bme->Pa2mmHg(ppa_qfe);

        uint32_t ppa_qnh;
        bme->GetQnhPressure(&ppa_qnh, 30);
        qnh = bme->Pa2mmHg(ppa_qnh);

        alt = bme->GetAltitude(ppa_qfe, ppa_qnh);

        dew = bme->GetDewpoint(hum, temp);

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
    while(1) {
    }
}
#endif
