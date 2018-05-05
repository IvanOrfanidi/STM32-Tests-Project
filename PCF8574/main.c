#define __ALLOCATE_NOW__

// User functions //
#include "includes.h"

// ID Task
xTaskHandle xHandleDebugTask;
xTaskHandle xHandlevLcdPcfTask;

void vLcdPcf(void* pvParameters);

int main()
{
   InitGPIO();
   // InitTIM3();
   // InitTIM4();

   InitIWDG();   // Init Watch Dog
   InitBKP();

#ifdef DEBUG_OUTPUT_USB
   Set_System();
   Set_USBClock();
   USB_Interrupts_Config();
   USB_Init();
#else
   rtc_init();
#endif

   RTC_t date;
   date.year = 2015;
   date.month = 10;
   date.mday = 24;

   date.hour = 23;
   date.min = 20;
   date.sec = 0;
   rtc_settime(&date);

   // Start Task //
   xTaskCreate(vLcdPcf, "vLcdPcf", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 1, &xHandlevLcdPcfTask);
   xTaskCreate(vDebugTask, "vDebugTask", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 1, &xHandleDebugTask);

   // Start scheduler //
   osKernelStart(NULL, NULL);
}

void vLcdPcf(void* pvParameters)
{
   char strMsgDebug[64];
   RTC_t stDate;

   PCF8574_I2C_Init();
   LcdPcf_Init();

   LcdPcf_Backlight(1);

   while (1)
   {
      rtc_gettime(&stDate);
      sprintf(strMsgDebug, "%02d/%02d/%02d  ", stDate.mday, stDate.month, stDate.year);

      LcdPcf_Goto(1, 0);
      prit_lcd(strMsgDebug);

      sprintf(strMsgDebug, "%02d:%02d:%02d", stDate.hour, stDate.min, stDate.sec);
      LcdPcf_Goto(2, 0);
      prit_lcd(strMsgDebug);
      osDelay(100);
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