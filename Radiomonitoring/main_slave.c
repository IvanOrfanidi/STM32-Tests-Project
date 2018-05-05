#define __ALLOCATE_NOW__

// User functions //
#include "includes.h"

_Bool SendAlarm(void);
void DeviceSLEEP(void);

/* Protocol TX
1 - 0x02 - Device TX (0x01 - Master, 0x02 - Slave)
2 - 0x01 - Device RX (0x01 - Master, 0x02 - Slave)
3 - 0x01 - FUNCTION (0x01 - DIAGNOSTICS, 0x02 - NOTICE, 0x03 - FAIL)
4 - 0x00 - VALUE
5 - 0xFF - CRC8
*/
// const uint8_t g_aucProt[5] = {0x02, 0x01, 0x01, 0x00, 0xFF};      //
/* Protocol RX
1 - 0x01 - Device TX (0x01 - Master, 0x02 - Slave)
2 - 0x02 - Device RX (0x01 - Master, 0x02 - Slave)
3 - 0x01 - FUNCTION (0x01 - DIAGNOSTICS, 0x02 - NOTICE, 0x03 - FAIL)
4 - 0x00 - VALUE (0x00 - OK)
5 - 0xFF - CRC8
*/

// 0x01, 0x02, 0x01, 0x00, 0x04  //����� �� ������� ��� ������ SendOK
// 0x01, 0x02, 0x02, 0x00, 0x04  //����� �� ������� ��� ������ Alarm
// 0x01, 0x02, 0x03, 0x00, 0x04  //����� �� ������� ��� ������ AccelFail

// ID Task
xTaskHandle xHandleGeneralTask;
void vGeneralTask(void* pvParameters);
_Bool SendError(void);

#define TIMEOUT_RESPONSE 3000

#define DEF_TIME_ACCEL_STOP 10 * 10   // 60 SEC
#define DELTA_ACCEL_AXIS_X 10
#define DELTA_ACCEL_AXIS_Y 10
#define DELTA_ACCEL_AXIS_Z 10

TDrf_Settings g_stDrfSettings;

int main()
{
   InitGPIO();
   InitBKP();
   // rtc_init();

   // Start Task //
   xTaskCreate(
      vGeneralTask, "vGeneralTask", configMINIMAL_STACK_SIZE * 6, NULL, tskIDLE_PRIORITY + 1, &xHandleGeneralTask);
   // Start scheduler //
   osKernelStart(NULL, NULL);
}

