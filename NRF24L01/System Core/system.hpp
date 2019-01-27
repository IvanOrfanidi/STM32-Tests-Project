/**
 ******************************************************************************
 * @file    system.hpp
 * @author  Ivan Orfanidi
 * @version V1.0.0
 * @date    27-01-2019
 * @brief   This file system
 ******************************************************************************
 * @attention
 *
 *
 * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SYSTEM_HPP
#define __SYSTEM_HPP

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

#ifdef __cplusplus


/*
 * @brief Class System
 */
class System
{
    public:
    
        static void SetNvicPriorityGroup(uint32_t);   ///< Set NVIC Priority Group

        static void InitSysTick(uint32_t ticks_us = 1000);   ///< Init System Tick

        static uint32_t ClockUpdate();   ///< Update and geting System Clock Core

        static void DelayMS(uint32_t);   ///< Delay msec

        static uint32_t GetSysCount();   ///< Get System Counter
        
        static void SleepDevice();       ///< Sleep Device

        static void WakeUpPinEnable();   ///< Enable WKUP pin

        static uint32_t SysCount;
};


extern "C" {
    /// Extern interrupt
    void SysTick_Handler(void);
}

#endif//__cplusplus

#endif