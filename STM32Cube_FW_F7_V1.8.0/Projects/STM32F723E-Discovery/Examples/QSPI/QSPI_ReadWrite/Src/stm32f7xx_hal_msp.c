/**
 ******************************************************************************
 * @file    QSPI/QSPI_ReadWrite/Src/stm32f7xx_hal_msp.c
 * @author  MCD Application Team
 * @brief   HAL MSP module.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
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
#include "main.h"

/** @addtogroup STM32F7xx_HAL_Examples
 * @{
 */

/** @defgroup HAL_MSP
 * @brief HAL MSP module.
 * @{
 */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup HAL_MSP_Private_Functions
 * @{
 */

/**
 * @brief QSPI MSP Initialization
 *        This function configures the hardware resources used in this example:
 *           - Peripheral's clock enable
 *           - Peripheral's GPIO Configuration
 *           - NVIC configuration for QSPI interrupts
 * @param hqspi: QSPI handle pointer
 * @retval None
 */
void HAL_QSPI_MspInit(QSPI_HandleTypeDef* hqspi)
{
   GPIO_InitTypeDef gpio_init_structure;

   /*##-1- Enable peripherals and GPIO Clocks #################################*/
   /* Enable the QuadSPI memory interface clock */
   QSPI_CLK_ENABLE();

   /* Reset the QuadSPI memory interface */
   QSPI_FORCE_RESET();
   QSPI_RELEASE_RESET();

   /* Enable GPIO clocks */
   QSPI_CS_GPIO_CLK_ENABLE();
   QSPI_CLK_GPIO_CLK_ENABLE();
   QSPI_D0_GPIO_CLK_ENABLE();
   QSPI_D1_GPIO_CLK_ENABLE();
   QSPI_D2_GPIO_CLK_ENABLE();
   QSPI_D3_GPIO_CLK_ENABLE();

   /*##-2- Configure peripheral GPIO ##########################################*/
   /* QSPI CS GPIO pin configuration  */
   gpio_init_structure.Pin = QSPI_CS_PIN;
   gpio_init_structure.Alternate = QSPI_CS_PIN_AF;
   gpio_init_structure.Mode = GPIO_MODE_AF_PP;
   gpio_init_structure.Pull = GPIO_PULLUP;
   gpio_init_structure.Speed = GPIO_SPEED_FREQ_HIGH;
   HAL_GPIO_Init(QSPI_CS_GPIO_PORT, &gpio_init_structure);

   /* QSPI CLK GPIO pin configuration  */
   gpio_init_structure.Pin = QSPI_CLK_PIN;
   gpio_init_structure.Alternate = QSPI_CLK_PIN_AF;
   gpio_init_structure.Pull = GPIO_NOPULL;
   HAL_GPIO_Init(QSPI_CLK_GPIO_PORT, &gpio_init_structure);

   /* QSPI D0 GPIO pin configuration  */
   gpio_init_structure.Pin = QSPI_D0_PIN;
   gpio_init_structure.Alternate = QSPI_D0_PIN_AF;
   HAL_GPIO_Init(QSPI_D0_GPIO_PORT, &gpio_init_structure);

   /* QSPI D1 GPIO pin configuration  */
   gpio_init_structure.Pin = QSPI_D1_PIN;
   gpio_init_structure.Alternate = QSPI_D1_PIN_AF;
   HAL_GPIO_Init(QSPI_D1_GPIO_PORT, &gpio_init_structure);

   /* QSPI D2 GPIO pin configuration  */
   gpio_init_structure.Pin = QSPI_D2_PIN;
   gpio_init_structure.Alternate = QSPI_D2_PIN_AF;
   HAL_GPIO_Init(QSPI_D2_GPIO_PORT, &gpio_init_structure);

   /* QSPI D3 GPIO pin configuration  */
   gpio_init_structure.Pin = QSPI_D3_PIN;
   gpio_init_structure.Alternate = QSPI_D3_PIN_AF;
   HAL_GPIO_Init(QSPI_D3_GPIO_PORT, &gpio_init_structure);

   /*##-3- Configure the NVIC for QSPI #########################################*/
   /* NVIC configuration for QSPI interrupt */
   HAL_NVIC_SetPriority(QUADSPI_IRQn, 0x0F, 0);
   HAL_NVIC_EnableIRQ(QUADSPI_IRQn);
}

/**
 * @brief QSPI MSP De-Initialization
 *        This function frees the hardware resources used in this example:
 *          - Disable the Peripheral's clock
 *          - Revert GPIO and NVIC configuration to their default state
 * @param hqspi: QSPI handle pointer
 * @retval None
 */
void HAL_QSPI_MspDeInit(QSPI_HandleTypeDef* hqspi)
{
   /*##-1- Disable the NVIC for QSPI ###########################################*/
   HAL_NVIC_DisableIRQ(QUADSPI_IRQn);

   /*##-2- Disable peripherals and GPIO Clocks ################################*/
   /* De-Configure QSPI pins */
   HAL_GPIO_DeInit(QSPI_CS_GPIO_PORT, QSPI_CS_PIN);
   HAL_GPIO_DeInit(QSPI_CLK_GPIO_PORT, QSPI_CLK_PIN);
   HAL_GPIO_DeInit(QSPI_D0_GPIO_PORT, QSPI_D0_PIN);
   HAL_GPIO_DeInit(QSPI_D1_GPIO_PORT, QSPI_D1_PIN);
   HAL_GPIO_DeInit(QSPI_D2_GPIO_PORT, QSPI_D2_PIN);
   HAL_GPIO_DeInit(QSPI_D3_GPIO_PORT, QSPI_D3_PIN);

   /*##-3- Reset peripherals ##################################################*/
   /* Reset the QuadSPI memory interface */
   QSPI_FORCE_RESET();
   QSPI_RELEASE_RESET();

   /* Disable the QuadSPI memory interface clock */
   QSPI_CLK_DISABLE();
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/