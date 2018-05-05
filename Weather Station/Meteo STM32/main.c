#define __ALLOCATE_NOW__

// User functions //
#include "includes.h"

// ID Task
xTaskHandle xHandleLcdTask;
xTaskHandle xHandlevPressureTask;
xTaskHandle xHandleExtHumidityTask;
xTaskHandle xHandleIntHumidityTask;
xTaskHandle xHandleGpsTask;
xTaskHandle xHandleButtonTask;

int main(void)
{
   InitGPIO();
   InitIWDG();   // Init Watch Dog
   InitBKP();

   sTIMET1_CCR1_Val = BKP_ReadBackupRegister(BKP_DR2);
   if (sTIMET1_CCR1_Val > TIM3_PERIOD || sTIMET1_CCR1_Val == NULL)
   {
      sTIMET1_CCR1_Val = 700;
   }
   InitTIM3();

   mINIT_WIFI_ESP = osMutexCreate(NULL);
   mSEND_DATA_SERVER = osMutexCreate(NULL);
   mGPS_DATA_ARRIVAL = osMutexCreate(NULL);
   mLCD_LED_INDICATING = osMutexCreate(NULL);
   mLCD_GPS_INDICATING = osMutexCreate(NULL);

   xSemaphoreTake(mINIT_WIFI_ESP, SLEEP_MS_100);
   xSemaphoreTake(mSEND_DATA_SERVER, SLEEP_MS_100);
   xSemaphoreTake(mGPS_DATA_ARRIVAL, SLEEP_MS_100);
   xSemaphoreTake(mLCD_LED_INDICATING, SLEEP_MS_100);
   xSemaphoreTake(mLCD_GPS_INDICATING, SLEEP_MS_100);

   // Start Task //
   xTaskCreate(vLcdTask, "vLcdTask", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 2, &xHandleLcdTask);
   xTaskCreate(
      vPressureTask, "vPressureTask", configMINIMAL_STACK_SIZE * 1, NULL, tskIDLE_PRIORITY + 1, &xHandlevPressureTask);
   xTaskCreate(vExtHumidityTask,
               "vExtHumidityTask",
               configMINIMAL_STACK_SIZE * 1,
               NULL,
               tskIDLE_PRIORITY + 1,
               &xHandleExtHumidityTask);
   xTaskCreate(vIntHumidityTask,
               "vIntHumidityTask",
               configMINIMAL_STACK_SIZE * 1,
               NULL,
               tskIDLE_PRIORITY + 1,
               &xHandleIntHumidityTask);
   xTaskCreate(vGpsHandler, "vGpsHandler", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 1, &xHandleGpsTask);
   xTaskCreate(
      vButtonTask, "vButtonTask", configMINIMAL_STACK_SIZE * 1, NULL, tskIDLE_PRIORITY + 1, &xHandleButtonTask);

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
      IWDG_ReloadCounter();   // Reload IWDG counter.
   }
}