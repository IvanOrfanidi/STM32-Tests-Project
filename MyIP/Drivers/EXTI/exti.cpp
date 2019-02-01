/**
 ******************************************************************************
 * @file    exit.cpp
 * @author  Ivan Orfanidi
 * @version V1.0.0
 * @date    28-March-2018
 * @brief   This file provides all the USART firmware method.
 ******************************************************************************
 * @attention
 *
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "exti.hpp"

/*
 * Main array pointers of classes Extis
 * Use EXTI_MAX_COUNT max quantity Extis
 */
Exti* Exti::Extis[EXTI_MAX_COUNT];

/**
 * @brief Empty function
*/
static void Nop()
{
    __NOP;
}

/**
 * @brief Ñonstructor
 * @param [in] preemption_priority - preemption priority
 * @param [in] sub_priority - sub priority
 * @param [in] function - the interrupt processing function
 */
Exti::Exti(GPIO_TypeDef* port, uint16_t pin, void (*function)())
{
    PreemptionPriority = 0;
    SubPriority = 0;
    Trigger = EXTI_Trigger_Rising_Falling;
    Port = port;
    Pin = pin;
    if(function) {
        Callback = function;
    }
    else {
        Callback = &Nop;
    }

    /* Enable GPIO clock */
    uint8_t portSource;
    if(Port == GPIOA) {
        portSource = GPIO_PortSourceGPIOA;
    }
    else if(Port == GPIOB) {
        portSource = GPIO_PortSourceGPIOB;
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    }
    else if(Port == GPIOC) {
        portSource = GPIO_PortSourceGPIOC;
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    }
    else if(Port == GPIOD) {
        portSource = GPIO_PortSourceGPIOD;
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    }
    else if(Port == GPIOE) {
        portSource = GPIO_PortSourceGPIOE;
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
    }
    else if(Port == GPIOF) {
        portSource = GPIO_PortSourceGPIOF;
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
    }
    else if(Port == GPIOG) {
        portSource = GPIO_PortSourceGPIOG;
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);
    }

    uint8_t pinSource;
    size_t freeClass;
    switch(Pin) {
        case GPIO_Pin_0:
            pinSource = GPIO_PinSource0;
            Irq = EXTI0_IRQn;
            Line = EXTI_Line0;
            freeClass = EXTI_0;
            break;
        case GPIO_Pin_1:
            pinSource = GPIO_PinSource1;
            Irq = EXTI1_IRQn;
            Line = EXTI_Line1;
            freeClass = EXTI_1;
            break;
        case GPIO_Pin_2:
            pinSource = GPIO_PinSource2;
            Irq = EXTI2_IRQn;
            Line = EXTI_Line2;
            freeClass = EXTI_2;
            break;
        case GPIO_Pin_3:
            pinSource = GPIO_PinSource3;
            Irq = EXTI3_IRQn;
            Line = EXTI_Line3;
            freeClass = EXTI_3;
            break;
        case GPIO_Pin_4:
            pinSource = GPIO_PinSource4;
            Irq = EXTI4_IRQn;
            Line = EXTI_Line4;
            freeClass = EXTI_4;
            break;
        case GPIO_Pin_5:
            pinSource = GPIO_PinSource5;
            Irq = EXTI9_5_IRQn;
            Line = EXTI_Line5;
            freeClass = EXTI_5;
            break;
        case GPIO_Pin_6:
            pinSource = GPIO_PinSource6;
            Irq = EXTI9_5_IRQn;
            Line = EXTI_Line6;
            freeClass = EXTI_6;
            break;
        case GPIO_Pin_7:
            pinSource = GPIO_PinSource7;
            Irq = EXTI9_5_IRQn;
            Line = EXTI_Line7;
            freeClass = EXTI_7;
            break;
        case GPIO_Pin_8:
            pinSource = GPIO_PinSource8;
            Irq = EXTI9_5_IRQn;
            Line = EXTI_Line8;
            freeClass = EXTI_8;
            break;
        case GPIO_Pin_9:
            pinSource = GPIO_PinSource9;
            Irq = EXTI9_5_IRQn;
            Line = EXTI_Line9;
            freeClass = EXTI_9;
            break;
        case GPIO_Pin_10:
            pinSource = GPIO_PinSource10;
            Irq = EXTI15_10_IRQn;
            Line = EXTI_Line10;
            freeClass = EXTI_10;
            break;
        case GPIO_Pin_11:
            pinSource = GPIO_PinSource11;
            Irq = EXTI15_10_IRQn;
            Line = EXTI_Line11;
            freeClass = EXTI_11;
            break;
        case GPIO_Pin_12:
            pinSource = GPIO_PinSource12;
            Irq = EXTI15_10_IRQn;
            Line = EXTI_Line12;
            freeClass = EXTI_12;
            break;
        case GPIO_Pin_13:
            pinSource = GPIO_PinSource13;
            Irq = EXTI15_10_IRQn;
            Line = EXTI_Line13;
            freeClass = EXTI_13;
            break;
        case GPIO_Pin_14:
            pinSource = GPIO_PinSource14;
            Irq = EXTI15_10_IRQn;
            Line = EXTI_Line14;
            freeClass = EXTI_14;
            break;

        default:
            pinSource = GPIO_PinSource15;
            Irq = EXTI15_10_IRQn;
            Line = EXTI_Line15;
            freeClass = EXTI_15;
    };

    /* Configure pin as input floating */
    GPIO_InitTypeDef gpioInitStructure;
    gpioInitStructure.GPIO_Pin = Pin;
    gpioInitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(Port, &gpioInitStructure);

    /* Enable AFIO clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    /* Tell system that you will use Port Source for EXTI Line */
    GPIO_EXTILineConfig(portSource, pinSource);

    /* Configure EXTI line */
    EXTI_InitTypeDef extiInitStructure;
    extiInitStructure.EXTI_Line = Line;
    extiInitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    extiInitStructure.EXTI_Trigger = Trigger;
    extiInitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&extiInitStructure);

    Extis[freeClass] = this;
    Disable();
}

