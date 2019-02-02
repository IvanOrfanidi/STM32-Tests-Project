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

/*
 * @brief Class Exti
 */
class Exti {
  public:
    enum Exti_t {
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

    /// Ñonstructor
    Exti(GPIO_TypeDef*, uint16_t, void (*)());

    /// Set Type Trigger
    void SetTypeTrigger(EXTITrigger_TypeDef);

    /// Set Priority Interrupt
    void SetPriority(uint8_t, uint8_t);

    void Enable() const;    /// Enable Interrupt

    void Disable() const;    /// Disable Interrupt

    /// Public Interrupt Handler
    static void ExtiHandler(size_t);

  private:
    void Handler();    /// Private Interrupt Handler

    static Exti* Extis[EXTI_MAX_COUNT];    ///< Main array pointers of classes Uarts

    void (*Callback)();    ///< Callback handler function

    GPIO_TypeDef* Port;    ///< Port interrupt

    uint16_t Pin;    ///< Pin interrupt

    EXTITrigger_TypeDef Trigger;    ///< Type trigger interrupt

    uint32_t Line;    ///< Line interrupt

    IRQn_Type Irq;    ///< Work interrupt

    uint8_t PreemptionPriority;    ///< Preemption priority interrupt

    uint8_t SubPriority;    ///< Sub priority interrupt
};

extern "C" {
/* This Ñ-function handles EXTIs global interrupt request */
void EXTI0_IRQHandler(void);
void EXTI1_IRQHandler(void);
void EXTI2_IRQHandler(void);
void EXTI3_IRQHandler(void);
void EXTI4_IRQHandler(void);
void EXTI9_5_IRQHandler(void);
void EXTI15_10_IRQHandler(void);
}

#endif    //__cplusplus

#endif