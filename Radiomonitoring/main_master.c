#define __ALLOCATE_NOW__

// User functions //
#include "includes.h"

/* Protocol: from Slave to Master
1 - 0x02 - Device TX (0x01 - Master, 0x02 - Slave)
2 - 0x01 - Device RX (0x01 - Master, 0x02 - Slave)
3 - 0x01 - Function (0x01 - Diagnostic, 0x02 - Notice)
4 - 0x00 - Value
5 - 0xFF - CRC8
*/
// const uint8_t g_aucProt[5] = {0x02, 0x01, 0x01, 0x00, 0xFF};      //
/* Protocol: from Master to Slave
1 - 0x01 - Device TX (0x01 - Master, 0x02 - Slave)
2 - 0x02 - Device RX (0x01 - Master, 0x02 - Slave)
3 - 0x01 - Function (0x01 - Diagnostic, 0x02 - Notice)
4 - 0x00 - Value (0x00 - OK)
5 - 0xFF - CRC8
*/

/* ---------- SETTING DEVICE -------------------------- */
/* Slave: 0x02, 0x01, 0x00, 0x00, 0xFF - Test Device */
/* Master: 0x01, 0x02, 0x00, 0x00, 0xFF - Test Device OK */

/* ---------- ERROR ACCEL -------------------------- */
/* Slave: 0x02, 0x01, 0x01, 0x01, 0xFF - Error Accel */
/* Master: 0x01, 0x02, 0x01, 0x00, 0xFF - Error Accel OK */

/* ---------- ALARM -------------------------- */
/* Slave: 0x02, 0x01, 0x02, 0x01, 0xFF - Alarm */
/* Master: 0x01, 0x02, 0x02, 0x00, 0xFF - Alarm OK */

#define TIMEOUT_TX_DISPLAY 2000
#define TIMEOUT_RX_DISPLAY 2000

#define TIMEOUT_SLAVE_PERQ 1000
#define COUNT_SLAVE_FAIL 3

#define TIMEOUT_RESPONSE 3000
#define TIMEOUT_SLEEP 100
// ID Task
xTaskHandle xHandleMainTask;
xTaskHandle xHandleDisplayTask;
xTaskHandle xHandleBuzzerTask;

TDrf_Settings g_stDrfSettings;

void vMainTask(void* pvParameters);
void vDisplayTask(void* pvParameters);
void vBuzzerTask(void* pvParameters);

void DeviceOk(void);
void AccelFail(void);
void SlaveFail(void);
void DeviceSLEEP(void);
void Alarm(void);

int main()
{
   InitGPIO();
   InitBKP();
   // rtc_init();

   // tm1637Init();
   // Optionally set brightness. 0 is off. By default, initialized to full brightness.
   // tm1637SetBrightness(3);
   // tm1637DisplayString("StAr", 0);

   // Создаём очередь данных для процесса дисплея.
   xQueueDisplayData = xQueueCreate(QUEUE_LENGTH, sizeof(TDisplayData));
   if (xQueueDisplayData == NULL)
   {
      goto BEGIN_RTOS_FAIL;
   }
   vQueueAddToRegistry(xQueueDisplayData, "xQueueDisplayData");

   xQueueBuzzer = xQueueCreate(QUEUE_LENGTH, sizeof(TBuzzerData));
   if (xQueueBuzzer == NULL)
   {
      goto BEGIN_RTOS_FAIL;
   }
   vQueueAddToRegistry(xQueueBuzzer, "xQueueBuzzer");

   // Создаем мьютекс готовности дисплея
   mDISPLAY_COMPLETED = osMutexCreate(NULL);
   xSemaphoreTake(mDISPLAY_COMPLETED, 0);

   // Start Task //
   if (xTaskCreate(
          vMainTask, "vMainTask", configMINIMAL_STACK_SIZE * 4, NULL, tskIDLE_PRIORITY + 2, &xHandleMainTask) != pdTRUE)
   {
      goto BEGIN_RTOS_FAIL;
   }
   if (xTaskCreate(vDisplayTask,
                   "vDisplayTask",
                   configMINIMAL_STACK_SIZE * 2,
                   NULL,
                   tskIDLE_PRIORITY + 1,
                   &xHandleDisplayTask) != pdTRUE)
   {
      goto BEGIN_RTOS_FAIL;
   }
   if (xTaskCreate(
          vBuzzerTask, "vBuzzerTask", configMINIMAL_STACK_SIZE * 1, NULL, tskIDLE_PRIORITY + 1, &xHandleBuzzerTask) !=
       pdTRUE)
   {
      goto BEGIN_RTOS_FAIL;
   }
   // Start scheduler //
   osKernelStart(NULL, NULL);

BEGIN_RTOS_FAIL:
   while (1)
      ;
}

