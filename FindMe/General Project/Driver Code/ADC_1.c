
#include "ADC_1.h"
#include "includes.h"

float f_mk_temp;
uint16_t ConverMeas(uint16_t);

/* Init ADC */
void ADCInit(void)
{
   ADC_CommonInitTypeDef ADC_CommonInitStructure;
   ADC_InitTypeDef ADC_InitStructure;

   if (!(RCC->CR & RCC_CR_HSIRDY))
   {   //Если не запущен внутренний осцилятор, то запустим его
      /* Enable The HSI (16Mhz) */
      RCC_HSICmd(ENABLE);
      __IO uint32_t StartUpCounter = 0, HSIStatus = 0;
      /* Wait till HSI is ready and if Time out is reached exit */
      do
      {
         HSIStatus = RCC->CR & RCC_CR_HSIRDY;
         StartUpCounter++;
      } while ((HSIStatus == 0) && (StartUpCounter != HSI_STARTUP_TIMEOUT));
   }

#ifdef FM4
   /* Init ADC GPIO */
   RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);   // ADC GPIO clock enable
   GPIO_InitTypeDef GPIO_InitStructure;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

   GPIO_InitStructure.GPIO_Pin = VIN_Meas_PIN;
   GPIO_Init(ADC_PORT, &GPIO_InitStructure);
#endif

   /* ADC1 clock enable */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

   /* ADC Common Init **********************************************************/
   ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
   ADC_CommonInit(&ADC_CommonInitStructure);

   /* ADC1 Configuration -----------------------------------------------------*/
   ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
   ADC_InitStructure.ADC_ScanConvMode = ENABLE;
   ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
   ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
   ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
   ADC_InitStructure.ADC_NbrOfConversion = 1;
   ADC_Init(ADC1, &ADC_InitStructure);

   /* ADC1 regular channel1 configuration */
   ADC_RegularChannelConfig(ADC1, VIN_ADC_CHNL_NUM, 1, ADC_SampleTime_384Cycles);

   /*если первое включение, то получим первое значение*/
   osDelay(SLEEP_MS_100);
   ADC_Cmd(ADC1, ENABLE);   // Enable ADC.
   while (ADC_GetFlagStatus(ADC1, ADC_FLAG_ADONS) == RESET)
   {
      osDelay(SLEEP_MS_1);
   }
   //запускаем преобразование
   ADC_SoftwareStartConv(ADC1);
   while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET)
   {
      osDelay(SLEEP_MS_1);
   }

   g_stInput.Meas_VIN = ConverMeas(ADC_GetConversionValue(ADC1));

   ADC_Cmd(ADC1, DISABLE);   // Disable ADC.
   // DP_GSM("D_MeasVin: %i\r\n", g_stInput.Meas_VIN);
}

void AdcTemperaturHandler(void)
{
   uint16_t usAverageVal;
   //получим температуру с сенсора МК
   ADC_RegularChannelConfig(ADC1, ADC_Channel_TempSensor, 1, ADC_SampleTime_384Cycles);
   ADC_Cmd(ADC1, ENABLE);
   while (ADC_GetFlagStatus(ADC1, ADC_FLAG_ADONS) == RESET)
   {
      osDelay(SLEEP_MS_1);
   }

   ADC_TempSensorVrefintCmd(ENABLE);
   ADC_SoftwareStartConv(ADC1);   //запускаем преобразование
   while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET)
   {
      osDelay(SLEEP_MS_1);
   }

   usAverageVal = ADC_GetConversionValue(ADC1);

   //выключаем температрный датчик и АЦП
   ADC_TempSensorVrefintCmd(DISABLE);
   ADC_Cmd(ADC1, DISABLE);

   /* пересчитаем в градусы цельсия */
   //калибровочные константы
   uint16_t ts_cal1 = *((uint16_t*)((uint32_t)TS_CAL1));
   uint16_t ts_cal2 = *((uint16_t*)((uint32_t)TS_CAL2));
   float k_mV_C = ((float)(ts_cal2 - ts_cal1)) / 80.0 * 3.0 / 4.096;

   //пересчитаем в градусы цельсия
#ifndef VCC_MCU
#   define VCC_MCU 3300
#endif
   f_mk_temp = VCC_MCU * ((float)usAverageVal) / 4096.0;   //напряжение на сенсоре
   f_mk_temp -= ((float)ts_cal1) * 3000.0 / 4096.0;   //разница в мВ с калибровочным значением при 30 градусах
   f_mk_temp = f_mk_temp / k_mV_C - 30;   //приводим к градусам цельсия
   // DP_GSM("D_ADC Temperatur: %f C\r\n", f_mk_temp);

   /* ADC1 regular channel1 configuration */
   ADC_RegularChannelConfig(ADC1, VIN_ADC_CHNL_NUM, 1, ADC_SampleTime_384Cycles);
}

void AdcHandler(void)
{
#ifdef FM3
   if (!(GSM_STATUS_ON))
      return;
#endif
   osDelay(SLEEP_MS_10);
   ADC_Cmd(ADC1, ENABLE);   // Enable ADC.
   while (ADC_GetFlagStatus(ADC1, ADC_FLAG_ADONS) == RESET)
   {
      osDelay(SLEEP_MS_1);
   }
   ADC_SoftwareStartConv(ADC1);   //запускаем преобразование
   while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET)
   {
      osDelay(SLEEP_MS_1);
   }

   uint16_t usAverageVal = ConverMeas(ADC_GetConversionValue(ADC1));   // Get Value
   ADC_Cmd(ADC1, DISABLE);   // Disable ADC.

   g_stInput.SecADC = time();
   /*складываем текущее значение с предыдущим и делим на два, чтобы получить среднее значение*/
   g_stInput.Meas_VIN = (g_stInput.Meas_VIN + usAverageVal) >> 1;
   // DP_GSM("D_MeasVin: %d\r\n", g_stInput.Meas_VIN);
}

static uint16_t ConverMeas(uint16_t Meas)
{
#ifdef FM3
   uint16_t vrefint_cal = *((uint16_t*)((uint32_t)VREFINT_CAL));   // internal reference voltage calibration values
   int32_t factory_ref_voltage = 3 * vrefint_cal * 1000;   // manufacturing process at VDDA = 3V
   uint16_t res = factory_ref_voltage / Meas;
#endif
#ifdef FM4
   uint16_t res = (uint16_t)((Meas * VCC_MCU / 4096) * BLEEDER_VIN) + DIODE_DOWN;
#endif
   return res;
}

int8_t GetTemperaturAdc(void)
{
   return (int8_t)(f_mk_temp);
}

uint16_t GetMeasVin(void)
{
   return g_stInput.Meas_VIN;
}
