

#include "includes.h"
#include "humidity_general.h"
#include "dht11.h"

void vExtHumidityTask(void* pvParameters)
{
   THumidity_Data stHumidityData;
   uint8_t aucHumidityData[5];

   // Создаём очередь
   xQueueExtHumidityForLcd = xQueueCreate(sizeof(uint8_t), sizeof(THumidity_Data));
   vQueueAddToRegistry(xQueueExtHumidityForLcd, "xQueueExtHumidityForLcd");

   while (1)
   {
      if (Read_DHT22(aucHumidityData) == DHT22_OK)
      {
         if ((aucHumidityData[1] == 0) && (aucHumidityData[3] == 0))
         {   // dht11
            stHumidityData.usRealHumidity = aucHumidityData[1];
            stHumidityData.usRealHumidity = stHumidityData.usRealHumidity << 8;
            stHumidityData.usRealHumidity |= aucHumidityData[0];
            stHumidityData.usRealHumidity *= 10;

            stHumidityData.sRealTemperatur = aucHumidityData[3];
            stHumidityData.sRealTemperatur = stHumidityData.sRealTemperatur << 8;
            stHumidityData.sRealTemperatur |= aucHumidityData[2];
            stHumidityData.sRealTemperatur *= 10;
         }
         else
         {   // dht21-dht22
            stHumidityData.usRealHumidity = aucHumidityData[0] * 256 + aucHumidityData[1];
            stHumidityData.sRealTemperatur = (aucHumidityData[2] & 0x7F) * 256 + aucHumidityData[3];
            if (aucHumidityData[2] & 0x80)
            {
               stHumidityData.sRealTemperatur *= -1;
            }
         }
         stHumidityData.bDataValid = 1;
      }
      else
      {
         stHumidityData.bDataValid = 0;
      }

      if (xQueueExtHumidityForLcd)
      {
         xQueueSendToFront(xQueueExtHumidityForLcd, &stHumidityData, (portTickType)0);
      }
      _delay_ms(500);
   }
}

void vIntHumidityTask(void* pvParameters)
{
   THumidity_Data stHumidityData;
   uint8_t aucHumidityData[5];

   // Создаём очередь
   xQueueIntHumidityForLcd = xQueueCreate(sizeof(uint8_t), sizeof(THumidity_Data));
   vQueueAddToRegistry(xQueueIntHumidityForLcd, "xQueueIntHumidityForLcd");

   while (1)
   {
      if (Read_DHT11(aucHumidityData) == DHT11_OK)
      {
         if ((aucHumidityData[1] == 0) && (aucHumidityData[3] == 0))
         {   // dht11
            stHumidityData.usRealHumidity = aucHumidityData[1];
            stHumidityData.usRealHumidity = stHumidityData.usRealHumidity << 8;
            stHumidityData.usRealHumidity |= aucHumidityData[0];
            stHumidityData.usRealHumidity *= 10;

            stHumidityData.sRealTemperatur = aucHumidityData[3];
            stHumidityData.sRealTemperatur = stHumidityData.sRealTemperatur << 8;
            stHumidityData.sRealTemperatur |= aucHumidityData[2];
            stHumidityData.sRealTemperatur *= 10;
         }
         else
         {   // dht21-dht22
            stHumidityData.usRealHumidity = aucHumidityData[0] * 256 + aucHumidityData[1];
            stHumidityData.sRealTemperatur = (aucHumidityData[2] & 0x7F) * 256 + aucHumidityData[3];
            if (aucHumidityData[2] & 0x80)
            {
               stHumidityData.sRealTemperatur *= -1;
            }
         }
         stHumidityData.bDataValid = 1;
      }
      else
      {
         stHumidityData.bDataValid = 0;
      }

      if (xQueueExtHumidityForLcd)
      {
         xQueueSendToFront(xQueueIntHumidityForLcd, &stHumidityData, (portTickType)0);
      }
      _delay_ms(500);
   }
}
