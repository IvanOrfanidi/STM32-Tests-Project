#include "button_general.h"

void vButtonTask(void* pvParameters)
{
   int16_t sTimerBack = sTIMET1_CCR1_Val;

   while (1)
   {
      if (GET_BUT1)
         sTIMET1_CCR1_Val += 100;
      if (GET_BUT2)
         sTIMET1_CCR1_Val -= 100;

      if (GET_BUT3)
      {
         xSemaphoreGive(mLCD_GPS_INDICATING);
      }

      if (sTimerBack != sTIMET1_CCR1_Val)
      {
         if (sTIMET1_CCR1_Val > TIM3_PERIOD)
            sTIMET1_CCR1_Val = TIM3_PERIOD;
         if (sTIMET1_CCR1_Val < 100)
            sTIMET1_CCR1_Val = 100;

         sTimerBack = sTIMET1_CCR1_Val;
         TIM_SetCompare1(TIM3, sTIMET1_CCR1_Val);
         BKP_WriteBackupRegister(BKP_DR2, sTIMET1_CCR1_Val);
         xSemaphoreGive(mLCD_LED_INDICATING);
      }

      _delay_ms(200);
   }
}