void vGeneralTask(void* pvParameters)
{
   portTickType xLastWakeTimerDelay;
   static char strResBuff[DRF_RX_BUFFER_SIZE];   //����� ���� ����� ��� ������.
   static uint32_t uiTimeAccelStateStop = 10;   // DEF_TIME_ACCEL_STOP;
   int16_t a_sAccelValue[3];
   _Bool bSendDeviceOK = 1;   //�������� �� ������� ��� ������ ������ �������������� ����.
   _Bool bAccelFail = 0;
   _Bool bAlarm = 0;
   static _Bool bDeviceActive = FALSE;
   static TAccel_Data stAccelData;
   TAccel_Data stAccelStop;

   osDelay(SLEEP_MS_100);
   InitGpioDRF();
   InitUSART(UART_DRF, DRF_BAUDRATE);
   InitDMA(UART_DRF);

   TDrf_Settings stDrfSettings;
   //���� ��������� ������������ �����������
   stDrfSettings.uiFreq = 454130;
   stDrfSettings.eDRfsk = DRfsk_2400bps;
   stDrfSettings.ecPout = Pout_20dBm;
   stDrfSettings.eDRin = DRin_2400bps;
   stDrfSettings.eParity = NO_PARITY;

   _Bool bInitConfigDrf = 0;
   bSendDeviceOK = 0;
   osDelay(SLEEP_MS_200);

   /* ������ ������ ����������� */
   if (GetConfigDRF(&g_stDrfSettings, DRF_BAUDRATE))
   {
      bInitConfigDrf = 1;
   }
   else
   {
      bSendDeviceOK = 1;
      /* ���������� ������ ����������� � �������� */
      uint8_t* ptr1 = (uint8_t*)&stDrfSettings;
      uint8_t* ptr2 = (uint8_t*)&g_stDrfSettings;
      for (int i = 0; i < sizeof(TDrf_Settings); i++)
      {
         if (ptr1[i] != ptr2[i])
         {
            bInitConfigDrf = 1;
            bSendDeviceOK = 0;
            break;
         }
      }
   }

   /* ������������� �������� ������ ���� ����� */
   if (bInitConfigDrf)
   {
      if (!(SetConfigDRF(&stDrfSettings, DRF_BAUDRATE)))
      {
         bSendDeviceOK = 1;
         g_stDrfSettings = stDrfSettings;
      }
   }

   /* ������������ ������������� */
   AccelInit();
   Accel_Clear_Settings();
   osDelay(SLEEP_MS_100);
   // Set_Interrupt(DOUBLE_TAP, 1, INT_1);
   // Set_Interrupt(SINGLE_TAP, 1, INT_2);
   /* �������� �����, ����� ��� ������� */
   SetRadioEnable(1);
   xLastWakeTimerDelay = xTaskGetTickCount();
   vTaskDelayUntil(&xLastWakeTimerDelay, (200 / portTICK_RATE_MS));

   while (1)
   {
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
      {   //��� �� ������, ������ ���.
         USART_Read(UART_DRF, strResBuff, sizeof(strResBuff));
         ReStartDmaUsart2();
         uint8_t crc = _crc8_update(&strResBuff[0], CRC8);
         if ((strResBuff[DEVICE_RX] == SLAVE) &&
             (strResBuff[CRC8] == crc))   //���� ��� ���������� ������� � ������� CRC, �� ����� ������.
         {
            _Bool bDeviceSLEEP = FALSE;
            if (strResBuff[DEVICE_TX] == MASTER)   //��� �� ������� slave
            {
               if (strResBuff[FUNCTION] == SETTING)   //��� ���������
               {
                  switch (strResBuff[VALUE])
                  {
                  case OK:
                     bDeviceActive = TRUE;
                     if (bAlarm)
                     {
                        bAlarm = TRUE;
                        uiTimeAccelStateStop = DEF_TIME_ACCEL_STOP;
                     }
                     break;
                  case SLEEP:
                     bDeviceSLEEP = TRUE;
                     break;
                  }

                  /* ����� ��� �������� �������� */
                  char aucProt[5] = { SLAVE, MASTER, SETTING, OK, 0xFF };
                  aucProt[CRC8] = _crc8_update(aucProt, CRC8);   //��������� CRC
                  USART_Write(UART_DRF, aucProt, sizeof(aucProt));   //���������� OK
               }
            }

            if (bDeviceSLEEP)
            {
               xLastWakeTimerDelay = xTaskGetTickCount();
               vTaskDelayUntil(&xLastWakeTimerDelay, (2000 / portTICK_RATE_MS));
               SleepSlaveDevice();
            }
         }
      }

      /* ����� ������������� */
      if (!(ADXL345_ReadXYZ(a_sAccelValue)))
      {
         stAccelData.sValueAxisX = a_sAccelValue[AXIS_X];
         stAccelData.sValueAxisY = a_sAccelValue[AXIS_Y];
         stAccelData.sValueAxisZ = a_sAccelValue[AXIS_Z];
         stAccelData.ucInterrupt = Get_Source_Interrupt();
         // Reset_Interrupt();
         // Set_Interrupt(DOUBLE_TAP, 1, INT_1);
         // Set_Interrupt(SINGLE_TAP, 1, INT_2);
         stAccelData.bDataValid = 1;
         // printf("AXIS X:%i Y:%i Z:%i\r\n", stAccelData.sValueAxisX, stAccelData.sValueAxisY,
         // stAccelData.sValueAxisZ);
      }
      else
      {
         // Reset_Interrupt();
         stAccelData.bDataValid = 0;
         bSendDeviceOK = 0;
         if (!(bAccelFail))
            bAccelFail = 1;
      }
      /*****************************/

      if (bDeviceActive)
      {
         /* ������� ���������� ����� �������������, �� ��������� ������� ���������� ����� ������� ��� ������ �����������
          * ������������ */
         if (uiTimeAccelStateStop)
         {
            /* ���� ���������� */
            stAccelStop = stAccelData;
            uiTimeAccelStateStop--;
            // printf("STOP AXIS X:%i Y:%i Z:%i\r\n", stAccelStop.sValueAxisX, stAccelStop.sValueAxisY,
            // stAccelStop.sValueAxisZ);
         }
         else
         {
            /* ���������� ���������, ������� ��� ������ �����������. ��������� � �������� �������� */
            if ((stAccelData.sValueAxisX > stAccelStop.sValueAxisX + DELTA_ACCEL_AXIS_X) ||
                (stAccelData.sValueAxisX < stAccelStop.sValueAxisX - DELTA_ACCEL_AXIS_X))
            {
               bAlarm = 1;
            }
            if ((stAccelData.sValueAxisY > stAccelStop.sValueAxisY + DELTA_ACCEL_AXIS_Y) ||
                (stAccelData.sValueAxisY < stAccelStop.sValueAxisY - DELTA_ACCEL_AXIS_Y))
            {
               bAlarm = 1;
            }
            if ((stAccelData.sValueAxisZ > stAccelStop.sValueAxisZ + DELTA_ACCEL_AXIS_Z) ||
                (stAccelData.sValueAxisZ < stAccelStop.sValueAxisZ - DELTA_ACCEL_AXIS_Z))
            {
               bAlarm = 1;
            }
         }
      }
      /*****************************/

      /* ���������� ������ � ������ */

      if (bAlarm)
      {   //�������
         bAlarm = SendAlarm();
         osDelay(SLEEP_MS_100);
         //����������������� ������ ����� �� 2 ���.
         uiTimeAccelStateStop = 20;
         bAlarm = 0;
      }

      if (bAccelFail)
      {   //������
         bAccelFail = SendError();
         osDelay(SLEEP_MS_1000);
      }
      /*****************************/

      /* ���������� ���������� ������� */
      if (bSendDeviceOK)
      {   //���������� �� �������
         _Bool SendOk(void);
         // bSendDeviceOK = SendOk();
         if (bSendDeviceOK)
         {
            osDelay(SLEEP_MS_1000);
            // bSendDeviceOK = 0;
         }
      }
      /*********************************/

      xLastWakeTimerDelay = xTaskGetTickCount();
      vTaskDelayUntil(&xLastWakeTimerDelay, (SLEEP_MS_100 / portTICK_RATE_MS));
   }
}