void vMainTask(void* pvParameters)
{
   portTickType xLastWakeTimerDelay;
   _Bool bDeviceActive = FALSE;
   static _Bool bDevSleep = FALSE;
   char strResBuff[DRF_RX_BUFFER_SIZE];   //Буфер куда будем все читать.
   volatile uint32_t uiTimerSlaveDevicePerq = TIMEOUT_SLAVE_PERQ;   //Счетчик опроса Slave девайcа
   uint8_t ucCountSlaveDeviceFail = COUNT_SLAVE_FAIL;

   TDisplayData stDisplayData;
   strcpy(stDisplayData.strMsg, "StAr");
   stDisplayData.displaySeparator = 0;
   stDisplayData.brightness = 3;
   stDisplayData.uiDelay = TIMEOUT_TX_DISPLAY << 1;
   xQueueSendToFront(xQueueDisplayData, &stDisplayData, portMAX_DELAY);
   TBuzzerData stBuzzerData;
   stBuzzerData.uiTimeBuzOn = 100;
   stBuzzerData.uiTimeBuzOff = 200;
   xQueueSendToFront(xQueueBuzzer, &stBuzzerData, portMAX_DELAY);
   stBuzzerData.uiTimeBuzOn = 100;
   stBuzzerData.uiTimeBuzOff = 200;
   xQueueSendToFront(xQueueBuzzer, &stBuzzerData, portMAX_DELAY);

   InitGpioDRF();
   InitUSART(UART_DRF, DRF_BAUDRATE);
   InitDMA(UART_DRF);

   //Инит структуры конфигурации радиомодема
   TDrf_Settings stDrfSettings;
   stDrfSettings.uiFreq = 454130;
   stDrfSettings.eDRfsk = DRfsk_2400bps;
   stDrfSettings.ecPout = Pout_20dBm;
   stDrfSettings.eDRin = DRin_2400bps;
   stDrfSettings.eParity = NO_PARITY;

   _Bool bInitConfigDrf = 0;
   /* Читаем конфиг радиомодема */
   if (GetConfigDRF(&g_stDrfSettings, DRF_BAUDRATE))
   {
      bInitConfigDrf = 1;
   }
   else
   {
      /* Сравниваем конфиг радиомодема с заданным */
      uint8_t* ptr1 = (uint8_t*)&stDrfSettings;
      uint8_t* ptr2 = (uint8_t*)&g_stDrfSettings;
      for (int i = 0; i < sizeof(TDrf_Settings); i++)
      {
         if (ptr1[i] != ptr2[i])
         {
            bInitConfigDrf = 1;
            break;
         }
      }
   }

   strcpy(stDisplayData.strMsg, "Find");
   stDisplayData.displaySeparator = 0;
   stDisplayData.brightness = 3;
   stDisplayData.uiDelay = TIMEOUT_TX_DISPLAY;
   xQueueSendToFront(xQueueDisplayData, &stDisplayData, portMAX_DELAY);

   /* Устанавливаем заданный конфиг если нужно */
   if (bInitConfigDrf)
   {
      if (!(SetConfigDRF(&stDrfSettings, DRF_BAUDRATE)))
      {
         g_stDrfSettings = stDrfSettings;
      }
   }

   /* Включаем радио, чтобы его слушать */
   SetRadioEnable(1);

   while (1)
   {
      /* Шлем приветствие чтобы slave проснулся */
      if (bDeviceActive == FALSE)
      {
         uiTimerSlaveDevicePerq = TIMEOUT_SLAVE_PERQ;
         if (ucCountSlaveDeviceFail)
            ucCountSlaveDeviceFail--;
         while (1)
            if (osMutexWait(mDISPLAY_COMPLETED, 0) == osOK)
               break;
         strcpy(stDisplayData.strMsg, "Find");
         stDisplayData.displaySeparator = 0;
         stDisplayData.brightness = 3;
         stDisplayData.uiDelay = TIMEOUT_TX_DISPLAY;
         xQueueSendToFront(xQueueDisplayData, &stDisplayData, portMAX_DELAY);
         stBuzzerData.uiTimeBuzOn = 100;
         stBuzzerData.uiTimeBuzOff = 0;
         xQueueSendToFront(xQueueBuzzer, &stBuzzerData, portMAX_DELAY);

         char aucProt[5] = { MASTER, SLAVE, SETTING, OK, 0xFF };
         aucProt[CRC8] = _crc8_update(aucProt, CRC8);   //Добавляем CRC
         USART_Write(UART_DRF, aucProt, sizeof(aucProt));   //Отправляем OK
         xLastWakeTimerDelay = xTaskGetTickCount();
         vTaskDelayUntil(&xLastWakeTimerDelay, (2500 / portTICK_RATE_MS));
         tm1637DisplayOff();
      }

      if (!(uiTimerSlaveDevicePerq))
      {
         while (1)
            if (osMutexWait(mDISPLAY_COMPLETED, 0) == osOK)
               break;
         strcpy(stDisplayData.strMsg, "qUES");
         stDisplayData.displaySeparator = 0;
         stDisplayData.brightness = 3;
         stDisplayData.uiDelay = TIMEOUT_TX_DISPLAY;
         xQueueSendToFront(xQueueDisplayData, &stDisplayData, portMAX_DELAY);

         char aucProt[5] = { MASTER, SLAVE, SETTING, OK, 0xFF };
         aucProt[CRC8] = _crc8_update(aucProt, CRC8);   //Добавляем CRC
         USART_Write(UART_DRF, aucProt, sizeof(aucProt));   //Отправляем OK
         xLastWakeTimerDelay = xTaskGetTickCount();
         vTaskDelayUntil(&xLastWakeTimerDelay, (2500 / portTICK_RATE_MS));
         tm1637DisplayOff();
         if (ucCountSlaveDeviceFail)
            ucCountSlaveDeviceFail--;
      }
      else
      {
         uiTimerSlaveDevicePerq--;
      }

      uint8_t ucRxLen = NULL;
      if (USART_Rx_Len(UART_DRF) != 0)
      {
         uint16_t len = USART_Rx_Len(UART_DRF);
         while (len != USART_Rx_Len(UART_DRF))
         {
            len = USART_Rx_Len(UART_DRF);
            xLastWakeTimerDelay = xTaskGetTickCount();
            vTaskDelayUntil(&xLastWakeTimerDelay, (1 / portTICK_RATE_MS));
         }
         ucRxLen = USART_Rx_Len(UART_DRF);
      }
      else
      {
         ReStartDmaUsart2();
      }

      if (ucRxLen != NULL)
      {   //Что то пришло, парсим это.
         memset(strResBuff, 0, sizeof(strResBuff));
         USART_Read(UART_DRF, strResBuff, sizeof(strResBuff));
         ReStartDmaUsart2();
         uint8_t crc = _crc8_update(&strResBuff[0], CRC8);
         if ((strResBuff[DEVICE_RX] == MASTER) &&
             (strResBuff[CRC8] == crc))   //Если это адресовано мне(мастеру) и совпала CRC, то парсю дальше.
         {
            if (strResBuff[DEVICE_TX] == SLAVE)   //Это от второго slave
            {
               _Bool bDeviceOK = FALSE;
               if (strResBuff[FUNCTION] == SETTING)   //Это настройки
               {
                  switch (strResBuff[VALUE])
                  {
                  case OK:
                     bDeviceOK = TRUE;
                     break;
                  }

                  /* Если инициализация, то не отвечаем на это сообщение и пищим */
                  if (bDeviceActive == FALSE)
                  {
                     bDeviceActive = TRUE;
                     TBuzzerData stBuzzerData;
                     stBuzzerData.uiTimeBuzOn = 100;
                     stBuzzerData.uiTimeBuzOff = 100;
                     xQueueSendToFront(xQueueBuzzer, &stBuzzerData, portMAX_DELAY);
                     DeviceOk();
                     while (1)
                        if (osMutexWait(mDISPLAY_COMPLETED, 100) == osOK)
                           break;
                     strcpy(stDisplayData.strMsg, "acti");
                     stDisplayData.displaySeparator = 0;
                     stDisplayData.brightness = 3;
                     stDisplayData.uiDelay = TIMEOUT_RX_DISPLAY;
                     xQueueSendToFront(xQueueDisplayData, &stDisplayData, portMAX_DELAY);
                  }
                  else
                  {
                     if (!(uiTimerSlaveDevicePerq))
                     {
                        uiTimerSlaveDevicePerq = TIMEOUT_SLAVE_PERQ;
                        ucCountSlaveDeviceFail =
                           COUNT_SLAVE_FAIL;   //Сбрасываем таймер попыток, девайс на связи, все ОК).
                     }
                     else
                     {
                        /* Ответ что получили послание */
                        char aucProt[5] = { MASTER, SLAVE, SETTING, OK, 0xFF };
                        aucProt[CRC8] = _crc8_update(aucProt, CRC8);   //Добавляем CRC
                        USART_Write(UART_DRF, aucProt, sizeof(aucProt));   //Отправляем OK
                     }

                     if (bDeviceOK == TRUE && bDevSleep == FALSE)
                     {
                        DeviceOk();
                     }
                     if (bDevSleep)
                     {
                        while (1)
                           if (osMutexWait(mDISPLAY_COMPLETED, 100) == osOK)
                              break;
                        strcpy(stDisplayData.strMsg, "AnS");
                        stDisplayData.displaySeparator = 0;
                        stDisplayData.brightness = 3;
                        stDisplayData.uiDelay = TIMEOUT_RX_DISPLAY;
                        xQueueSendToFront(xQueueDisplayData, &stDisplayData, portMAX_DELAY);
                        TBuzzerData stBuzzerData;
                        stBuzzerData.uiTimeBuzOn = 100;
                        stBuzzerData.uiTimeBuzOff = 100;
                        xQueueSendToFront(xQueueBuzzer, &stBuzzerData, portMAX_DELAY);
                        DeviceSLEEP();
                     }
                  }
               }

               if (strResBuff[FUNCTION] == DIAGNOSTICS)   // Это диагностика
               {
                  _Bool bAccelFail = FALSE;
                  switch (strResBuff[VALUE])
                  {
                  case OK:
                     break;
                  case ACCEL_FAIL:
                     bAccelFail = TRUE;
                     break;
                  }

                  if (bAccelFail)
                  {
                     AccelFail();
                  }
               }

               if (strResBuff[FUNCTION] == NOTICE)   // Это тревога
               {
                  _Bool bAlarm = FALSE;
                  switch (strResBuff[VALUE])
                  {
                  case ALARM:
                     bAlarm = TRUE;   //Тревога, тревога, Волк унес зайчат...
                     break;
                  }

                  /* Ответ что получили послание */
                  char aucProt[5] = { MASTER, SLAVE, NOTICE, OK, 0xFF };
                  aucProt[CRC8] = _crc8_update(aucProt, CRC8);   //Добавляем CRC
                  USART_Write(UART_DRF, aucProt, sizeof(aucProt));   //Отправляем OK

                  if (bAlarm)
                  {
                     Alarm();
                  }
               }
            }
         }
      }

      if (GET_BUT)
      {
         strcpy(stDisplayData.strMsg, "qUES");
         stDisplayData.displaySeparator = 0;
         stDisplayData.brightness = 3;
         stDisplayData.uiDelay = TIMEOUT_TX_DISPLAY;
         xQueueSendToFront(xQueueDisplayData, &stDisplayData, portMAX_DELAY);
         stBuzzerData.uiTimeBuzOn = 100;
         stBuzzerData.uiTimeBuzOff = 0;
         xQueueSendToFront(xQueueBuzzer, &stBuzzerData, portMAX_DELAY);

         bDevSleep = TRUE;
         char aucProt[5] = { MASTER, SLAVE, SETTING, SLEEP, 0xFF };
         aucProt[CRC8] = _crc8_update(aucProt, CRC8);   //Добавляем CRC
         USART_Write(UART_DRF, aucProt, sizeof(aucProt));   //Отправляем OK
         xLastWakeTimerDelay = xTaskGetTickCount();
         vTaskDelayUntil(&xLastWakeTimerDelay, (2500 / portTICK_RATE_MS));
         tm1637DisplayOff();
      }

      if (!(ucCountSlaveDeviceFail))
      {   // Slave отвалился
         SlaveFail();
      }

      if (bDeviceActive == FALSE)
      {
         static int timeout_sleep = 0;
         timeout_sleep++;
         if (timeout_sleep > TIMEOUT_SLEEP)
         {
            DeviceSLEEP();
         }
      }

      xLastWakeTimerDelay = xTaskGetTickCount();
      vTaskDelayUntil(&xLastWakeTimerDelay, (SLEEP_MS_100 / portTICK_RATE_MS));
   }
}

