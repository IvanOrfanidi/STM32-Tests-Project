
#include "lcd_general.h"
#include "includes.h"

void vLcdTask(void* pvParameters)
{
   RTC_t stDate;
   char strMsgstDate[20];
   uint16_t year = 0;
   uint8_t month = 0;
   uint8_t mday = 0;

   // RC522
   uint8_t str[MFRC522_MAX_LEN];   // MFRC522_MAX_LEN = 16
   memset(str, 0, MFRC522_MAX_LEN);
   SPI_RC522_Init();
   MFRC522_Init();

   LCD_Init();
   LCD_Send_CMD(DISP_ON);
   LCD_Send_CMD(CLR_DISP);
   _delay_ms(250);
   LCD_Send_STR("-HELLO-");
   _delay_ms(1000);

   while (1)
   {
      rtc_gettime(&stDate);
      LCD_Send_CMD(DD_RAM_ADDR1);

      sprintf(strMsgstDate, "%02d:%02d:%02d", stDate.hour, stDate.min, stDate.sec);
      for (uint8_t i = 0; i < strlen(strMsgstDate); i++)
      {
         LCD_Send_CHAR(strMsgstDate[i]);
      }

      if (!MFRC522_Request(PICC_REQIDL, str))
      {
         if (!MFRC522_Anticoll(str))
         {
            memset(strMsgstDate, 0, sizeof(strMsgstDate));
            for (int i = 0; i < 4; i++)
            {
               char strTemp[3];
               sprintf(strTemp, "%02X ", str[i]);
               strcat(strMsgstDate, strTemp);
            }
            LCD_Send_CMD(DD_RAM_ADDR2);
            for (uint8_t i = 0; i < strlen(strMsgstDate); i++)
            {
               LCD_Send_CHAR(strMsgstDate[i]);
            }
            //_delay_ms(1000);
         }
      }

      _delay_ms(1);
   }

   vTaskDelete(NULL);
}
