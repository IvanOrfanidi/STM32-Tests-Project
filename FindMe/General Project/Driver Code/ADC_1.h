#ifndef _ADC_1_H_
#define _ADC_1_H_
#include <stdint.h>

#define TS_CAL1 (uint32_t)0x1FF8007A    // Temperature sensor calibration values
#define TS_CAL2 (uint32_t)0x1FF8007E    // Temperature sensor calibration values

#define VREFINT_CAL (uint32_t)0x1FF80078    // Internal reference voltage calibration values

void ADCInit(void);
void AdcHandler(void);
void AdcTemperaturHandler(void);
uint16_t GetMeasVin(void);
int8_t GetTemperaturAdc(void);

#endif