void vDisplayTask(void* pvParameters)
{
   tm1637Init();
   // Optionally set brightness. 0 is off. By default, initialized to full brightness.
   char brightness = 3;
   tm1637SetBrightness(brightness);

   TDisplayData stDisplayData;
   while (1)
   {
      if (xQueueReceive(xQueueDisplayData, &stDisplayData, (portTickType)100) != pdFAIL)
      {
         while (xQueueReceive(xQueueDisplayData, &stDisplayData, (portTickType)0) != pdFAIL)
            ;
         if (stDisplayData.brightness != brightness)
         {
            brightness = stDisplayData.brightness;
            tm1637SetBrightness(brightness);
         }
         tm1637DisplayString(stDisplayData.strMsg, stDisplayData.displaySeparator);
         if (stDisplayData.uiDelay != portMAX_DELAY)
         {
            portTickType xLastWakeTimerDelay = xTaskGetTickCount();
            vTaskDelayUntil(&xLastWakeTimerDelay, (portTickType)(stDisplayData.uiDelay / portTICK_RATE_MS));
            tm1637DisplayOff();
         }
         if (osMutexWait(mDISPLAY_COMPLETED, 0) == osOK)
         {
            xSemaphoreTake(mDISPLAY_COMPLETED, 0);
         }
         xSemaphoreGive(mDISPLAY_COMPLETED);
      }
   }
}