_Bool SendAlarm(void)
{
   static _Bool bSendOK;
   char aucProt[5];
   aucProt[DEVICE_TX] = SLAVE;
   aucProt[DEVICE_RX] = MASTER;

   /* �������� ������� */
   aucProt[FUNCTION] = NOTICE;
   aucProt[VALUE] = ALARM;
   aucProt[CRC8] = _crc8_update(aucProt, CRC8);   //��������� CRC

   _Bool SendData(const char* ptr, int len);
   bSendOK = SendData(aucProt, sizeof(aucProt));
   ReStartDmaUsart2();
   return bSendOK;
}

_Bool SendError(void)
{
   static _Bool bSendOK;
   char aucProt[5];
   aucProt[DEVICE_TX] = SLAVE;
   aucProt[DEVICE_RX] = MASTER;

   /* �������� ����� ������������� */
   aucProt[FUNCTION] = DIAGNOSTICS;
   aucProt[VALUE] = ACCEL_FAIL;
   aucProt[CRC8] = _crc8_update(aucProt, CRC8);   //��������� CRC

   _Bool SendData(const char* ptr, int len);
   bSendOK = SendData(aucProt, sizeof(aucProt));
   ReStartDmaUsart2();
   return bSendOK;
}

_Bool SendOk(void)
{
   static _Bool bSendOK;
   char aucProt[5];
   aucProt[DEVICE_TX] = SLAVE;
   aucProt[DEVICE_RX] = MASTER;

   /* �������� OK */
   aucProt[FUNCTION] = SETTING;
   aucProt[VALUE] = OK;
   aucProt[CRC8] = _crc8_update(aucProt, CRC8);   //��������� CRC

   _Bool SendData(const char* ptr, int len);
   bSendOK = SendData(aucProt, sizeof(aucProt));
   ReStartDmaUsart2();
   return bSendOK;
}

_Bool SendData(const char* ptr, int len)
{
   _Bool bSendOK = 1;
   portTickType xLastWakeTimerDelay;
   char strRes[32];

   //������ ��� ������� ��������.
   for (uint8_t i = 0; i < 3; i++)
   {
      memset(strRes, 0, sizeof(strRes));
      USART_Write(UART_DRF, ptr, len);   //����������
      xLastWakeTimerDelay = xTaskGetTickCount();
      vTaskDelayUntil(&xLastWakeTimerDelay, (TIMEOUT_RESPONSE / portTICK_RATE_MS));   //���� ������
      USART_Read(UART_DRF, strRes, sizeof(strRes));   //��������� �����

      for (uint8_t i = 0; i < sizeof(strRes); i++)
      {
         if (strRes[i] != 0)
         {
            if ((strRes[i] == MASTER) && (strRes[i + 1] == SLAVE) && (strRes[i + 4] == _crc8_update(&strRes[i], 4)))
            {
               if ((strRes[i + 2] == ptr[3]) && (strRes[i + 3] == OK))
               {
                  bSendOK = 0;   //��� �� ������� �� ������� �����
                  break;
               }
            }
         }
      }
      if (!(bSendOK))
      {   //��� �� ������� �� ������� �����
         break;
      }
      USART_Read(UART_DRF, strRes, sizeof(strRes));
   }
   osDelay(SLEEP_MS_100);

   /* Clear RX Buffer */
   USART_Read(UART_DRF, strRes, sizeof(strRes));

   return bSendOK;
}

void DeviceSLEEP(void)
{
   Accel_Clear_Settings();
   Accel_Sleep();
   osDelay(100);
   SleepSlaveDevice();
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
   volatile static unsigned long ulFreeSizeStackTask;   //��������� ����� � ��������� RTOS.
   volatile static unsigned long ulFreeHeapSize;   //��������� ����� � "����" RTOS
   portTickType WakeTick = 0;
   WakeTick = xTaskGetTickCount() + configTICK_RATE_HZ;
   while (1)
   {
      if (xTaskGetTickCount() >= WakeTick)
      {
#if (INCLUDE_uxTaskGetStackHighWaterMark == 1)
         ulFreeSizeStackTask = uxTaskGetStackHighWaterMark(xHandleGeneralTask) << 2;
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