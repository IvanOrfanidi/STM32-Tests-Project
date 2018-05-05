
#include "accel_general.h"
#include "adxl345.h"
#include "includes.h"

#define ACCEL_THRESACT 2500 / 62.5   //Установка порога Толчка
#define ACCEL_THRESHOLD 2500 / 62.5   //Установка порога Толчка
#define ACCEL_DURATION 20000 / 625   //Установка лимита времени для Толчка
#define ACCEL_LATENT 10 / 1.25   //Установка задержки для Толчка
#define ACCEL_WINDOWS 60 / 1.25   //Установка временного окна для Второго Толчка
#define ACCEL_TIME_INACTIVITY 30   //Установка времени Неактивности

void vAccelTask(void* pvParameters)
{
   int16_t a_sAccelValue[3];
   static TAccel_Data stAccelData;
   char* pSt = (char*)&stAccelData;
   char toSend;

   // Создаём очередь
   xQueueAccelDataToUsart = xQueueCreate(sizeof(stAccelData), sizeof(uint32_t));
   vQueueAddToRegistry(xQueueAccelDataToUsart, "xQueueAccelDataToUsart");

   AccelInit();

   Accel_Clear_Settings();

   _delay_ms(200);

#ifdef ACCEL_THRESACT
   Set_Activity_Threshold(ACCEL_THRESACT);
#endif
   Set_Tap_Threshold(ACCEL_THRESHOLD);   //Установка порога Толчка
   Set_Tap_Duration(ACCEL_DURATION);   //Установка лимита времени для Толчка
   Set_Tap_Latency(ACCEL_LATENT);   //Установка задержки для Толчка
   Set_Tap_Window(ACCEL_WINDOWS);   //Установка временного окна для Второго Толчка
#ifdef ACCEL_TIME_INACTIVITY
   Set_Time_Inactivity(ACCEL_TIME_INACTIVITY);   //Установка времени Неактивности
#endif
   Reset_Interrupt();

#ifdef USE_ADXL345_INT1
   Set_Interrupt(SINGLE_TAP, 1, INT_1);
#endif
#ifdef USE_ADXL345_INT2
   Set_Interrupt(DOUBLE_TAP, 1, INT_2);
#endif
   // Standby_Mode(TRUE);
   // Accel_Sleep();

   while (1)
   {
      if (!(ADXL345_ReadXYZ(a_sAccelValue)))
      {
         stAccelData.sValueAxisX = a_sAccelValue[AXIS_X];
         stAccelData.sValueAxisY = a_sAccelValue[AXIS_Y];
         stAccelData.sValueAxisZ = a_sAccelValue[AXIS_Z];
         stAccelData.ucInterrupt = Get_Source_Interrupt();
         if (stAccelData.ucInterrupt != 0x02 && stAccelData.ucInterrupt != 0x82)
         {
            stAccelData.bDataValid = 1;
         }
         stAccelData.bDataValid = 1;
      }
      else
      {
         // Reset_Interrupt();
         stAccelData.bDataValid = 0;
      }

      //Если очередь создана, то заполняем её.
      if (xQueueAccelDataToUsart)
      {
         for (uint8_t i = 0; i < sizeof(stAccelData); i++)
         {
            toSend = pSt[i];
            xQueueSendToFront(xQueueAccelDataToUsart, (void*)&toSend, (portTickType)0);
         }
      }
      _delay_ms(500);
   }
}
