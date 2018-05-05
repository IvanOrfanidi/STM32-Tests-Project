
#include "includes.h"
#include "compass_general.h"

void vCompassTask(void* pvParameters)
{
   int16_t TempDataCompass[3];
   TCompass_Data stCompassData;
   char* pSt = (char*)&stCompassData;
   char toSend;

   // Создаём очередь
   xQueueCompassDataToUsart = xQueueCreate(sizeof(TCompass_Data), sizeof(uint8_t));
   vQueueAddToRegistry(xQueueCompassDataToUsart, "xQueueCompassDataToUsart");

   HMC5883L_I2C_Init();
   HMC5883L_Initialize();

   while (1)
   {
      if (HMC5883L_TestConnection() == TRUE)
      {
         HMC5883L_GetHeading(TempDataCompass);
         stCompassData.sValueAxisX = TempDataCompass[0];
         stCompassData.sValueAxisY = TempDataCompass[1];
         stCompassData.sValueAxisZ = TempDataCompass[2];
         stCompassData.bDataValid = TRUE;
      }
      else
      {
         stCompassData.bDataValid = FALSE;
      }

      if (xQueueCompassDataToUsart)
      {
         for (uint8_t i = 0; i < sizeof(TCompass_Data); i++)
         {
            toSend = pSt[i];
            xQueueSendToFront(xQueueCompassDataToUsart, (void*)&toSend, (portTickType)0);
         }
      }

      osDelay(1000);
   }
}