/**
 ******************************************************************************
 * @file    stm32l0xx_it.c
 * @brief   Interrupt Service Routines.
 ******************************************************************************
 *
 * COPYRIGHT(c) 2017 STMicroelectronics
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_hal.h"
#include "stm32l0xx.h"
#include "stm32l0xx_it.h"

/* External variables --------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M0+ Processor Interruption and Exception Handlers         */
/******************************************************************************/
void NMI_Handler(void)
{
}

/**
 * @brief This function handles Hard fault interrupt.
 */
void HardFault_Handler(void)
{
   while (1)
   {
   }
}

/**
 * @brief This function handles System tick timer.
 */
void SysTick_Handler(void)
{
   HAL_IncTick();
}

/******************************************************************************/
/* STM32L0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32l0xx.s).                    */
/******************************************************************************/
__weak void RTC_IRQHandler(void)
{
}

/**
 * @brief This function handles EXTI line 4 to 15 interrupts.
 */
__weak void EXTI4_15_IRQHandler(void)
{
   HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
}

/**
This function handles LPTIM1 global interrupt / LPTIM1 wake-up interrupt through EXTI line 29.
*/
__weak void LPTIM1_IRQHandler(void)
{
}

/**
 * @brief This function handles ADC1, COMP1 and COMP2 interrupts (COMP interrupts through EXTI lines 21 and 22).
 */
__weak void ADC1_COMP_IRQHandler(void)
{
}

/**
 * @brief Handle UART interrupt request.
 * @param huart: UART handle.
 * @retval None
 */
__weak void LPUART1_IRQHandler(void)
{
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
