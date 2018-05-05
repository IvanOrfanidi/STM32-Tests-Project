
#ifndef ADC_GENERAL_H_
#define ADC_GENERAL_H_

/* Private define ------------------------------------------------------------*/
#define ADC1_DR_Address ((uint32_t)0x4001244C)

void vAdcTask(void* pvParameters);
void InitADC(void);

#endif