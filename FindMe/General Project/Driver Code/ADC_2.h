#ifndef _ADC_2_H_
#define _ADC_2_H_

#define ADC_BUFFER_SIZE 64
#define ADC1_DR_ADDRESS ((uint32_t)0x40012458)    //Адрес DMA ADC

void ADCInit(void);
void AdcHandler(void);

#endif