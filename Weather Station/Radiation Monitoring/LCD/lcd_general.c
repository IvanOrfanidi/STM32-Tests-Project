
#include "lcd_general.h"
#include "includes.h"

void vLcdTask(void* pvParameters)
{
   char strMsgLcd[16 * 2];
   char strMsgTemp[16];

   LCD_Init();
   LCD_Send_CMD(DISP_ON);
   LCD_Send_CMD(CLR_DISP);
   _delay_ms(250);
   LCD_Send_STR("-=RUN APPLICAT=-");
   _delay_ms(1000);

   while (1)
   {
      _delay_ms(1000);
   }

   vTaskDelete(NULL);
}
