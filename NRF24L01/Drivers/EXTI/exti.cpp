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
#include "stm32f10x_gpio.h"


uint32_t Exti0::Cpm = 0;


/**
 * @bref Initialisation EXT0 Interrupt
 */
void Exti0::InitNvic(uint8_t preemption_priority = 0, uint8_t sub_priority = 0)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable GPIOA clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    /* Configure PA.00 pin as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Enable AFIO clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    /* Tell system that you will use PB0 for EXTI_Line0 */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource0);

    /* Configure EXTI0 line */
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; // Triggers on falling edge
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Enable and set EXTI0 Interrupt to the lowest priority */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = preemption_priority;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = sub_priority;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_EnableIRQ(EXTI0_IRQn);
}

void Exti0::SetCpm(uint32_t cpm)
{
    NVIC_DisableIRQ(EXTI0_IRQn);
    Cpm = cpm;
    NVIC_EnableIRQ(EXTI0_IRQn);
}

uint32_t Exti0::GetCpm()
{
    return Cpm;
}

void EXTI0_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
       while(!(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0)));
       
       Exti0::Cpm++;
       
       /* Clear the  EXTI line 0 pending bit */
       EXTI_ClearITPendingBit(EXTI_Line0);
    }
}