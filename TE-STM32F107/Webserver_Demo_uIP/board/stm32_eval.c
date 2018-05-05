/**
 ******************************************************************************
 * @file    stm32_eval.c
 * @author  MCD Application Team
 * @version V3.1.2
 * @date    09/28/2009
 * @brief   This file provides firmware functions to manage Leds, push-buttons
 *          and COM ports available on STM32 Evaluation Boards from STMicroelectronics.
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
 * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32_eval.h"

/** @addtogroup Utilities
 * @{
 */

/** @defgroup STM32_EVAL
 * @brief This file provides firmware functions to manage Leds, push-buttons
 *        and COM ports available on STM32 Evaluation Boards from STMicroelectronics.
 * @{
 */

/** @defgroup STM32_EVAL_Private_TypesDefinitions
 * @{
 */
/**
 * @}
 */

/** @defgroup STM32_EVAL_Private_Defines
 * @{
 */
/**
 * @}
 */

/** @defgroup STM32_EVAL_Private_Macros
 * @{
 */
/**
 * @}
 */

/** @defgroup STM32_EVAL_Private_Variables
 * @{
 */
GPIO_TypeDef* GPIO_PORT[LEDn] = { LED1_GPIO_PORT, LED2_GPIO_PORT, LED3_GPIO_PORT };
const uint16_t GPIO_PIN[LEDn] = { LED1_GPIO_PIN, LED2_GPIO_PIN, LED3_GPIO_PIN };
const uint32_t GPIO_CLK[LEDn] = { LED1_GPIO_CLK, LED2_GPIO_CLK, LED3_GPIO_CLK };

USART_TypeDef* COM_USART[COMn] = { EVAL_COM1, EVAL_COM2 };

GPIO_TypeDef* COM_PORT[COMn] = { EVAL_COM1_GPIO, EVAL_COM2_GPIO };

const uint32_t COM_USART_CLK[COMn] = { EVAL_COM1_CLK, EVAL_COM2_CLK };

const uint32_t COM_POR_CLK[COMn] = { EVAL_COM1_GPIO_CLK, EVAL_COM2_GPIO_CLK };

const uint16_t COM_TX_PIN[COMn] = { EVAL_COM1_TxPin, EVAL_COM2_TxPin };

const uint16_t COM_RX_PIN[COMn] = { EVAL_COM1_RxPin, EVAL_COM2_RxPin };

/**
 * @}
 */

/** @defgroup STM32_EVAL_Private_FunctionPrototypes
 * @{
 */

/**
 * @}
 */

/** @defgroup STM32_EVAL_Private_Functions
 * @{
 */

/**
 * @brief  Configures LED GPIO.
 * @param  Led: Specifies the Led to be configured.
 *   This parameter can be one of following parameters:
 *     @arg LED1
 *     @arg LED2
 *     @arg LED3
 * @retval None
 */
void STM_EVAL_LEDInit(Led_TypeDef Led)
{
   GPIO_InitTypeDef GPIO_InitStructure;

   /* Enable the GPIO_LED Clock */
   RCC_APB2PeriphClockCmd(GPIO_CLK[Led], ENABLE);

   /* Configure the GPIO_LED pin */
   GPIO_InitStructure.GPIO_Pin = GPIO_PIN[Led];
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

   GPIO_Init(GPIO_PORT[Led], &GPIO_InitStructure);
}

/**
 * @brief  Turns selected LED On.
 * @param  Led: Specifies the Led to be set on.
 *   This parameter can be one of following parameters:
 *     @arg LED1
 *     @arg LED2
 *     @arg LED3
 *     @arg LED4
 * @retval None
 */
void STM_EVAL_LEDOn(Led_TypeDef Led)
{
   GPIO_PORT[Led]->BSRR = GPIO_PIN[Led];
}

/**
 * @brief  Turns selected LED Off.
 * @param  Led: Specifies the Led to be set off.
 *   This parameter can be one of following parameters:
 *     @arg LED1
 *     @arg LED2
 *     @arg LED3
 *     @arg LED4
 * @retval None
 */
void STM_EVAL_LEDOff(Led_TypeDef Led)
{
   GPIO_PORT[Led]->BRR = GPIO_PIN[Led];
}

/**
 * @brief  Toggles the selected LED.
 * @param  Led: Specifies the Led to be toggled.
 *   This parameter can be one of following parameters:
 *     @arg LED1
 *     @arg LED2
 *     @arg LED3
 *     @arg LED4
 * @retval None
 */
void STM_EVAL_LEDToggle(Led_TypeDef Led)
{
   GPIO_PORT[Led]->ODR ^= GPIO_PIN[Led];
}

/**
 * @brief  Configures COM port.
 * @param  COM: Specifies the COM port to be configured.
 *   This parameter can be one of following parameters:
 *     @arg COM1
 *     @arg COM2
 * @param  USART_InitStruct: pointer to a USART_InitTypeDef structure that
 *   contains the configuration information for the specified USART peripheral.
 * @retval None
 */
void STM_EVAL_COMInit(COM_TypeDef COM, USART_InitTypeDef* USART_InitStruct)
{
   GPIO_InitTypeDef GPIO_InitStructure;

   /* Enable GPIO clock */
   RCC_APB2PeriphClockCmd(COM_POR_CLK[COM] | RCC_APB2Periph_AFIO, ENABLE);

   /* Enable UART clock */
   if (COM == COM1)
   {
      /* Enable the USART2 Pins Software Remapping */
      GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE);
   }
   else
   {
      /* Enable the USART3 Pins Software Remapping */
      GPIO_PinRemapConfig(GPIO_FullRemap_USART3, ENABLE);
   }

   RCC_APB1PeriphClockCmd(COM_USART_CLK[COM], ENABLE);

   /* Configure USART Tx as alternate function push-pull */
   GPIO_InitStructure.GPIO_Pin = COM_TX_PIN[COM];
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(COM_PORT[COM], &GPIO_InitStructure);

   /* Configure USART Rx as input floating */
   GPIO_InitStructure.GPIO_Pin = COM_RX_PIN[COM];
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
   GPIO_Init(COM_PORT[COM], &GPIO_InitStructure);

   /* USART configuration */
   USART_Init(COM_USART[COM], USART_InitStruct);

   /* Enable USART */
   USART_Cmd(COM_USART[COM], ENABLE);
}

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */
/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