/**
 * @bref Set Type Trigger
 * @param [in] trigger - type trigger:
 *             EXTI_Trigger_Rising
 *             EXTI_Trigger_Falling 
 *             EXTI_Trigger_Rising_Falling 
 */
void Exti::SetTypeTrigger(EXTITrigger_TypeDef trigger)
{
    Trigger = trigger;
    EXTI_InitTypeDef extiInitStructure;
    extiInitStructure.EXTI_Line = Line;
    extiInitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    extiInitStructure.EXTI_Trigger = Trigger;
    extiInitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&extiInitStructure);
}

/**
 * @bref Set Priority Interrupt
 * @param [in] preemption_priority - preemption priority, default 0
 * @param [in] sub_priority - sub priority, default 0
 */
void Exti::SetPriority(uint8_t preemption_priority = 0, uint8_t sub_priority = 0)
{
    switch(Pin) {
        case GPIO_Pin_0:
            Irq = EXTI0_IRQn;
            break;
        case GPIO_Pin_1:
            Irq = EXTI1_IRQn;
            break;
        case GPIO_Pin_2:
            Irq = EXTI2_IRQn;
            break;
        case GPIO_Pin_3:
            Irq = EXTI3_IRQn;
            break;
        case GPIO_Pin_4:
            Irq = EXTI4_IRQn;
            break;
        case GPIO_Pin_5:
        case GPIO_Pin_6:
        case GPIO_Pin_7:
        case GPIO_Pin_8:
        case GPIO_Pin_9:
            Irq = EXTI9_5_IRQn;
            break;

        default:
            Irq = EXTI15_10_IRQn;
    };

    PreemptionPriority = preemption_priority;
    SubPriority = sub_priority;

    /* Enable and set EXTI Interrupt to the lowest priority */
    NVIC_InitTypeDef nvicInitStructure;
    nvicInitStructure.NVIC_IRQChannel = Irq;
    nvicInitStructure.NVIC_IRQChannelPreemptionPriority = PreemptionPriority;
    nvicInitStructure.NVIC_IRQChannelSubPriority = SubPriority;
    nvicInitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvicInitStructure);
}

/**
 * @bref Enable Interrupt
 */
void Exti::Enable() const
{
    NVIC_EnableIRQ(Irq);
}

/**
 * @bref Disable Interrupt
 */
void Exti::Disable() const
{
    NVIC_DisableIRQ(Irq);
}

