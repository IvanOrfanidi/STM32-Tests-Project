
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LPTIM_H
#define __LPTIM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32l0xx_hal.h"

void MX_LPTIM1_Init(void);
void HAL_LPTIM_MspInit(LPTIM_HandleTypeDef* hlptim);
void HAL_LPTIM_MspDeInit(LPTIM_HandleTypeDef* hlptim);
uint32_t getLtimCountVal(void);

#ifdef __cplusplus
}
#endif

#endif