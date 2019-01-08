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
#include <stdlib.h>
#include <string.h>
#include "stm32f10x_gpio.h"


#ifdef __cplusplus

class Exti
{
    public:
    
        enum Exti_t
        {
            EXTI_0,
            EXTI_1,
            EXTI_2,
            EXTI_3,
            EXTI_4,
            EXTI_5,
            EXTI_6,
            EXTI_7,
            EXTI_8,
            EXTI_9,
            EXTI_10,
            EXTI_11,
            EXTI_12,
            EXTI_13,
            EXTI_14,
            EXTI_15,
            
            EXTI_MAX_COUNT
        };
    
        Exti(GPIO_TypeDef*, uint16_t, void (*)());
        
        void SetTypeTrigger(EXTITrigger_TypeDef);

        void SetPriority(uint8_t, uint8_t);
        
        void Enable() const;
        
        void Disable() const;
        
        static void ExtiHandler(size_t);
        
    private:
        
        void Run();
    
        static Exti* Extis[EXTI_MAX_COUNT];
        
        void (*Callback)();
    
        GPIO_TypeDef* Port;
        
        uint16_t Pin;
        
        EXTITrigger_TypeDef Trigger;
        
        uint32_t Line;
        
        IRQn_Type Irq;
        
        uint8_t PreemptionPriority;
        
        uint8_t SubPriority;
};

extern "C" {
    void EXTI0_IRQHandler(void);
    void EXTI1_IRQHandler(void);
    void EXTI2_IRQHandler(void);
    void EXTI3_IRQHandler(void);
    void EXTI4_IRQHandler(void);
    void EXTI9_5_IRQHandler(void);
    void EXTI15_10_IRQHandler(void);
}

#endif
#endif