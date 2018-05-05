
#define __ALLOCATE_NOW__

// User functions //
#include "includes.h"

// ID Task
xTaskHandle xHandlevRadMonTask;
// xTaskHandle xHandleLcdTask;
// xTaskHandle xHandleGSM;
xTaskHandle xHandleWifiTask;
xTaskHandle xHandleOnewireTask;

int main()
{
   InitGPIO();

   InitIWDG();   // Init Watch Dog
   InitBKP();
   // rtc_init();

   /* Configure PA.00 in interrupt mode */
   EXTI0_Config();
   /* Generate software interrupt: simulate a falling edge applied on EXTI0 line */
   // EXTI_GenerateSWInterrupt(EXTI_Line0);

   InitGpioDRF();
   InitUSART(UART_DRF, DRF_BAUDRATE);
   InitDMA(UART_DRF);

   // Start Task //
   xTaskCreate(
      vRadMonTask, "vRadMonTask", configMINIMAL_STACK_SIZE * 4, NULL, tskIDLE_PRIORITY + 3, &xHandlevRadMonTask);
   xTaskCreate(
      vWifiEspTask, "vWifiEspTask", configMINIMAL_STACK_SIZE * 6, NULL, tskIDLE_PRIORITY + 2, &xHandleWifiTask);
   xTaskCreate(
      vOnewireTask, "vOnewireTask", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 1, &xHandleOnewireTask);

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