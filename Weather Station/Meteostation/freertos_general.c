
#include "freertos_general.h"
#include "includes.h"

#define SIZE_STACK_ADC_TASK configMINIMAL_STACK_SIZE * 1
#define SIZE_STASK_OW_TASK configMINIMAL_STACK_SIZE * 1
#define SIZE_STASK_PRESSURE_TASK configMINIMAL_STACK_SIZE * 1
#define SIZE_STASK_HUMIDITY_TASK configMINIMAL_STACK_SIZE * 1
#define SIZE_STACK_DEBUG_TASK configMINIMAL_STACK_SIZE * 2
#define SIZE_STASK_IP_PERIODIC_TASK configMINIMAL_STACK_SIZE * 2
#define SIZE_STASK_IP_TASK configMINIMAL_STACK_SIZE * 2

#define SIZE_STASK_BUT_TASK configMINIMAL_STACK_SIZE * 1
#define SIZE_STASK_LCD_TASK configMINIMAL_STACK_SIZE * 2
#define SIZE_STACK_SD_CARD_TASK configMINIMAL_STACK_SIZE * 2

void vButTask(void* pvParameters)
{
   while (1)
   {
      _delay_ms(1000);
   }

   vTaskDelete(NULL);
}

void InitRTOS(void)
{
   // Start Semaphore //

   // Start Task //
   // xTaskCreate(vAdcTask, "vAdcTask", SIZE_STACK_ADC_TASK, NULL, tskIDLE_PRIORITY + 2, &xHandleAdcTask);
   // xTaskCreate(vOnewireTask, "vOnewireTask", SIZE_STASK_OW_TASK, NULL, tskIDLE_PRIORITY + 1, &xHandleOnewireTask);
   xTaskCreate(
      vPressureTask, "vPressureTask", SIZE_STASK_PRESSURE_TASK, NULL, tskIDLE_PRIORITY + 1, &xHandlePressureTask);
   xTaskCreate(
      vHumidityTask, "vHumidityTask", SIZE_STASK_HUMIDITY_TASK, NULL, tskIDLE_PRIORITY + 1, &xHandleHumidityTask);
   // xTaskCreate(vLcdTask, "vLcdTask", SIZE_STASK_LCD_TASK, NULL, tskIDLE_PRIORITY + 1, &xHandleLcdTask);
   // xTaskCreate(vSD_Card, "vSD_Card", SIZE_STACK_SD_CARD_TASK, NULL, tskIDLE_PRIORITY + 2, &xHandleSDCard);
   // xTaskCreate( vTask_uIP_periodic, "uIPp", SIZE_STASK_IP_PERIODIC_TASK, NULL, tskIDLE_PRIORITY + 2, ( xTaskHandle *
   // ) NULL); xTaskCreate( vTask_uIP, "uIP", SIZE_STASK_IP_TASK, NULL, tskIDLE_PRIORITY + 2, ( xTaskHandle * ) NULL);
   xTaskCreate(vDebugTask, "vDebugTask", SIZE_STACK_DEBUG_TASK, NULL, tskIDLE_PRIORITY + 1, &xHandleDebugTask);

   // Start scheduler //
   osKernelStart(NULL, NULL);
}

void vApplicationMallocFailedHook(void)
{
   for (;;)
      ;
}
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char* pcTaskName)
{
   for (;;)
      ;
}

void vApplicationIdleHook(void)
{   //это и есть поток Idle с минимальным приоритетом.

   while (1)
   {
   }
}