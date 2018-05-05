#define __ALLOCATE_NOW__

// User functions //
#include "includes.h"

extern const unsigned char _acdevushkilyudi39547[];

void vOledTask(void* pvParameters)
{
   InitUSART(UART_DBG, DBG_BAUDRATE);
   InitDMA(UART_DBG);

   SSD1306_Init();
   /*
   SSD1306_GotoXY(30, 4);                                      //set cursor
   SSD1306_Puts("STM32F10x", &Font_7x10, SSD1306_COLOR_WHITE); //put string
   
   //SSD1306_UpdateScreen();
   SSD1306_GotoXY(50, 20);
   SSD1306_Puts("2017", &Font_7x10, SSD1306_COLOR_BLACK);
   //SSD1306_Puts("2017", &Font_7x10, SSD1306_COLOR_WHITE);
   //SSD1306_DrawFilledRectangle(50, 20, 27, 8, SSD1306_COLOR_WHITE); //draw white filled rectangle
   SSD1306_GotoXY(20, 44);
   SSD1306_Puts("Hello World!!", &Font_7x10, SSD1306_COLOR_WHITE);

   SSD1306_DrawCircle(10, 33, 7, SSD1306_COLOR_WHITE);

   SSD1306_UpdateScreen();
   */

   SSD1306_GotoXY(0, 0);
   int j = SSD1306_HEIGHT * SSD1306_WIDTH;
   for (int i = 0; i < SSD1306_WIDTH; i++)
   {
      for (int n = 0; n < SSD1306_HEIGHT; n++)
      {
         if (_acdevushkilyudi39547[j])
         {
            SSD1306_DrawPixel(i, n, SSD1306_COLOR_WHITE);
         }
         else
         {
            SSD1306_DrawPixel(i, n, SSD1306_COLOR_BLACK);
         }
         j--;
      }
   }
   SSD1306_UpdateScreen();

   while (1)
      ;
   while (1)
   {
      SSD1306_DrawFilledCircle(10, 33, 7, SSD1306_COLOR_WHITE);
      SSD1306_UpdateScreen();
      osDelay(500);
      SSD1306_DrawFilledCircle(10, 33, 7, SSD1306_COLOR_BLACK);
      SSD1306_UpdateScreen();
      osDelay(500);
   }
}

// ID Task
xTaskHandle xHandleOledTask;

int main()
{
   InitGPIO();
   // InitTIM3();
   // InitTIM4();

   // InitIWDG();    // Init Watch Dog
   InitBKP();

   rtc_init();

   // Start Task //
   xTaskCreate(vOledTask, "vOledTask", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 1, &xHandleOledTask);

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
         ulFreeSizeStackTask = uxTaskGetStackHighWaterMark(xHandleOledTask) << 2;
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