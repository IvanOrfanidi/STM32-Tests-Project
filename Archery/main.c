#define __ALLOCATE_NOW__

// User functions //
#include "includes.h"

#define DEF_SLEEP_DEVICE_TIMEOUT 15 * 60 * 10
#define DEF_SLEEP_DISPLAY_TIMEOUT 100
#define DEF_CHARGING_DISPLAY_TIMEOUT 50

// ID Task
xTaskHandle xHandleMainTask;
xTaskHandle xHandleAccelTask;

int g_usShot;

void IncreaseCountShot(void)
{
   g_usShot++;
   if (g_usShot > 9999)
      g_usShot = 0;
}

void vMainTask(void* pvParameters)
{
   portTickType xLastWakeTimerDelay;

   static uint32_t uiSleepDeviceTimeout = DEF_SLEEP_DEVICE_TIMEOUT;
   static uint16_t usSleepDisplayTimeout = DEF_SLEEP_DISPLAY_TIMEOUT;
   static uint16_t usChargingDisplayTimeout = DEF_CHARGING_DISPLAY_TIMEOUT;
   char strMsgChargingDisplay[4];
   uint8_t ucCountCol = 4;
   static int usBackShot = 0;   // предыдущий выстрел

   tm1637Init();
   // Optionally set brightness. 0 is off. By default, initialized to full brightness.
   tm1637SetBrightness(3);

   int ReadCountShot(void);
   g_usShot = ReadCountShot();   // текущий выстрел
   // g_usShot = 50;
   if (g_usShot > 0 && GET_BUT)
   {
      xLastWakeTimerDelay = xTaskGetTickCount();
      vTaskDelayUntil(&xLastWakeTimerDelay, (100 / portTICK_RATE_MS));
      if (GET_BUT)
         g_usShot--;

      while (GET_BUT)
      {
         tm1637DisplayDecimal(g_usShot, 0);
         void SaveCountShot(int count_shot);
         SaveCountShot(g_usShot);
         xLastWakeTimerDelay = xTaskGetTickCount();
         vTaskDelayUntil(&xLastWakeTimerDelay, (500 / portTICK_RATE_MS));
      }
   }

   while (1)
   {
      TYPE_CHARGING ChargingHandler(void);
      g_eStatusCharging = ChargingHandler();
      // g_eStatusCharging = CHARGING_ON;
      switch (g_eStatusCharging)
      {
      case CHARGING_COMPLET:
         if (!(g_stAccelData.bDataValid))
         {
            tm1637DisplayString("Err", 0);
         }
         else
         {
            tm1637DisplayString("END", 0);
         }
         usChargingDisplayTimeout = DEF_CHARGING_DISPLAY_TIMEOUT;
         break;

      case CHARGING_ON:
         if (usChargingDisplayTimeout)
         {
            usChargingDisplayTimeout--;
            if (!(g_stAccelData.bDataValid))
            {
               tm1637DisplayString("Err", 0);
            }
            else
            {
               tm1637DisplayDecimal(g_usShot, 0);
            }
            break;
         }
         ucCountCol++;
         if (ucCountCol >= sizeof(strMsgChargingDisplay))
            ucCountCol = 0;
         memset(strMsgChargingDisplay, 0, sizeof(strMsgChargingDisplay));
         strMsgChargingDisplay[ucCountCol] = '-';
         if (!(g_stAccelData.bDataValid))
         {
            tm1637DisplayString("Err", 0);
         }
         else
         {
            tm1637DisplayString(strMsgChargingDisplay, 0);
         }
         break;

      case CHARGING_OFF:
         usChargingDisplayTimeout = DEF_CHARGING_DISPLAY_TIMEOUT;
         if (usSleepDisplayTimeout)
         {
            if (!(g_stAccelData.bDataValid))
            {
               tm1637DisplayString("Err", 0);
            }
            else
            {
               tm1637DisplayDecimal(g_usShot, 0);
            }
         }
         else
         {
            tm1637DisplayOff();
         }
      }

      if (GET_BUT)
      {
         usSleepDisplayTimeout = DEF_SLEEP_DISPLAY_TIMEOUT;
         usChargingDisplayTimeout = DEF_CHARGING_DISPLAY_TIMEOUT;
         if (!(g_stAccelData.bDataValid))
         {
            tm1637DisplayString("Err", 0);
         }
         else
         {
            tm1637DisplayDecimal(g_usShot, 0);
         }
         xLastWakeTimerDelay = xTaskGetTickCount();
         vTaskDelayUntil(&xLastWakeTimerDelay, (200 / portTICK_RATE_MS));
         if (GET_BUT)
         {
            xLastWakeTimerDelay = xTaskGetTickCount();
            vTaskDelayUntil(&xLastWakeTimerDelay, (800 / portTICK_RATE_MS));
            if (GET_BUT)
               g_usShot = 0;
            if (!(g_stAccelData.bDataValid))
            {
               tm1637DisplayString("Err", 0);
            }
            else
            {
               tm1637DisplayDecimal(g_usShot, 0);
            }
         }
      }

      if (usBackShot == g_usShot)
      {
         if (g_eStatusCharging == CHARGING_OFF)
         {
            uiSleepDeviceTimeout--;
            if (!(uiSleepDeviceTimeout))
            {
               /* Sleep Device */
               uiSleepDeviceTimeout = DEF_SLEEP_DEVICE_TIMEOUT;
               usSleepDisplayTimeout = DEF_SLEEP_DISPLAY_TIMEOUT;
               USART_Write(UART_DBG, "-STOP APPL-\r\n", strlen("-STOP APPL-\r\n"));
               Accel_Clear_Settings();
               Accel_Sleep();
               tm1637DisplayOff();
               DeInitUSART(UART_DBG);
               DeInitDMA(UART_DBG);
               osDelay(250);
               SleepDevice();
            }
         }
         else
         {
            uiSleepDeviceTimeout = DEF_SLEEP_DEVICE_TIMEOUT;
            usSleepDisplayTimeout = DEF_SLEEP_DISPLAY_TIMEOUT;
         }
         if (usSleepDisplayTimeout)
         {
            usSleepDisplayTimeout--;
         }
      }
      else
      {
         void SaveCountShot(int count_shot);
         SaveCountShot(g_usShot);
         usBackShot = g_usShot;
         uiSleepDeviceTimeout = DEF_SLEEP_DEVICE_TIMEOUT;
         usSleepDisplayTimeout = DEF_SLEEP_DISPLAY_TIMEOUT;
      }

      xLastWakeTimerDelay = xTaskGetTickCount();
      vTaskDelayUntil(&xLastWakeTimerDelay, (10 / portTICK_RATE_MS));
   }
}