void vBuzzerTask(void* pvParameters)
{
   portTickType xLastWakeTimerDelay;
   TBuzzerData stBuzzerData;

   while (1)
   {
      if (xQueueReceive(xQueueBuzzer, &stBuzzerData, (portTickType)100) != pdFAIL)
      {
         while (xQueueReceive(xQueueBuzzer, &stBuzzerData, (portTickType)0) != pdFAIL)
            ;
         BUZ_ON;
         xLastWakeTimerDelay = xTaskGetTickCount();
         vTaskDelayUntil(&xLastWakeTimerDelay, (portTickType)(stBuzzerData.uiTimeBuzOn / portTICK_RATE_MS));
         BUZ_OFF;
         if (stBuzzerData.uiTimeBuzOff)
         {
            xLastWakeTimerDelay = xTaskGetTickCount();
            vTaskDelayUntil(&xLastWakeTimerDelay, (portTickType)(stBuzzerData.uiTimeBuzOff / portTICK_RATE_MS));
            BUZ_ON;
            xLastWakeTimerDelay = xTaskGetTickCount();
            vTaskDelayUntil(&xLastWakeTimerDelay, (portTickType)(stBuzzerData.uiTimeBuzOn / portTICK_RATE_MS));
            BUZ_OFF;
         }
      }
   }
}

