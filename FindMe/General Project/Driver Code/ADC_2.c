
#include "includes.h"
#include "ADC_2.h"

uint16_t g_ausADC_ConvertedValueOne[ADC_BUFFER_SIZE];
u16 usAverageVal;

// Init ADC
void ADCInit(void)
{
   GPIO_InitTypeDef GPIO_InitStructure;
   ADC_InitTypeDef ADC_InitStructure;
   ADC_CommonInitTypeDef ADC_CommonInitStructure;
   DMA_InitTypeDef DMA_InitStructure;
   NVIC_InitTypeDef NVIC_InitStructure;

   // Init ADC GPIO
   RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);   // ADC GPIO clock enable

   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

   GPIO_InitStructure.GPIO_Pin = VIN_Meas_PIN;
   GPIO_Init(ADC_PORT, &GPIO_InitStructure);

   //*************************************//

   // ADC1 clock enable
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

   /*------------------------ DMA1 configuration ------------------------------*/
   /* Enable DMA1 clock */
   RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

   /*----------------- ADC1 configuration with DMA enabled --------------------*/
   /* Enable the HSI oscillator */
   RCC_HSICmd(ENABLE);

   /* Check that HSI oscillator is ready */
   while (RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET)
      IWDG_ReloadCounter();
   ;

   /* Enable ADC1 clock */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

   /* ADC Common Init **********************************************************/
   ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div1;
   ADC_CommonInit(&ADC_CommonInitStructure);

   /* ADC1 configuration */
   ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
   ADC_InitStructure.ADC_ScanConvMode = ENABLE;
   ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
   ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
   ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
   ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_TRGO;
   ADC_InitStructure.ADC_NbrOfConversion = 1;
   ADC_Init(ADC1, &ADC_InitStructure);

   /* ADC1 regular channel1 configuration */
   ADC_RegularChannelConfig(ADC1, VIN_ADC_CHNL_NUM, 1, ADC_SampleTime_16Cycles);

   /* DMA1 channel1 configuration */
   DMA_DeInit(DMA1_Channel1);
   DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_ADDRESS;
   DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)g_ausADC_ConvertedValueOne;
   DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
   DMA_InitStructure.DMA_BufferSize = sizeof(g_ausADC_ConvertedValueOne) / (sizeof(uint16_t));
   DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
   DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
   DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
   DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
   DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
   DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
   DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
   DMA_Init(DMA1_Channel1, &DMA_InitStructure);

   /* Enable the request after last transfer for DMA Circular mode */
   ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);

   /* Wait until the ADC1 is ready */
   /*
   while(ADC_GetFlagStatus(ADC1, ADC_FLAG_ADONS) == RESET){
      IWDG_ReloadCounter();
   }
   */

   // Interrupt DMA ADC
   NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
   DMA_ITConfig(DMA1_Channel1, DMA_IT_TC | DMA_IT_HT, ENABLE);   // Interrupt ADC_BUFFER_SIZE and ADC_BUFFER_SIZE/2

   /* Enable DMA1 channel1 */
   DMA_Cmd(DMA1_Channel1, ENABLE);
   /* Enable ADC1 */
   ADC_Cmd(ADC1, ENABLE);
   /* Enable ADC1 DMA */
   ADC_DMACmd(ADC1, ENABLE);

   /* Start ADC1 Software Conversion */
   ADC_SoftwareStartConv(ADC1);
}

void ADCDeInit(void)
{
   ADC_DMACmd(ADC1, DISABLE);
   ADC_Cmd(ADC1, DISABLE);
   DMA_Cmd(DMA1_Channel1, DISABLE);
}

void AdcHandler(void)
{
   RTC_t DateRTC;
   uint16_t usTempAverageVal;

   rtc_gettime(&DateRTC);
   g_stInput.SecADC = Date2Sec(&DateRTC);

   __disable_interrupt();
   usTempAverageVal = usAverageVal;
   __enable_interrupt();

   g_stInput.Meas_VIN = (uint16_t)((usTempAverageVal * 3300 / 4096) * BLEEDER_VIN) + 1500;
}

void CalcAnalogInputs(uint16_t* pData)
{
   uint32_t uiTempAver = 0;
   for (int i = 0; i < ADC_BUFFER_SIZE / 2; i++)
   {
      uiTempAver += pData[i];
   }
   uiTempAver /= ADC_BUFFER_SIZE / 2;
   usAverageVal = (u16)uiTempAver;
}

/* INTERRUPT */
void DMA1_Channel1_IRQHandler(void)
{
   if (DMA_GetITStatus(DMA1_IT_TC1))   // Full
   {
      CalcAnalogInputs(&g_ausADC_ConvertedValueOne[ADC_BUFFER_SIZE / 2]);
      DMA_ClearITPendingBit(DMA1_IT_TC1);
   }

   if (DMA_GetITStatus(DMA1_IT_HT1))   // Half
   {
      CalcAnalogInputs(&g_ausADC_ConvertedValueOne[0]);
      DMA_ClearITPendingBit(DMA1_IT_HT1);
   }
}