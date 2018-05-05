
#include "includes.h"
#include "dht22.h"

int read_cycle(int cur_tics, uint8_t neg_tic)
{
   int cnt_tics;

   if (cur_tics < MAX_TICS)
   {
      cnt_tics = 0;
   }
   if (neg_tic)
   {
      while (!GPIO_ReadInputDataBit(DHT22_PORT, DHT22_PIN) && (cnt_tics < MAX_TICS))
      {
         cnt_tics++;
      }
   }
   else
   {
      while (GPIO_ReadInputDataBit(DHT22_PORT, DHT22_PIN) && (cnt_tics < MAX_TICS))
      {
         cnt_tics++;
      }
   }
   return cnt_tics;
}

int Read_DHT22(uint8_t* pBuf)
{
   GPIO_InitTypeDef GPIO_InitStructure;
   uint8_t aucHumidityData[5];
   memset(aucHumidityData, 0, sizeof(aucHumidityData));
   uint16_t dt[42];
   int cnt;
   uint8_t i;
   uint8_t checkSum;
   uint8_t* buf = &aucHumidityData[0];

   RCC_APB2PeriphClockCmd(DHT22_PORT_CLOCK, ENABLE);

   GPIO_InitStructure.GPIO_Pin = DHT22_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(DHT22_PORT, &GPIO_InitStructure);
   GPIO_HIGH(DHT22_PORT, DHT22_PIN);

   // reset DHT22
   _delay_ms(500);
   GPIO_LOW(DHT22_PORT, DHT22_PIN);
   _delay_ms(20);

   //__disable_interrupt();
   vTaskSuspendAll();
   GPIO_HIGH(DHT22_PORT, DHT22_PIN);
   // start reading
   cnt = 0;
   for (i = 0; (i < 83 && cnt < MAX_TICS); i++)
   {
      if (i & 1)
      {
         cnt = read_cycle(cnt, 1);
      }
      else
      {
         cnt = read_cycle(cnt, 0);
         dt[i / 2] = cnt;
      }
   }
   xTaskResumeAll();
   //__enable_interrupt();

   if (cnt >= MAX_TICS)
   {
      return DHT22_NO_CONN;
   }

   // convert data
   for (i = 2; i < sizeof(dt); i++)
   {
      (*buf) <<= 1;
      if (dt[i] > MAX_LOG_VAL)
         (*buf)++;
      if (!((i - 1) % 8) && (i > 2))
         buf++;
   }

   // calculate checksum
   checkSum = aucHumidityData[0] + aucHumidityData[1] + aucHumidityData[2] + aucHumidityData[3];
   if (checkSum != aucHumidityData[4])
   {
      return DHT22_CS_ERROR;
   }

   for (i = 0; i < sizeof(aucHumidityData); i++)
   {
      pBuf[i] = aucHumidityData[i];
   }
   return DHT22_OK;
}