/* Обработчик тревог и аварий */
void Alarm(void)
{
   TDisplayData stDisplayData;
   TBuzzerData stBuzzerData;
   portTickType xLastWakeTimerDelay;

   /* Отправляем Slave потверждение */
   char aucProt[5] = { MASTER, SLAVE, NOTICE, OK, 0xFF };
   aucProt[CRC8] = _crc8_update(aucProt, CRC8);   //Добавляем CRC
   USART_Write(UART_DRF, aucProt, sizeof(aucProt));   //Отправляем OK

   while (1)
      if (osMutexWait(mDISPLAY_COMPLETED, 100) == osOK)
         break;
   strcpy(stDisplayData.strMsg, "ALAr");
   stDisplayData.displaySeparator = 0;
   stDisplayData.brightness = 3;
   stDisplayData.uiDelay = portMAX_DELAY;
   xQueueSendToFront(xQueueDisplayData, &stDisplayData, portMAX_DELAY);

   while (!(GET_BUT))
   {
      stBuzzerData.uiTimeBuzOn = 200;
      stBuzzerData.uiTimeBuzOff = 100;
      xQueueSendToFront(xQueueBuzzer, &stBuzzerData, portMAX_DELAY);

      xLastWakeTimerDelay = xTaskGetTickCount();
      vTaskDelayUntil(&xLastWakeTimerDelay, (100 / portTICK_RATE_MS));
   }
   BUZ_OFF;
   strcpy(stDisplayData.strMsg, "SLEP");
   stDisplayData.displaySeparator = 0;
   stDisplayData.brightness = 3;
   stDisplayData.uiDelay = portMAX_DELAY;
   xQueueSendToFront(xQueueDisplayData, &stDisplayData, portMAX_DELAY);

   xLastWakeTimerDelay = xTaskGetTickCount();
   vTaskDelayUntil(&xLastWakeTimerDelay, (TIMEOUT_TX_DISPLAY << 1 / portTICK_RATE_MS));

   BUZ_OFF;
   tm1637DisplayOff();
   SleepMasterDevice();
}

