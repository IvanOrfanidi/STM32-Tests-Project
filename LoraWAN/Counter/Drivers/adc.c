
#include "adc.h"
#include "stm32l0xx_hal.h"
#include <string.h>

ADC_HandleTypeDef hadc;

int index_meas = 0;
uint16_t buff_meas[SIZE_BUFFER_MEAS_ADC];

#ifdef __cplusplus
extern "C" {
#endif

/* ADC init function */
void MX_ADC_Init(void)
{
   __HAL_RCC_ADC1_CLK_ENABLE();
   ADC_ChannelConfTypeDef sConfig;

   /**Configure the global features of the ADC
   (Clock, Resolution, Data Alignment and number of conversion)     */
   hadc.Instance = ADC1;
   hadc.Init.OversamplingMode = DISABLE;
   hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV1;
   hadc.Init.Resolution = ADC_RESOLUTION_12B;
   hadc.Init.SamplingTime = ADC_SAMPLETIME_160CYCLES_5;
   hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
   hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
   hadc.Init.ContinuousConvMode = DISABLE;
   hadc.Init.DiscontinuousConvMode = DISABLE;
   hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
   hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
   hadc.Init.DMAContinuousRequests = DISABLE;
   hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
   hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
   hadc.Init.LowPowerAutoWait = DISABLE;
   hadc.Init.LowPowerFrequencyMode = DISABLE;
   hadc.Init.LowPowerAutoPowerOff = DISABLE;
   if (HAL_ADC_Init(&hadc) != HAL_OK)
   {
      _Error_Handler(__FILE__, __LINE__);
   }

   /**Configure for the selected ADC regular channel to be converted.
    */
   sConfig.Channel = ADC_CHANNEL_VREFINT;
   sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
   if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
   {
      _Error_Handler(__FILE__, __LINE__);
   }

   HAL_ADCEx_EnableVREFINT();

   /* Start ADC */
   HAL_ADC_Start_IT(&hadc);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc1)
{
   uint32_t ConverMeas(uint32_t);
   buff_meas[index_meas++] = (uint16_t)ConverMeas(HAL_ADC_GetValue(&hadc));

   if (index_meas < SIZE_BUFFER_MEAS_ADC)
   {   // Запускаем преобразование АЦП если буфер не заполнен
      HAL_ADC_Start_IT(&hadc);
   }
   else
   {
      // End meas
   }
}

/*
Получить усредненное напряжение питание.
Input: no.
Return: напряжение в mV за n-е количество преобразований.
*/
uint16_t getMeasVin(void)
{
   uint16_t meas = 0;
   loop(index_meas)
   {
      meas = (meas + buff_meas[i]) >> 1;
   }
   extern _Bool __DEBUG__;
   if (__DEBUG__)
   {
      printf("Ubat: %dmV\r\n", meas);
   }
   return meas;
}

/*
Отчика буфера и запуск преобразований АЦП.
*/
void cleanMeasVin(void)
{
   memset(buff_meas, 0, SIZE_BUFFER_MEAS_ADC);
   index_meas = 0;
   HAL_ADC_Start_IT(&hadc);   // Start ADC
}

static uint32_t ConverMeas(uint32_t Meas)
{
   uint32_t vrefint_cal = *((uint16_t*)((uint32_t)VREFINT_CAL));   // internal reference voltage calibration values
   int32_t factory_ref_voltage = 3 * vrefint_cal * 1000;   // manufacturing process at VDDA = 3V
   uint16_t res = factory_ref_voltage / Meas;
   return res;
}

/* INTERRUPT ******************************************************************/
/**
 * @brief This function handles ADC1, COMP1 and COMP2 interrupts (COMP interrupts through EXTI lines 21 and 22).
 */
void ADC1_COMP_IRQHandler(void)
{
   HAL_ADC_IRQHandler(&hadc);
}

#ifdef __cplusplus
}
#endif