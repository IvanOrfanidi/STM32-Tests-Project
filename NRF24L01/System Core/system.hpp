/**
 ******************************************************************************
 * @file    system.hpp
 * @author  Ivan Orfanidi
 * @version V1.0.0
 * @date    04-2019
 * @brief   This file system
 ******************************************************************************
 * @attention
 *
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SYSTEM_HPP
#define __SYSTEM_HPP

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include <time.h>

#ifdef __cplusplus

/*
 * @brief Class System
 */
class System {
  public:
    static void SetNvicPriorityGroup(time_t);    ///< Set NVIC Priority Group

    static void InitSysTick(time_t ticks_us = 1000);    ///< Init System Tick

    static time_t ClockUpdate();    ///< Update and geting System Clock Core

    static void SysTickClock(FunctionalState);

    static void DelayMS(time_t);    ///< Delay msec

    static time_t GetSysCount();    ///< Get System Counter

    static void SleepDevice();    ///< Sleep Device

    static void WakeUpPinEnable();    ///< Enable WKUP pin

    static time_t SysCount;

  private:
    static void SysTickEnableIRQ()
    {
        SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
        SysTickState = ENABLE;
    }

    static void SysTickDisableIRQ()
    {
        SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
        SysTickState = DISABLE;
    }

    static FunctionalState SysTickState;
};

extern "C" {
/// Extern interrupt
void SysTick_Handler(void);
}

#endif    //__cplusplus

#endif