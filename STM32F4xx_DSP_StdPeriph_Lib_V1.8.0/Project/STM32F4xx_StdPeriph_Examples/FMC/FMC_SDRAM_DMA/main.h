/**
  ******************************************************************************
  * @file    FMC/FMC_SDRAM_DMA/main.h
  * @author  MCD Application Team
  * @version V1.8.0
  * @date    04-November-2016
  * @brief   Header for main.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2016 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm324x9i_eval.h"
#include "stm324x9i_eval_fmc_sdram.h"

/* Private defines -----------------------------------------------------------*/
/* DMA configurations for transfer with FMC */
#define DMA_STREAM DMA2_Stream0
#define DMA_CHANNEL DMA_Channel_0
#define DMA_STREAM_CLOCK RCC_AHB1Periph_DMA2
#define DMA_STREAM_IRQ DMA2_Stream0_IRQn
#define DMA_IT_TCIF DMA_IT_TCIF0
#define DMA_FLAG_TCIF DMA_FLAG_TCIF0
#define DMA_STREAM_IRQHANDLER DMA2_Stream0_IRQHandler

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
