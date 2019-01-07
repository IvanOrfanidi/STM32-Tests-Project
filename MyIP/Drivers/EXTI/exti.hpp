/**
 ******************************************************************************
 * @file    exti.h
 * @author  Ivan Orfanidi
 * @version V1.0.0
 * @date    28-March-2018
 * @brief   This file contains all the methods prototypes for the USART
 *          firmware library.
 ******************************************************************************
 * @attention
 *
 *
 * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __EXTI_HPP
#define __EXTI_HPP

#include <stdint.h>

#ifdef __cplusplus

class Exti0
{
  public:
  
    static void InitNvic(uint8_t, uint8_t);

    static void SetCpm(uint32_t);

    static uint32_t GetCpm();

    static uint32_t Cpm;  
};

extern "C" {
    void EXTI0_IRQHandler(void);
}

#endif
#endif