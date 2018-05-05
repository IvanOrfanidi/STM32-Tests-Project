#define __ALLOCATE_NOW__

// User functions //
#include "includes.h"

/* Пример взят https://github.com/Harinadha/STM32_HMC5883Llib */

// ID Task
xTaskHandle xHandleDebugTask;
xTaskHandle xHandleCompassTask;

int main()
{
   InitGPIO();

   InitBKP();

   Set_System();
   Set_USBClock();
   USB_Interrupts_Config();
   USB_Init();

   // Start Task //
   xTaskCreate(
      vCompassTask, "vCompassTask", configMINIMAL_STACK_SIZE * 1, NULL, tskIDLE_PRIORITY + 1, &xHandleCompassTask);
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