#define __ALLOCATE_NOW__

// User functions //
#include "includes.h"

// ID Task
xTaskHandle xHandleDebugTask;

int main()
{
   InitGPIO();

   tm1637Init();
   // Optionally set brightness. 0 is off. By default, initialized to full brightness.
   tm1637SetBrightness(3);
   // Display the value "1234" and turn on the `:` that is between digits 2 and 3.
   tm1637DisplayDecimal(1234, 1);
   while (1)
   {
   }
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
   unsigned long ulFreeSizeStackTask;   //свободное место в процессах RTOS.
   unsigned long ulFreeHeapSize;   //свободное место в "куче" RTOS
   uint8_t ucCPU_load;   //загрузка RTOS
   portTickType WakeTick = 0;
   uint32_t count = 0;
   uint32_t max_count = 0;   //максимальное значение счетчика, вычисляется при калибровке и соответствует 100% CPU idle

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
         if (count > max_count)
         {
            max_count = count;   //калибровка
         }
         ucCPU_load = (uint8_t)(100.0 - 100.0 * (float)count / (float)max_count);   //вычисляем текущую загрузку
         count = 0;   //обнуляем счетчик
      }
      if (WakeTick > xTaskGetTickCount() + configTICK_RATE_HZ << 1)
      {
         WakeTick = configTICK_RATE_HZ + xTaskGetTickCount();
      }
      count++;   //приращение счетчика
      IWDG_ReloadCounter();   // Reload IWDG counter
   }
}