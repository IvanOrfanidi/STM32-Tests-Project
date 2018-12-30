/**
 ******************************************************************************
 * @file    boadr.hpp
 * @author  Ivan Orfanidi
 * @version V1.0.0
 * @date    28-March-2018
 * @brief   This file contains all the methods prototypes for the RTC
 *          firmware library.
 ******************************************************************************
 * @attention
 *
 *
 * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BOARD_HPP
#define __BOARD_HPP

#include <stdint.h>

#define GpioHigh(a, b) a->BSRR = b
#define GpioLow(a, b) a->BRR = b
#define GpioToggle(a, b) a->ODR ^= b

/// LIGHT LED
#define LED_PIN GPIO_Pin_12
#define PORT_LED GPIOB
#define PORT_LED_CLK RCC_APB2Periph_GPIOB

#ifdef __cplusplus


enum IicConfig_t
{
  I2C_SPEED = 400000
};


class Board
{
    public:
        static void SetNvicPriorityGroup(uint32_t);   ///< Set NVIC Priority Group
        static void InitSysTick(uint32_t ticks_us = 1000);   ///< Init System Tick
        static uint32_t ClockUpdate();   ///< Update and geting System Clock Core
        static void InitLed();   ///< Initialisation Led
        static void InitBKP();   ///< Initialisation Backup
        static void InitIWDG();   ///< Initialisation Watchdog timer
        static void SleepDevice();   ///< Sleep Device
        static void WakeUpPinEnable();   ///< Enable WKUP pin
        static void LedOn();   ///< Enable Led
        static void LedOff();   ///< Disable Led
        static void DelayMS(uint32_t);   ///< Delay msec
        static uint32_t GetSysCount();   ///< Get System Counter

        static uint32_t SysCount;

    protected:
        Board();   ///< Constructor

    private:
        virtual ~Board();
};

extern "C" {
void SysTick_Handler(void);
}
#endif

#endif