/**
 * @bref Public Interrupt Handler
 * @param [in] line - line interrupt
 */
void Exti::ExtiHandler(size_t line)
{
    if(Extis[line] != nullptr) {
        Extis[line]->Handler();
    }
}

/**
 * @bref Private Interrupt Handler
 */
void Exti::Handler()
{
    (*Callback)();
}

/* This Ñ-function handles EXTIs global interrupt request */
void EXTI0_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line0) != RESET) {
        Exti::ExtiHandler(Exti::EXTI_0);

        // Clear the  EXTI line pending bit
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

void EXTI1_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line1) != RESET) {
        Exti::ExtiHandler(Exti::EXTI_1);

        // Clear the  EXTI line pending bit
        EXTI_ClearITPendingBit(EXTI_Line1);
    }
}

void EXTI2_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line2) != RESET) {
        Exti::ExtiHandler(Exti::EXTI_2);

        // Clear the  EXTI line pending bit
        EXTI_ClearITPendingBit(EXTI_Line2);
    }
}

void EXTI3_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line3) != RESET) {
        Exti::ExtiHandler(Exti::EXTI_3);

        // Clear the  EXTI line pending bit
        EXTI_ClearITPendingBit(EXTI_Line3);
    }
}

void EXTI4_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line4) != RESET) {
        Exti::ExtiHandler(Exti::EXTI_4);

        // Clear the  EXTI line pending bit
        EXTI_ClearITPendingBit(EXTI_Line4);
    }
}

void EXTI9_5_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line5) != RESET) {
        Exti::ExtiHandler(Exti::EXTI_5);

        // Clear the  EXTI line pending bit
        EXTI_ClearITPendingBit(EXTI_Line5);
    }

    if(EXTI_GetITStatus(EXTI_Line6) != RESET) {
        Exti::ExtiHandler(Exti::EXTI_6);

        // Clear the  EXTI line pending bit
        EXTI_ClearITPendingBit(EXTI_Line6);
    }

    if(EXTI_GetITStatus(EXTI_Line7) != RESET) {
        Exti::ExtiHandler(Exti::EXTI_7);

        // Clear the  EXTI line pending bit
        EXTI_ClearITPendingBit(EXTI_Line7);
    }

    if(EXTI_GetITStatus(EXTI_Line8) != RESET) {
        Exti::ExtiHandler(Exti::EXTI_8);

        // Clear the  EXTI line pending bit
        EXTI_ClearITPendingBit(EXTI_Line8);
    }

    if(EXTI_GetITStatus(EXTI_Line9) != RESET) {
        Exti::ExtiHandler(Exti::EXTI_9);

        // Clear the  EXTI line pending bit
        EXTI_ClearITPendingBit(EXTI_Line9);
    }
}

void EXTI15_10_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line10) != RESET) {
        Exti::ExtiHandler(Exti::EXTI_10);

        // Clear the  EXTI line pending bit
        EXTI_ClearITPendingBit(EXTI_Line10);
    }

    if(EXTI_GetITStatus(EXTI_Line11) != RESET) {
        Exti::ExtiHandler(Exti::EXTI_11);

        // Clear the  EXTI line pending bit
        EXTI_ClearITPendingBit(EXTI_Line11);
    }

    if(EXTI_GetITStatus(EXTI_Line12) != RESET) {
        Exti::ExtiHandler(Exti::EXTI_12);

        // Clear the  EXTI line pending bit
        EXTI_ClearITPendingBit(EXTI_Line12);
    }

    if(EXTI_GetITStatus(EXTI_Line13) != RESET) {
        Exti::ExtiHandler(Exti::EXTI_13);

        // Clear the  EXTI line pending bit
        EXTI_ClearITPendingBit(EXTI_Line13);
    }

    if(EXTI_GetITStatus(EXTI_Line14) != RESET) {
        Exti::ExtiHandler(Exti::EXTI_14);

        // Clear the  EXTI line pending bit
        EXTI_ClearITPendingBit(EXTI_Line14);
    }

    if(EXTI_GetITStatus(EXTI_Line15) != RESET) {
        Exti::ExtiHandler(Exti::EXTI_15);

        // Clear the  EXTI line pending bit
        EXTI_ClearITPendingBit(EXTI_Line15);
    }
}