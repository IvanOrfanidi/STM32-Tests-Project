
#include "debug.h"
#include "includes.h"

int HandlerOnewire(char* pOut);
int HandlerPressure(char* pOut);
int HandlerHumidity(char* pOut);
int HandlerAdc(char* pOut);

void vDebugTask(void* pvParameters)
{
   int Res;

   InitUSART(UART_DBG, DBG_BAUDRATE);
   InitDMA(UART_DBG);

   char strMsgDebug[128];
   char strMsgTemp[64];

   while (1)
   {
      memset(strMsgDebug, 0, sizeof(strMsgDebug));
      Res = HandlerOnewire(strMsgTemp);
      if (!(Res))
      {
         strcat(strMsgDebug, strMsgTemp);
      }

      Res = HandlerPressure(strMsgTemp);
      if (!(Res))
      {
         strcat(strMsgDebug, strMsgTemp);
      }

      Res = HandlerAdc(strMsgTemp);
      if (!(Res))
      {
         strcat(strMsgDebug, strMsgTemp);
      }

      Res = HandlerHumidity(strMsgTemp);
      if (!(Res))
      {
         strcat(strMsgDebug, strMsgTemp);
      }

      strcat(strMsgDebug, "\r\n");

#ifdef DEBUG_OUTPUT_USART
      USART_Write(UART_DBG, strMsgDebug, strlen(strMsgDebug));
#endif

      _delay_ms(1000);
   }
}

int HandlerOnewire(char* pOut)
{
   float fTemperatur;
   TOnewire_Data stTemperaturData;
   char* pSt = (char*)&stTemperaturData;
   char toSend;

   //Проверяем создана ли очередь и читаем из нее.
   if (xQueueOnewireDataToUsart != 0)
   {
      for (uint8_t i = 0; i < sizeof(stTemperaturData); i++)
      {
         if (xQueueReceive(xQueueOnewireDataToUsart, (void*)&toSend, (portTickType)10))
         {
            pSt[sizeof(stTemperaturData) - 1 - i] = toSend;
         }
         else
         {
            //В очереде пока ничего нет.
            return 1;
         }
      }

      if (stTemperaturData.bDataValid)
      {
         fTemperatur = stTemperaturData.iRealTemperatur;
         fTemperatur /= 10;
         if (fTemperatur < 0)
         {
            sprintf(pOut, "TEMPERATUR ONEWIRE: %.01f C\r\n", fTemperatur);
         }
         else
         {
            sprintf(pOut, "TEMPERATUR ONEWIRE: +%.01f C\r\n", fTemperatur);
         }
         return 0;
      }
      else
      {
         sprintf(pOut, "TEMPERATUR: ERROR\r\n");
         return 0;
      }
   }

   //Очереди вообще не удалось создаться.
   return 2;
}

int HandlerPressure(char* pOut)
{
   TPressure_Data stPressureData;
   char* pSt = (char*)&stPressureData;
   char toSend;

   float fTemperatur;   //Температура в С
   float fRealPressurePa;   //Давление в кПа
   int iRealPressureMMHG;   //Давление в ммРс

   //Проверяем создана ли очередь и читаем из нее.
   if (xQueuePressureDataToUsart != 0)
   {
      for (uint8_t i = 0; i < sizeof(stPressureData); i++)
      {
         if (xQueueReceive(xQueuePressureDataToUsart, (void*)&toSend, (portTickType)10))
         {
            pSt[sizeof(stPressureData) - 1 - i] = toSend;
         }
         else
         {
            //В очереде пока ничего нет.
            return 1;
         }
      }

      fTemperatur = stPressureData.iRealTemperatur;
      fTemperatur /= 10;

      fRealPressurePa = stPressureData.iRealPressurePa;
      fRealPressurePa /= 1000;

      iRealPressureMMHG = stPressureData.iRealPressureHg;

      if (fTemperatur < 0)
      {
         sprintf(pOut,
                 "TEMPERATUR BMP: -%.01f C\r\nPRESSURE: %.03f kPa (%i mmHg)\r\n",
                 fTemperatur,
                 fRealPressurePa,
                 iRealPressureMMHG);
      }
      else
      {
         sprintf(pOut,
                 "TEMPERATUR BMP: +%.01f C\r\nPRESSURE: %.03f kPa (%i mmHg)\r\n",
                 fTemperatur,
                 fRealPressurePa,
                 iRealPressureMMHG);
      }

      return 0;
   }

   //Очереди вообще не удалось создаться.
   return 2;
}

int HandlerAdc(char* pOut)
{
   ADCmeasCFG stAdcReceptacle;
   char* pSt = (char*)&stAdcReceptacle;
   char toSend;

   //Проверяем создана ли очередь и читаем из нее.
   if (xQueueAdcDataToUsart != 0)
   {
      for (uint8_t i = 0; i < sizeof(stAdcReceptacle); i++)
      {
         if (xQueueReceive(xQueueAdcDataToUsart, (void*)&toSend, (portTickType)10))
         {
            pSt[sizeof(stAdcReceptacle) - 1 - i] = toSend;
         }
         else
         {
            //В очереде пока ничего нет.
            return 1;
         }
      }

      sprintf(pOut,
              "ADC#1 VALUE: %i\r\nADC#2 VALUE: %i\r\nADC#3 VALUE: %i\r\n",
              stAdcReceptacle.usValue1,
              stAdcReceptacle.usValue2,
              stAdcReceptacle.usValue3);

      return 0;
   }

   //Очереди вообще не удалось создаться.
   return 2;
}

int HandlerHumidity(char* pOut)
{
   THumidity_Data stHumidityData;
   char* pSt = (char*)&stHumidityData;
   char toSend;

   //Проверяем создана ли очередь и читаем из нее.
   if (xQueueHumidityDataToUsart != 0)
   {
      for (uint8_t i = 0; i < sizeof(stHumidityData); i++)
      {
         if (xQueueReceive(xQueueHumidityDataToUsart, (void*)&toSend, (portTickType)10))
         {
            pSt[sizeof(stHumidityData) - 1 - i] = toSend;
         }
         else
         {
            //В очереде пока ничего нет.
            return 1;
         }
      }

      if (stHumidityData.bDataValid)
      {
         sprintf(pOut,
                 "TEMPERATUR DHT: +%i C\r\nHUMIDITY: %i",
                 stHumidityData.usRealTemperatur,
                 stHumidityData.usRealHumidity);
         strcat(pOut, "%\r\n");
         return 0;
      }
      else
      {
         sprintf(pOut, "HUMIDITY: ERROR\r\n");
         return 0;
      }
   }

   //Очереди вообще не удалось создаться.
   return 2;
}