int main()
{
   InitGPIO();
   InitBKP();
#ifdef USE_BUZ
   InitTIM_BUZ();
#endif

   InitUSART(UART_DBG, DBG_BAUDRATE);
   InitDMA(UART_DBG);

   USART_Write(UART_DBG, "\r\n-RUN APPL-\r\n", strlen("-RUN APPL-\r\n"));

   // Init Task //
   xTaskCreate(vMainTask, "vMainTask", configMINIMAL_STACK_SIZE * 4, NULL, tskIDLE_PRIORITY + 1, &xHandleMainTask);
   xTaskCreate(vAccelTask, "vAccelTask", configMINIMAL_STACK_SIZE * 4, NULL, tskIDLE_PRIORITY + 2, &xHandleAccelTask);

   // Start scheduler //
   osKernelStart(NULL, NULL);

   while (1)
      ;
}

TYPE_CHARGING ChargingHandler(void)
{
   static TYPE_CHARGING brightness = CHARGING_ON;

   if (!(GPIO_ReadInputDataBit(CHARGING_GPIO_PORT, CHARGING_POWER_GPIO_PIN)))
   {
      tm1637SetBrightness(MAX_BRIGHTNESS);
      brightness = CHARGING_ON;
   }
   if (GPIO_ReadInputDataBit(CHARGING_GPIO_PORT, CHARGING_POWER_GPIO_PIN))
   {
      if (GPIO_ReadInputDataBit(CHARGING_GPIO_PORT, CHARGING_COMPLETED_GPIO_PIN))
      {
         tm1637SetBrightness(MIN_BRIGHTNESS);
         brightness = CHARGING_OFF;
      }
   }

   if (!(GPIO_ReadInputDataBit(CHARGING_GPIO_PORT, CHARGING_COMPLETED_GPIO_PIN)))
   {
      tm1637SetBrightness(MAX_BRIGHTNESS);
      brightness = CHARGING_COMPLET;
   }
   return brightness;
}

int ReadCountShot(void)
{
   return (int)BKP_ReadBackupRegister(BKP_DR2);
}

void SaveCountShot(int count_shot)
{
   BKP_WriteBackupRegister(BKP_DR2, (uint16_t)count_shot);
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
   portTickType WakeTick = xTaskGetTickCount() + configTICK_RATE_HZ;
   while (1)
   {
      if (xTaskGetTickCount() >= WakeTick)
      {
         /* Определяем загруженность OS */
         WakeTick += configTICK_RATE_HZ;
      }
      if (WakeTick > xTaskGetTickCount() + configTICK_RATE_HZ << 1)
      {
         WakeTick = configTICK_RATE_HZ + xTaskGetTickCount();
      }
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