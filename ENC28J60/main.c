#define __ALLOCATE_NOW__

// User functions //
#include "includes.h"

// ID Task
xTaskHandle xHandleDebugTask;

int main()
{
   InitGPIO();
   // InitTIM3();
   // InitTIM4();

   // InitIWDG();    // Init Watch Dog
   InitBKP();

#ifdef DEBUG_OUTPUT_USB
   Set_System();
   Set_USBClock();
   USB_Interrupts_Config();
   USB_Init();
#else
   rtc_init();
#endif

   // Start Task //
   xTaskCreate(
      vTask_uIP_periodic, "uIPp", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 2, (xTaskHandle*)NULL);
   xTaskCreate(vTask_uIP, "uIP", configMINIMAL_STACK_SIZE * 4, NULL, tskIDLE_PRIORITY + 2, (xTaskHandle*)NULL);

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
{
   volatile unsigned long ulFreeSizeStackTask;   //свободное место в процессах RTOS.
   volatile unsigned long ulFreeHeapSize;   //свободное место в "куче" RTOS
   portTickType WakeTick = 0;
   WakeTick = xTaskGetTickCount() + configTICK_RATE_HZ;
   while (1)
   {
      if (xTaskGetTickCount() >= WakeTick)
      {
#if (INCLUDE_uxTaskGetStackHighWaterMark == 1)
         ulFreeSizeStackTask = uxTaskGetStackHighWaterMark(xHandleDebugTask) << 2;
         ulFreeHeapSize = (unsigned long)xPortGetFreeHeapSize();   // in Byte
#endif

         /* Определяем загруженность OS */
         WakeTick += configTICK_RATE_HZ;
      }
      if (WakeTick > xTaskGetTickCount() + configTICK_RATE_HZ << 1)
      {
         WakeTick = configTICK_RATE_HZ + xTaskGetTickCount();
      }
      IWDG_ReloadCounter();   // Reload IWDG counter
   }
}

#ifdef USE_FULL_ASSERT

/**
 * @brief  Reports the name of the source file and the source line number
 *   where the assert_param error has occurred.
 * @param file: pointer to the source file name
 * @param line: assert_param error line source number
 * @retval : None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
   /* User can add his own implementation to report the file name and line number,
      ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

   /* Infinite loop */
   while (1)
   {
   }
}
#endif