void SlaveFail(void)
{
   TDisplayData stDisplayData;
   TBuzzerData stBuzzerData;
   portTickType xLastWakeTimerDelay;

   while (1)
      if (osMutexWait(mDISPLAY_COMPLETED, 100) == osOK)
         break;
   strcpy(stDisplayData.strMsg, "LOST");
   stDisplayData.displaySeparator = 0;
   stDisplayData.brightness = 3;
   stDisplayData.uiDelay = portMAX_DELAY;
   xQueueSendToFront(xQueueDisplayData, &stDisplayData, portMAX_DELAY);

   while (!(GET_BUT))
   {
      stBuzzerData.uiTimeBuzOn = 200;
      stBuzzerData.uiTimeBuzOff = 100;
      xQueueSendToFront(xQueueBuzzer, &stBuzzerData, portMAX_DELAY);

      xLastWakeTimerDelay = xTaskGetTickCount();
      vTaskDelayUntil(&xLastWakeTimerDelay, (100 / portTICK_RATE_MS));
   }
   BUZ_OFF;
   strcpy(stDisplayData.strMsg, "SLEP");
   stDisplayData.displaySeparator = 0;
   stDisplayData.brightness = 3;
   stDisplayData.uiDelay = portMAX_DELAY;
   xQueueSendToFront(xQueueDisplayData, &stDisplayData, portMAX_DELAY);

   xLastWakeTimerDelay = xTaskGetTickCount();
   vTaskDelayUntil(&xLastWakeTimerDelay, (TIMEOUT_TX_DISPLAY << 1 / portTICK_RATE_MS));

   BUZ_OFF;
   tm1637DisplayOff();
   SleepMasterDevice();
}

void DeviceOk(void)
{
   TDisplayData stDisplayData;
   while (1)
      if (osMutexWait(mDISPLAY_COMPLETED, 100) == osOK)
         break;
   strcpy(stDisplayData.strMsg, "AnS");
   stDisplayData.displaySeparator = 0;
   stDisplayData.brightness = 3;
   stDisplayData.uiDelay = TIMEOUT_RX_DISPLAY;
   xQueueSendToFront(xQueueDisplayData, &stDisplayData, portMAX_DELAY);
}

