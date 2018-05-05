#define __ALLOCATE_NOW__

// User functions //
#include "includes.h"

// ID Task
xTaskHandle xHandleLcdTask;
xTaskHandle xHandleDebugTask;

int main()
{
   InitGPIO();

   InitBKP();

   Set_System();
   Set_USBClock();
   USB_Interrupts_Config();
   USB_Init();

   rtc_init();
   /*
   RTC_t date;
   date.year = 2015;
   date.month = 10;
   date.mday = 24;
   
   date.hour = 23;
   date.min = 20;
   date.sec = 0;
   rtc_settime(&date);
   */

   // Start Task //
   xTaskCreate(vLcdTask, "vLcdTask", configMINIMAL_STACK_SIZE * 1, NULL, tskIDLE_PRIORITY + 1, &xHandleLcdTask);
   xTaskCreate(vDebugTask, "vDebugTask", configMINIMAL_STACK_SIZE * 1, NULL, tskIDLE_PRIORITY + 1, &xHandleDebugTask);

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