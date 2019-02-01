/**
 ******************************************************************************
 * @file    stm32_eval.h
 * @author  MCD Application Team
 * @version V3.1.2
 * @date    09/28/2009
 * @brief   Header file for stm32_eval.c module.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32_EVAL_H
#define __STM32_EVAL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

/** @addtogroup Utilities
 * @{
 */

/** @addtogroup STM3210E_EVAL
 * @{
 */

/** @defgroup STM3210E_EVAL_Exported_Types
 * @{
 */
/**
 * @}
 */

/** @defgroup STM3210E_EVAL_Exported_Constants
 * @{
 */
/** @addtogroup STM3210E_EVAL_LED
 * @{
 */
#define LEDn 4
#define LED1_GPIO_PORT GPIOB
#define LED1_GPIO_CLK RCC_APB2Periph_GPIOB
#define LED1_GPIO_PIN GPIO_Pin_9

#define LED2_GPIO_PORT GPIOC
#define LED2_GPIO_CLK RCC_APB2Periph_GPIOC
#define LED2_GPIO_PIN GPIO_Pin_8

#define LED3_GPIO_PORT GPIOA
#define LED3_GPIO_CLK RCC_APB2Periph_GPIOA
#define LED3_GPIO_PIN GPIO_Pin_9

/** @addtogroup STM3210E_EVAL_COM
 * @{
 */
#define COMn 2

/**
 * @brief Definition for COM port1, connected to USART1
 */
#define EVAL_COM1 USART2
#define EVAL_COM1_GPIO GPIOD
#define EVAL_COM1_CLK RCC_APB1Periph_USART2
#define EVAL_COM1_GPIO_CLK RCC_APB2Periph_GPIOD
#define EVAL_COM1_RxPin GPIO_Pin_6
#define EVAL_COM1_TxPin GPIO_Pin_5

/**
 * @brief Definition for COM port2, connected to USART2
 */
#define EVAL_COM2 USART3
#define EVAL_COM2_GPIO GPIOD
#define EVAL_COM2_CLK RCC_APB1Periph_USART3
#define EVAL_COM2_GPIO_CLK RCC_APB2Periph_GPIOD
#define EVAL_COM2_RxPin GPIO_Pin_9
#define EVAL_COM2_TxPin GPIO_Pin_8

typedef enum {
    LED1 = 0,
    LED2 = 1,
    LED3 = 2,
    LED4 = 3
} Led_TypeDef;

typedef enum {
    Mode_GPIO = 0,
    Mode_EXTI = 1
} Button_Mode_TypeDef;

typedef enum {
    COM1 = 0,
    COM2 = 1
} COM_TypeDef;

/**
 * @}
 */

/** @defgroup STM32_EVAL_Exported_Macros
 * @{
 */
/**
 * @}
 */

/** @defgroup STM32_EVAL_Exported_Functions
 * @{
 */
void STM_EVAL_LEDInit(Led_TypeDef Led);
void STM_EVAL_LEDOn(Led_TypeDef Led);
void STM_EVAL_LEDOff(Led_TypeDef Led);
void STM_EVAL_LEDToggle(Led_TypeDef Led);
void STM_EVAL_COMInit(COM_TypeDef COM, USART_InitTypeDef* USART_InitStruct);

#ifdef __cplusplus
}
#endif

#endif /* __STM32_EVAL_H */
/**
 * @}
 */

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
