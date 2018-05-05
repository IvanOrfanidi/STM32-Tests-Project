#ifndef _ADC_H_
#define _ADC_H_

/* Private define ------------------------------------------------------------*/
#define ADC1_DR_Address ((uint32_t)0x4001244C)

#ifndef ADC_CALIBRATION
#   define ADC_CALIBRATION 1
#endif

typedef struct
{
   uint16_t usWkupPinValue;
   uint16_t usBatMeasValue;
} ADCmeasCFG;

void ADC_Configuration(void);
void GetAdcValue(ADCmeasCFG* pstAdcSource);

#endif
