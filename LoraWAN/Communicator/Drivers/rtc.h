#ifndef _RTC_H
#define _RTC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "stm32l0xx_hal.h"

#define visocosn(year) ((year % 4) ? 0 : 1)

void MX_RTC_Init(void);
void HAL_RTC_MspDeInit(RTC_HandleTypeDef* hrtc);
void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc);

void Sec2Date(RTC_TimeTypeDef* pDestTime, RTC_DateTypeDef* pDestDate, uint32_t ulSec);
uint32_t Date2Sec(RTC_TimeTypeDef* pDestTime, RTC_DateTypeDef* pDestDate);

#ifdef __cplusplus
}
#endif

#endif