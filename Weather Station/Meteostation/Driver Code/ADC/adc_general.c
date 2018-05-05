
#include "adc_general.h"
#include "includes.h"

ADCmeasCFG stAdcSource;

void vAdcTask(void* pvParameters)
{
   // Создаём очередь
   xQueueAdcDataToUsart = xQueueCreate(sizeof(stAdcSource), sizeof(uint32_t));
   vQueueAddToRegistry(xQueueAdcDataToUsart, "xQueueAdcDataToUsart");

   ADCmeasCFG stAdcReceptacle;
   char* pSt = (char*)&stAdcReceptacle;
   char toSend;
   _Bool bAlarmTrue = 0;
   InitADC();

   _delay_ms(1000);

   while (1)
   {
      __disable_interrupt();
      stAdcReceptacle = stAdcSource;
      __enable_interrupt();
      bAlarmTrue = 0;

      if (stAdcReceptacle.usValue1 > 1200)
      {
         bAlarmTrue = 1;
      }

      if (stAdcReceptacle.usValue2 > 1200)
      {
         bAlarmTrue = 1;
      }

      if (stAdcReceptacle.usValue3 > 1200)
      {
         bAlarmTrue = 1;
      }

      if (bAlarmTrue)
      {
      }
      else
      {
      }

      if (xQueueAdcDataToUsart != 0)
      {
         for (uint8_t i = 0; i < sizeof(stAdcReceptacle); i++)
         {
            toSend = pSt[i];
            xQueueSendToFront(xQueueAdcDataToUsart, (void*)&toSend, (portTickType)0);
         }
      }
      _delay_ms(1000);
   }

   vTaskDelete(NULL);
}

void InitADC(void)
{
   /* Private variables ---------------------------------------------------------*/
   ADC_InitTypeDef ADC_InitStructure;
   DMA_InitTypeDef DMA_InitStructure;
   NVIC_InitTypeDef NVIC_InitStructure;
   GPIO_InitTypeDef GPIO_InitStructure;

   /* ADCCLK = PCLK2/2 */
   RCC_ADCCLKConfig(RCC_PCLK2_Div2);
   /* Enable peripheral clocks ------------------------------------------------*/
   /* Enable DMA1 clock */
   RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
   /* Enable ADC1 and GPIOC clock */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA, ENABLE);

   /* Configure PC.04 (ADC Channel14) as analog input -------------------------*/
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
   GPIO_Init(GPIOA, &GPIO_InitStructure);

   /* DMA1 channel1 configuration ----------------------------------------------*/
   DMA_DeInit(DMA1_Channel1);
   DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
   DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&stAdcSource;
   DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
   DMA_InitStructure.DMA_BufferSize = sizeof(stAdcSource) / (sizeof(uint16_t));
   DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
   DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
   DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
   DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
   DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
   DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
   DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
   DMA_Init(DMA1_Channel1, &DMA_InitStructure);

   /* Enable DMA1 channel1 */
   DMA_Cmd(DMA1_Channel1, ENABLE);

   /* ADC1 configuration ------------------------------------------------------*/
   ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
   ADC_InitStructure.ADC_ScanConvMode = ENABLE;
   ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
   ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
   ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
   ADC_InitStructure.ADC_NbrOfChannel = sizeof(stAdcSource) / (sizeof(uint16_t));
   ADC_Init(ADC1, &ADC_InitStructure);

   /* ADC1 regular channel14 configuration */
   ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 1, ADC_SampleTime_55Cycles5);
   ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 2, ADC_SampleTime_55Cycles5);
   ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 3, ADC_SampleTime_55Cycles5);

   // Interrupt DMA ADC
   NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
   DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);   // Interrupt ADC_BUFFER_SIZE

   /* Enable ADC1 DMA */
   ADC_DMACmd(ADC1, ENABLE);

   /* Enable ADC1 */
   ADC_Cmd(ADC1, ENABLE);

   /* Enable ADC1 reset calibration register */
   ADC_ResetCalibration(ADC1);
   /* Check the end of ADC1 reset calibration register */
   while (ADC_GetResetCalibrationStatus(ADC1))
      ;

   /* Start ADC1 calibration */
   ADC_StartCalibration(ADC1);
   /* Check the end of ADC1 calibration */
   while (ADC_GetCalibrationStatus(ADC1))
      ;

   /* Start ADC1 Software Conversion */
   ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

void DMA1_Channel1_IRQHandler(void)
{
   if (DMA_GetITStatus(DMA1_IT_TC1))   // Full 1/1
   {
      DMA_ClearITPendingBit(DMA1_IT_TC1);
   }

   if (DMA_GetITStatus(DMA1_IT_HT1))   // Half 1/2
   {
      DMA_ClearITPendingBit(DMA1_IT_HT1);
   }
}
