
#include "dht11.h"

int read_cycle_dht11(int cur_tics, uint8_t neg_tic)
{
   int cnt_tics;

   if (cur_tics < MAX_TICS)
   {
      cnt_tics = 0;
   }
   if (neg_tic)
   {
      while (!GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN) && (cnt_tics < MAX_TICS))
      {
         cnt_tics++;
      }
   }
   else
   {
      while (GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN) && (cnt_tics < MAX_TICS))
      {
         cnt_tics++;
      }
   }
   return cnt_tics;
}

int Read_DHT11(uint8_t* pBuf)
{
   GPIO_InitTypeDef GPIO_InitStructure;
   uint8_t aucHumidityData[5];
   memset(aucHumidityData, 0, sizeof(aucHumidityData));
   uint16_t dt[42];
   int cnt;
   uint8_t i;
   uint8_t checkSum;
   uint8_t* buf = &aucHumidityData[0];

   RCC_APB2PeriphClockCmd(DHT11_PORT_CLOCK, ENABLE);

   GPIO_InitStructure.GPIO_Pin = DHT11_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(DHT11_PORT, &GPIO_InitStructure);
   GPIO_HIGH(DHT11_PORT, DHT11_PIN);

   // reset DHT11
   _delay_ms(500);
   GPIO_LOW(DHT11_PORT, DHT11_PIN);
   _delay_ms(20);

   //__disable_interrupt();
   vTaskSuspendAll();
   GPIO_HIGH(DHT11_PORT, DHT11_PIN);
   // start reading
   cnt = 0;
   for (i = 0; (i < 83 && cnt < MAX_TICS); i++)
   {
      if (i & 1)
      {
         cnt = read_cycle_dht11(cnt, 1);
      }
      else
      {
         cnt = read_cycle_dht11(cnt, 0);
         dt[i / 2] = cnt;
      }
   }
   xTaskResumeAll();
   //__enable_interrupt();

   if (cnt >= MAX_TICS)
   {
      return DHT11_NO_CONN;
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
      return DHT11_CS_ERROR;
   }

   for (i = 0; i < sizeof(aucHumidityData); i++)
   {
      pBuf[i] = aucHumidityData[i];
   }
   return DHT11_OK;
}