void DeviceSLEEP(void)
{
   TDisplayData stDisplayData;
   TBuzzerData stBuzzerData;
   while (1)
      if (osMutexWait(mDISPLAY_COMPLETED, 100) == osOK)
         break;
   strcpy(stDisplayData.strMsg, "SLEP");
   stDisplayData.displaySeparator = 0;
   stDisplayData.brightness = 3;
   stDisplayData.uiDelay = TIMEOUT_TX_DISPLAY << 1;
   xQueueSendToFront(xQueueDisplayData, &stDisplayData, portMAX_DELAY);
   stBuzzerData.uiTimeBuzOn = 100;
   stBuzzerData.uiTimeBuzOff = 200;
   xQueueSendToFront(xQueueBuzzer, &stBuzzerData, portMAX_DELAY);
   stBuzzerData.uiTimeBuzOn = 100;
   stBuzzerData.uiTimeBuzOff = 200;
   xQueueSendToFront(xQueueBuzzer, &stBuzzerData, portMAX_DELAY);

   while (1)
      if (osMutexWait(mDISPLAY_COMPLETED, 100) == osOK)
         break;
   tm1637DisplayOff();
   SleepMasterDevice();
}

void AccelFail(void)
{
   TDisplayData stDisplayData;
   TBuzzerData stBuzzerData;
   portTickType xLastWakeTimerDelay;

   /* Отправляем Slave потверждение */
   char aucProt[5] = { MASTER, SLAVE, DIAGNOSTICS, OK, 0xFF };
   aucProt[CRC8] = _crc8_update(aucProt, CRC8);   //Добавляем CRC
   USART_Write(UART_DRF, aucProt, sizeof(aucProt));   //Отправляем OK

   while (1)
      if (osMutexWait(mDISPLAY_COMPLETED, 100) == osOK)
         break;
   strcpy(stDisplayData.strMsg, "ACEL");
   stDisplayData.displaySeparator = 0;
   stDisplayData.brightness = 3;
   stDisplayData.uiDelay = portMAX_DELAY;
   xQueueSendToFront(xQueueDisplayData, &stDisplayData, portMAX_DELAY);

   while (!(GET_BUT))
   {
      stBuzzerData.uiTimeBuzOn = 200;
      stBuzzerData.uiTimeBuzOff = 100;
      xQueueSendToFront(xQueueBuzzer, &stBuzzerData, portMAX_DELAY);

      xLastWakeTimerDelay = xTaskGetTickCount();
      vTaskDelayUntil(&xLastWakeTimerDelay, (100 / portTICK_RATE_MS));
   }
   BUZ_OFF;
   strcpy(stDisplayData.strMsg, "SLEP");
   stDisplayData.displaySeparator = 0;
   stDisplayData.brightness = 3;
   stDisplayData.uiDelay = portMAX_DELAY;
   xQueueSendToFront(xQueueDisplayData, &stDisplayData, portMAX_DELAY);

   xLastWakeTimerDelay = xTaskGetTickCount();
   vTaskDelayUntil(&xLastWakeTimerDelay, (TIMEOUT_TX_DISPLAY << 1 / portTICK_RATE_MS));

   BUZ_OFF;
   tm1637DisplayOff();
   SleepMasterDevice();
}

uint16_t ReadCountWakeUp(void)
{
   return BKP_ReadBackupRegister(BKP_DR2);
}

void SaveCountWakeUp(uint16_t count_wkup)
{
   BKP_WriteBackupRegister(BKP_DR2, count_wkup);
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
   volatile static unsigned long ulFreeSizeStackTask;   //свободное место в процессах RTOS.
   volatile static unsigned long ulFreeHeapSize;   //свободное место в "куче" RTOS
   portTickType WakeTick = 0;
   WakeTick = xTaskGetTickCount() + configTICK_RATE_HZ;
   while (1)
   {
      if (xTaskGetTickCount() >= WakeTick)
      {
#if (INCLUDE_uxTaskGetStackHighWaterMark == 1)
         ulFreeSizeStackTask = uxTaskGetStackHighWaterMark(xHandleMainTask) << 2;
         ulFreeHeapSize = (unsigned long)xPortGetFreeHeapSize();   // in Byte
#endif
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
      ;
}
#endif