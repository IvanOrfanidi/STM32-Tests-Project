#ifndef _ADC_H
#define _ADC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32l0xx_hal.h"

#define TS_CAL1 (uint32_t)0x1FF8007A   // Temperature sensor calibration values
#define TS_CAL2 (uint32_t)0x1FF8007E   // Temperature sensor calibration values

#define VREFINT_CAL (uint32_t)0x1FF80078   // Internal reference voltage calibration values

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc1);
void MX_ADC_Init(void);
uint16_t getMeasVin(void);
void cleanMeasVin(void);

#ifdef __cplusplus
}
#endif

#endif
