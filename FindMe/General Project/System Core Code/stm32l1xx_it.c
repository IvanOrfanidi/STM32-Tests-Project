/**
 ******************************************************************************
 * @file    Project/STM32L1xx_StdPeriph_Template/stm32l1xx_it.c
 * @author  MCD Application Team
 * @version V1.0.0RC1
 * @date    07/02/2010
 * @brief   Main Interrupt Service Routines.
 *          This file provides template for all exceptions handler and
 *          peripherals interrupt service routine.
 ******************************************************************************
 * @copy
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx_it.h"
#include "includes.h"

/** @addtogroup Template_Project
 * @{
 */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
 * @brief  This function handles NMI exception.
 * @param  None
 * @retval None
 */
void NMI_Handler(void)
{
    if(RCC->CIR & RCC_CIR_CSSF) {
        RCC->CIR |= RCC_CIR_CSSC;    //Сбросить флаг системы контроля сбоя HSE
        RCC->CR &= ~RCC_CR_CSSON;    //Отключаем работу системы защиты сбоя HSE
        SetSysClockHSI();            //переключаем тактирование на работу от внутренней RC цепочки
    }
}

/**
 * @brief  This function handles Hard Fault exception.
 * @param  None
 * @retval None
 */
xTaskHandle xCurrentTaskHandle;    // ID текущего процесса(Debug)
char* pNameCurrentTask = NULL;     //Имя текущего процесса(Debug)
volatile unsigned long StackPointer = 0;
volatile uint8_t* pProgramCounter = NULL;
volatile uint32_t ProgramCounter = 0;

void HardFault_Handler(void)
{
    /* Go to infinite loop when Hard Fault exception occurs */
#ifdef FREERTOS_CONFIG_H
    xCurrentTaskHandle = xTaskGetCurrentTaskHandle();
    pNameCurrentTask = pcTaskGetTaskName(xCurrentTaskHandle);
#endif
    StackPointer = __get_PSP();
    if(!(StackPointer)) {
        StackPointer = __get_MSP();
        pProgramCounter = (uint8_t*)(StackPointer + 0x30);
    }
    else {
        pProgramCounter = (uint8_t*)(StackPointer + 0x18);
    }

    for(int8_t i = sizeof(ProgramCounter); i >= 0; i--) {
        ProgramCounter |= pProgramCounter[i] << i * 8;
    }
    while(1)
        ;    //Ждем перезагрузки по watchdogу.
}

/**
 * @brief  This function handles Memory Manage exception.
 * @param  None
 * @retval None
 */
void MemManage_Handler(void)
{
    /* Go to infinite loop when Memory Manage exception occurs */
    while(1) {
    }
}

/**
 * @brief  This function handles Bus Fault exception.
 * @param  None
 * @retval None
 */
void BusFault_Handler(void)
{
    /* Go to infinite loop when Bus Fault exception occurs */
    while(1) {
    }
}

/**
 * @brief  This function handles Usage Fault exception.
 * @param  None
 * @retval None
 */
void UsageFault_Handler(void)
{
    /* Go to infinite loop when Usage Fault exception occurs */
    while(1) {
    }
}

/**
 * @brief  This function handles Debug Monitor exception.
 * @param  None
 * @retval None
 */
void DebugMon_Handler(void)
{
}

/**
 * @brief  This function handles External interrupt Line 0 request.
 * @param  None
 * @retval None
 */
/*
// Внешнее прерывание от Акселероиетра.
void EXTI0_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line0) != RESET) {
 
    // Clear WAKEUP_BUTTON_EXTI_LINE pending bit
    EXTI_ClearITPendingBit(EXTI_Line0);
    s32 cmd = CMD_ACCEL_READ_IRQ;
    xQueueSendFromISR(xAccelQueue, &cmd, NULL);
  }
}
*/
/**
 * @brief  This function handles RTC_WKUP_IRQHandler .
 * @param  None
 * @retval : None
 */
void RTC_WKUP_IRQHandler(void)
{
    if(RTC_GetITStatus(RTC_IT_WUT) != RESET) {
        RTC_ClearITPendingBit(RTC_IT_WUT);
        RTC_WaitForSynchro();
    }
    if(RTC_GetITStatus(RTC_IT_ALRA) != RESET) {
        RTC_ClearITPendingBit(RTC_IT_ALRA);
        RTC_WaitForSynchro();
    }
    if(RTC_GetITStatus(RTC_IT_ALRB) != RESET) {
        RTC_ClearITPendingBit(RTC_IT_ALRB);
        RTC_WaitForSynchro();
    }
}
/**
 * @brief  This function handles RTC_Alarm_IRQHandler .
 * @param  None
 * @retval : None
 */
void RTC_Alarm_IRQHandler(void)
{
    if(RTC_GetITStatus(RTC_IT_ALRA) != RESET) {
        RTC_ClearITPendingBit(RTC_IT_ALRA);
        RTC_WaitForSynchro();
    }
    if(RTC_GetITStatus(RTC_IT_ALRB) != RESET) {
        RTC_ClearITPendingBit(RTC_IT_ALRB);
        RTC_WaitForSynchro();
    }
    EXTI_ClearITPendingBit(EXTI_Line17);
}
/**
 * @}
 */

/**
 * @brief  This function handles the PVD Output interrupt request.
 * @param  None
 * @retval None
 */
void PVD_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line16) != RESET) {
        /* Clear the Key Button EXTI line pending bit */
        EXTI_ClearITPendingBit(EXTI_Line16);
    }
}

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
