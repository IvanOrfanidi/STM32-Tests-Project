
#include "lcd_general.h"
#include "includes.h"

THumidity_Data stExtHumidityData;
THumidity_Data stIntHumidityData;
TPressure_Data stPressureData;
RTC_t stDateTime;
uint8_t eStepLcd = 0;

void vLcdTask(void* pvParameters)
{
   char strMsgLcd[(16 * 2) + 1];
   char strMsgTemp[32];

   LCD_Init();
   LCD_Send_CMD(DISP_ON);
   LCD_Send_CMD(CLR_DISP);
   _delay_ms(250);
   LCD_Send_STR("-=RUN APPLICAT=-");
   _delay_ms(1000);

   while (1)
   {
      memset(strMsgLcd, 0, sizeof(strMsgLcd));

      while (xQueueReceive(xQueuePressureForLcd, &stPressureData, (portTickType)0) != pdFALSE)
         ;
      while (xQueueReceive(xQueueExtHumidityForLcd, &stExtHumidityData, (portTickType)0) != pdFALSE)
         ;
      while (xQueueReceive(xQueueIntHumidityForLcd, &stIntHumidityData, (portTickType)0) != pdFALSE)
         ;

      if (!(stExtHumidityData.bDataValid))
      {
         stExtHumidityData.sRealTemperatur = -850;
         stExtHumidityData.usRealHumidity = 999;
      }

      // rtc_gettime(&stDateTime);
      stDateTime.hour = 1;
      stDateTime.min = 2;
      stDateTime.sec = 3;
      sprintf(strMsgTemp,
              "%02d:%02d:%02d %dmmHg",
              stDateTime.hour,
              stDateTime.min,
              stDateTime.sec,
              stPressureData.iRealPressureHg);

      memcpy(&strMsgLcd[0], strMsgTemp, strlen(strMsgTemp));

      if (eStepLcd == 0)
      {
         sprintf(strMsgTemp,
                 "Ext % 3dC Int% 2dC",
                 stExtHumidityData.sRealTemperatur / 10,
                 stPressureData.iRealTemperatur / 10);
         memcpy(&strMsgLcd[sizeof(strMsgLcd) / 2], strMsgTemp, strlen(strMsgTemp));
      }
      if (eStepLcd == 40)
      {
         sprintf(strMsgTemp,
                 "Ext % 3d%% Int% 2d%%",
                 stExtHumidityData.usRealHumidity / 10,
                 stIntHumidityData.usRealHumidity / 10);
         memcpy(&strMsgLcd[sizeof(strMsgLcd) / 2], strMsgTemp, strlen(strMsgTemp));
      }

      eStepLcd++;
      if (eStepLcd >= 80)
      {
         eStepLcd = 0;
      }

      if (osMutexWait(mLCD_GPS_INDICATING, 0) == osOK)
      {
         xSemaphoreTake(mLCD_GPS_INDICATING, SLEEP_MS_100);
         eStepLcd = 41;
      }

      if (osMutexWait(mLCD_LED_INDICATING, 0) == osOK)
      {
         xSemaphoreTake(mLCD_LED_INDICATING, SLEEP_MS_100);
         memset(&strMsgLcd[sizeof(strMsgLcd) / 2], 0x20, sizeof(strMsgLcd) / 2);
         strMsgLcd[sizeof(strMsgLcd) / 2] = 0;
         for (int16_t i = 0, n = sizeof(strMsgLcd) / 2; i < sTIMET1_CCR1_Val; i += 100, n++)
         {
            strMsgLcd[n] = 'X';
         }

         sprintf(strMsgTemp, " % 3d%%", sTIMET1_CCR1_Val / 10);
         memcpy(&strMsgLcd[sizeof(strMsgLcd) - strlen(strMsgTemp)], strMsgTemp, strlen(strMsgTemp));
         eStepLcd = 41;
      }

      // LCD_Send_CMD(CLR_DISP);
      LCD_Send_CMD(DD_RAM_ADDR1);
      // LCD_Send_STR(strMsgLcd);
      for (uint8_t i = 0; i < strlen(strMsgLcd); i++)
      {
         _delay_ms(1);
         if (i == sizeof(strMsgLcd) / 2)
         {
            LCD_Send_CMD(DD_RAM_ADDR2);
         }
         if (strMsgLcd[i])
         {
            LCD_Send_CHAR(strMsgLcd[i]);
         }
         else
         {
            break;
         }
      }

      _delay_ms(100);
   }

   vTaskDelete(NULL);
}
