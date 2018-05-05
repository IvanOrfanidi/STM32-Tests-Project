

#include "includes.h"
#include "humidity_general.h"

void vHumidityTask(void* pvParameters)
{
   THumidity_Data stHumidityData;
   uint8_t aucHumidityData[5];
   char* pSt = (char*)&stHumidityData;
   char toSend;
   /*
      // Создаём очередь
   xQueueOnewireDataToLcd = xQueueCreate(sizeof(stHumidityData), sizeof(uint32_t));
   vQueueAddToRegistry(xQueueHumidityDataToLcd, "xQueueHumidityDataToLcd" );
  */

   // Создаём очередь
   xQueueHumidityDataToUsart = xQueueCreate(sizeof(stHumidityData), sizeof(uint32_t));
   vQueueAddToRegistry(xQueueHumidityDataToUsart, "xQueueHumidityDataToUsart");

   tim_init(TIM2); /* init delay_us used in DHT11 lib */

   while (1)
   {
      if (dht11_read(aucHumidityData) == DHT11_OK)
      {
         stHumidityData.usRealHumidity = aucHumidityData[1];
         stHumidityData.usRealHumidity = stHumidityData.usRealHumidity << 8;
         stHumidityData.usRealHumidity |= aucHumidityData[0];

         stHumidityData.usRealTemperatur = aucHumidityData[3];
         stHumidityData.usRealTemperatur = stHumidityData.usRealTemperatur << 8;
         stHumidityData.usRealTemperatur |= aucHumidityData[2];

         stHumidityData.bDataValid = 1;
      }
      else
      {
         stHumidityData.bDataValid = 0;
      }

      if (xQueueHumidityDataToUsart)
      {
         for (uint8_t i = 0; i < sizeof(stHumidityData); i++)
         {
            toSend = pSt[i];
            xQueueSendToFront(xQueueHumidityDataToUsart, (void*)&toSend, (portTickType)0);
         }
      }

      _delay_ms(1000);
   }
}