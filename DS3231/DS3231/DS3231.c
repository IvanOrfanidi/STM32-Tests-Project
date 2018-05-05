

#include "includes.h"
#include "DS3231.h"

#define DEF_TIMEOUT_I2C 500

uint8_t Bcd2Bin(uint8_t bcd)
{
   uint8_t dec;

   dec = 10 * (bcd >> 4);
   dec += bcd & 0x0F;
   return dec;
}
uint8_t Bin2Bcd(uint8_t bin)
{
   uint8_t low = 0;
   uint8_t high = 0;

   // high nibble
   high = bin / 10;
   // low nibble
   low = bin - (high * 10);
   return (high << 4 | low);
}

void InitI2C(void)
{
   // Init Structure
   GPIO_InitTypeDef GPIO_InitStructure;
   // Init I2C
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;   // PP onboard?
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(GPIOB, &GPIO_InitStructure);

   I2C_InitTypeDef I2CInit;
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);   // Enable I2C clock
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

   I2C_DeInit(I2C1);   // I2C reset to initial state
   I2CInit.I2C_Mode = I2C_Mode_I2C;   // I2C mode is I2C ^_^
   I2CInit.I2C_DutyCycle = I2C_DutyCycle_2;   // I2C fast mode duty cycle (WTF is this?)
   I2CInit.I2C_OwnAddress1 = 1;   // This device address (7-bit or 10-bit)
   I2CInit.I2C_Ack = I2C_Ack_Enable;   // Acknowledgement enable
   I2CInit.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;   // choose 7-bit address for acknowledgement
   I2CInit.I2C_ClockSpeed = IOE_I2C_SPEED;   // 400kHz ?
   I2C_Cmd(I2C1, ENABLE);   // Enable I2C
   I2C_Init(I2C1, &I2CInit);   // Configure I2C

   I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY);
}

void InitDs3231(void)
{
   int iTimeout = DEF_TIMEOUT_I2C;
   // Check connection to DS3231
   I2C_GenerateSTART(I2C_PORT, ENABLE);   // Send START condition
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT))
   {
      iTimeout--;
      if (!(iTimeout))
      {
         return;
      }
   }   // Wait for EV5
   I2C_Send7bitAddress(I2C_PORT, DS3231_addr, I2C_Direction_Transmitter);   // Send DS3231 slave address
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
   {   // Wait for EV6

      iTimeout--;
      if (!(iTimeout))
      {
         return;
      }
   }
   I2C_GenerateSTOP(I2C_PORT, ENABLE);

   // DS3231 init
   I2C_GenerateSTART(I2C_PORT, ENABLE);
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT))
   {   // Wait for EV5

      iTimeout--;
      if (!(iTimeout))
      {
         return;
      }
   }
   I2C_Send7bitAddress(I2C_PORT, DS3231_addr, I2C_Direction_Transmitter);   // Send DS3231 slave address
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
   {   // Wait for EV6

      iTimeout--;
      if (!(iTimeout))
      {
         return;
      }
   }
   I2C_SendData(I2C_PORT, DS3231_control);   // Send DS3231 control register address
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
   {   // Wait for EV8

      iTimeout--;
      if (!(iTimeout))
      {
         return;
      }
   }
   I2C_SendData(I2C_PORT, INTCN | A1IE);   //
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
   {   // Wait for EV8

      iTimeout--;
      if (!(iTimeout))
      {
         return;
      }
   }
   I2C_SendData(I2C_PORT, 0x00);   // DS3231 clear alarm flags
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
   {   // Wait for EV8

      iTimeout--;
      if (!(iTimeout))
      {
         return;
      }
   }
   I2C_GenerateSTOP(I2C_PORT, ENABLE);
}

void ResetAlarmDs3231(void)
{
   int iTimeout = DEF_TIMEOUT_I2C;
   // Check connection to DS3231
   I2C_GenerateSTART(I2C_PORT, ENABLE);   // Send START condition
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT))
   {
      iTimeout--;
      if (!(iTimeout))
      {
         return;
      }
   }   // Wait for EV5
   I2C_Send7bitAddress(I2C_PORT, DS3231_addr, I2C_Direction_Transmitter);   // Send DS3231 slave address
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
   {   // Wait for EV6

      iTimeout--;
      if (!(iTimeout))
      {
         return;
      }
   }
   I2C_GenerateSTOP(I2C_PORT, ENABLE);

   // DS3231 init
   I2C_GenerateSTART(I2C_PORT, ENABLE);
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT))
   {   // Wait for EV5

      iTimeout--;
      if (!(iTimeout))
      {
         return;
      }
   }
   I2C_Send7bitAddress(I2C_PORT, DS3231_addr, I2C_Direction_Transmitter);   // Send DS3231 slave address
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
   {   // Wait for EV6

      iTimeout--;
      if (!(iTimeout))
      {
         return;
      }
   }
   I2C_SendData(I2C_PORT, DS3231_control);   // Send DS3231 control register address
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
   {   // Wait for EV8

      iTimeout--;
      if (!(iTimeout))
      {
         return;
      }
   }
   I2C_SendData(I2C_PORT, INTCN | A1IE);   //
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
   {   // Wait for EV8

      iTimeout--;
      if (!(iTimeout))
      {
         return;
      }
   }
   I2C_SendData(I2C_PORT, 0x00);   // DS3231 clear alarm flags
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
   {   // Wait for EV8

      iTimeout--;
      if (!(iTimeout))
      {
         return;
      }
   }
   I2C_GenerateSTOP(I2C_PORT, ENABLE);
}

int DS3231_ReadDateRAW(DS3231_date_TypeDef* date)
{
   unsigned int i;
   char buffer[7];

   int iTimeout = DEF_TIMEOUT_I2C;

   I2C_AcknowledgeConfig(I2C_PORT, ENABLE);   // Enable I2C acknowledge

   I2C_GenerateSTART(I2C_PORT, ENABLE);   // Send START condition
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT))
   {
      iTimeout--;
      if (!(iTimeout))
      {
         return -1;
      }
   }   // Wait for EV5

   I2C_Send7bitAddress(I2C_PORT, DS3231_addr, I2C_Direction_Transmitter);   // Send DS3231 slave address
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
   {   // Wait for EV6

      iTimeout--;
      if (!(iTimeout))
      {
         return -1;
      }
   }

   I2C_SendData(I2C_PORT, DS3231_seconds);   // Send DS3231 seconds register address
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
   {   // Wait for EV8

      iTimeout--;
      if (!(iTimeout))
      {
         return -1;
      }
   }

   I2C_GenerateSTART(I2C_PORT, ENABLE);   // Send repeated START condition (aka Re-START)
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT))
   {   // Wait for EV5

      iTimeout--;
      if (!(iTimeout))
      {
         return -1;
      }
   }

   I2C_Send7bitAddress(I2C_PORT, DS3231_addr, I2C_Direction_Receiver);   // Send DS3231 slave address for READ
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
   {   // Wait for EV6

      iTimeout--;
      if (!(iTimeout))
      {
         return -1;
      }
   }

   for (i = 0; i < 6; i++)
   {
      while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_RECEIVED))
      {   // Wait for EV7 (Byte received from slave)

         iTimeout--;
         if (!(iTimeout))
         {
            return -1;
         }
      }
      buffer[i] = I2C_ReceiveData(I2C_PORT);   // Receive byte
   }

   I2C_AcknowledgeConfig(I2C_PORT, DISABLE);   // Disable I2C acknowledgement
   I2C_GenerateSTOP(I2C_PORT, ENABLE);   // Send STOP condition
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_RECEIVED))
   {   // Wait for EV7 (Byte received from slave)

      iTimeout--;
      if (!(iTimeout))
      {
         return -1;
      }
   }

   buffer[i] = I2C_ReceiveData(I2C_PORT);   // Receive last byte

   memcpy(date, &buffer[0], 7);
   return 0;
}

void DS3231_WriteDateRAW(DS3231_date_TypeDef* date)
{
   unsigned int i;
   char buffer[7];
   memcpy(&buffer[0], date, 7);
   int iTimeout = DEF_TIMEOUT_I2C;

   I2C_AcknowledgeConfig(I2C_PORT, ENABLE);   // Enable I2C acknowledge

   I2C_GenerateSTART(I2C_PORT, ENABLE);   // Send START condition
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT))
   {   // Wait for EV5

      iTimeout--;
      if (!(iTimeout))
      {
         return;
      }
   }

   I2C_Send7bitAddress(I2C_PORT, DS3231_addr, I2C_Direction_Transmitter);   // Send DS3231 slave address
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
   {   // Wait for EV6

      iTimeout--;
      if (!(iTimeout))
      {
         return;
      }
   }

   I2C_SendData(I2C_PORT, DS3231_seconds);   // Send DS3231 seconds register address
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
   {   // Wait for EV8

      iTimeout--;
      if (!(iTimeout))
      {
         return;
      }
   }

   for (i = 0; i < 7; i++)
   {
      I2C_SendData(I2C_PORT, buffer[i]);   // Send DS3231 seconds register address
      iTimeout = DEF_TIMEOUT_I2C;
      while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
      {   // Wait for EV8

         iTimeout--;
         if (!(iTimeout))
         {
            return;
         }
      }
   }

   I2C_GenerateSTOP(I2C_PORT, ENABLE);
}

int DS3231_ReadAlarm1RAW(DS3231_date_TypeDef* date)
{
   unsigned int i;
   char buffer[7];

   int iTimeout = DEF_TIMEOUT_I2C;

   I2C_AcknowledgeConfig(I2C_PORT, ENABLE);   // Enable I2C acknowledge

   I2C_GenerateSTART(I2C_PORT, ENABLE);   // Send START condition
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT))
   {
      iTimeout--;
      if (!(iTimeout))
      {
         return -1;
      }
   }   // Wait for EV5

   I2C_Send7bitAddress(I2C_PORT, DS3231_addr, I2C_Direction_Transmitter);   // Send DS3231 slave address
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
   {   // Wait for EV6

      iTimeout--;
      if (!(iTimeout))
      {
         return -1;
      }
   }

   I2C_SendData(I2C_PORT, DS3231_alarm1);   // Send DS3231 alarm 1 register address
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
   {   // Wait for EV8

      iTimeout--;
      if (!(iTimeout))
      {
         return -1;
      }
   }

   I2C_GenerateSTART(I2C_PORT, ENABLE);   // Send repeated START condition (aka Re-START)
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT))
   {   // Wait for EV5

      iTimeout--;
      if (!(iTimeout))
      {
         return -1;
      }
   }

   I2C_Send7bitAddress(I2C_PORT, DS3231_addr, I2C_Direction_Receiver);   // Send DS3231 slave address for READ
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
   {   // Wait for EV6

      iTimeout--;
      if (!(iTimeout))
      {
         return -1;
      }
   }

   for (i = 0; i < 6; i++)
   {
      while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_RECEIVED))
      {   // Wait for EV7 (Byte received from slave)

         iTimeout--;
         if (!(iTimeout))
         {
            return -1;
         }
      }
      buffer[i] = I2C_ReceiveData(I2C_PORT);   // Receive byte
   }

   I2C_AcknowledgeConfig(I2C_PORT, DISABLE);   // Disable I2C acknowledgement
   I2C_GenerateSTOP(I2C_PORT, ENABLE);   // Send STOP condition
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_RECEIVED))
   {   // Wait for EV7 (Byte received from slave)

      iTimeout--;
      if (!(iTimeout))
      {
         return -1;
      }
   }

   buffer[i] = I2C_ReceiveData(I2C_PORT);   // Receive last byte

   memcpy(date, &buffer[0], 7);
   return 0;
}

void DS3231_WriteAlarm1RAW(DS3231_date_TypeDef* date)
{
   unsigned int i;
   char buffer[7];
   memcpy(&buffer[0], date, 7);

   for (int i = 3; i < sizeof(buffer); i++)
   {
      buffer[i] |= 0x80;
   }

   int iTimeout = DEF_TIMEOUT_I2C;

   I2C_AcknowledgeConfig(I2C_PORT, ENABLE);   // Enable I2C acknowledge

   I2C_GenerateSTART(I2C_PORT, ENABLE);   // Send START condition
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT))
   {   // Wait for EV5

      iTimeout--;
      if (!(iTimeout))
      {
         return;
      }
   }

   I2C_Send7bitAddress(I2C_PORT, DS3231_addr, I2C_Direction_Transmitter);   // Send DS3231 slave address
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
   {   // Wait for EV6

      iTimeout--;
      if (!(iTimeout))
      {
         return;
      }
   }

   I2C_SendData(I2C_PORT, DS3231_alarm1);   // Send DS3231 seconds register address
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
   {   // Wait for EV8

      iTimeout--;
      if (!(iTimeout))
      {
         return;
      }
   }

   for (i = 0; i < sizeof(buffer); i++)
   {
      I2C_SendData(I2C_PORT, buffer[i]);   // Send DS3231 seconds register address
      iTimeout = DEF_TIMEOUT_I2C;
      while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
      {   // Wait for EV8

         iTimeout--;
         if (!(iTimeout))
         {
            return;
         }
      }
   }
   I2C_GenerateSTOP(I2C_PORT, ENABLE);
}

uint8_t DS3231_ReadTemp(void)
{
   int iTimeout = DEF_TIMEOUT_I2C;
   I2C_AcknowledgeConfig(I2C_PORT, ENABLE);   // Enable I2C acknowledge

   I2C_GenerateSTART(I2C_PORT, ENABLE);   // Send START condition
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT))
   {   // Wait for EV5

      iTimeout--;
      if (!(iTimeout))
      {
         return 255;
      }
   }

   I2C_Send7bitAddress(I2C_PORT, DS3231_addr, I2C_Direction_Transmitter);   // Send DS3231 slave address
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
   {   // Wait for EV6

      iTimeout--;
      if (!(iTimeout))
      {
         return 255;
      }
   }

   I2C_SendData(I2C_PORT, DS3231_tmp_MSB);   // Send DS3231 temperature MSB register address
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
   {   // Wait for EV8

      iTimeout--;
      if (!(iTimeout))
      {
         return 255;
      }
   }

   I2C_GenerateSTART(I2C_PORT, ENABLE);   // Send repeated START condition (aka Re-START)
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT))
   {   // Wait for EV5

      iTimeout--;
      if (!(iTimeout))
      {
         return 255;
      }
   }

   I2C_Send7bitAddress(I2C_PORT, DS3231_addr, I2C_Direction_Receiver);   // Send DS3231 slave address for READ
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
   {   // Wait for EV6

      iTimeout--;
      if (!(iTimeout))
      {
         return 255;
      }
   }

   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_RECEIVED))
   {   // Wait for EV7 (Byte received from slave)

      iTimeout--;
      if (!(iTimeout))
      {
         return 255;
      }
   }
   uint8_t temperature = I2C_ReceiveData(I2C_PORT);   // Receive temperature MSB

   I2C_AcknowledgeConfig(I2C_PORT, DISABLE);   // Disable I2C acknowledgement

   I2C_GenerateSTOP(I2C_PORT, ENABLE);   // Send STOP condition
   iTimeout = DEF_TIMEOUT_I2C;
   while (!I2C_CheckEvent(I2C_PORT, I2C_EVENT_MASTER_BYTE_RECEIVED))
   {   // Wait for EV7 (Byte received from slave)

      iTimeout--;
      if (!(iTimeout))
      {
         return 255;
      }
   }
   return temperature;
}

void DS3231_ReadDate(HRF_date_TypeDef* hrf_date)
{
   DS3231_date_TypeDef raw_date;

   if (DS3231_ReadDateRAW(&raw_date))
   {
      InitI2C();
      DS3231_ReadDateRAW(&raw_date);
   }

   hrf_date->Seconds = (raw_date.seconds >> 4) * 10 + (raw_date.seconds & 0x0f);
   hrf_date->Minutes = (raw_date.minutes >> 4) * 10 + (raw_date.minutes & 0x0f);
   hrf_date->Hours = (raw_date.hours >> 4) * 10 + (raw_date.hours & 0x0f);
   hrf_date->Day = (raw_date.mday >> 4) * 10 + (raw_date.mday & 0x0f);
   hrf_date->Month = (raw_date.month >> 4) * 10 + (raw_date.month & 0x0f);
   hrf_date->Year = (raw_date.year >> 4) * 10 + (raw_date.year & 0x0f) + 2000;
   hrf_date->DOW = raw_date.day_of_week;
}

void DS3231_ReadAlarm1(HRF_date_TypeDef* hrf_date)
{
   DS3231_date_TypeDef raw_date;

   if (DS3231_ReadAlarm1RAW(&raw_date))
   {
      InitI2C();
      DS3231_ReadAlarm1RAW(&raw_date);
   }

   hrf_date->Seconds = (raw_date.seconds >> 4) * 10 + (raw_date.seconds & 0x0f);
   hrf_date->Minutes = (raw_date.minutes >> 4) * 10 + (raw_date.minutes & 0x0f);
   hrf_date->Hours = (raw_date.hours >> 4) * 10 + (raw_date.hours & 0x0f);
   hrf_date->Day = (raw_date.mday >> 4) * 10 + (raw_date.mday & 0x0f);
   hrf_date->Month = (raw_date.month >> 4) * 10 + (raw_date.month & 0x0f);
   hrf_date->Year = (raw_date.year >> 4) * 10 + (raw_date.year & 0x0f) + 2000;
   hrf_date->DOW = raw_date.day_of_week;
}

void DS3231_DateToTimeStr(DS3231_date_TypeDef* raw_date, char* str)
{
   *str++ = (raw_date->hours >> 4) + '0';
   *str++ = (raw_date->hours & 0x0f) + '0';
   *str++ = ':';
   *str++ = (raw_date->minutes >> 4) + '0';
   *str++ = (raw_date->minutes & 0x0f) + '0';
   *str++ = ':';
   *str++ = (raw_date->seconds >> 4) + '0';
   *str++ = (raw_date->seconds & 0x0f) + '0';
   *str++ = 0;
}

void DS3231_DateToDateStr(DS3231_date_TypeDef* raw_date, char* str)
{
   *str++ = (raw_date->mday >> 4) + '0';
   *str++ = (raw_date->mday & 0x0f) + '0';
   *str++ = '.';
   *str++ = (raw_date->month >> 4) + '0';
   *str++ = (raw_date->month & 0x0f) + '0';
   *str++ = '.';
   *str++ = '2';
   *str++ = '0';
   *str++ = (raw_date->year >> 4) + '0';
   *str++ = (raw_date->year & 0x0f) + '0';
   *str++ = 0;
}

void ConvertDsToRtc(const DS3231_date_TypeDef* pDateI, RTC_t* pDateO)
{
   pDateO->sec = Bcd2Bin(pDateI->seconds);
   pDateO->min = Bcd2Bin(pDateI->minutes);
   pDateO->hour = Bcd2Bin(pDateI->hours);
   pDateO->mday = Bcd2Bin(pDateI->mday);
   pDateO->month = Bcd2Bin(pDateI->month);
   pDateO->year = Bcd2Bin(pDateI->year) + 2000;
   pDateO->wday = Bcd2Bin(pDateI->day_of_week);
}

void ConvertRtcToDs(const RTC_t* pDateI, DS3231_date_TypeDef* pDateO)
{
   pDateO->seconds = Bin2Bcd(pDateI->sec);
   pDateO->minutes = Bin2Bcd(pDateI->min);
   pDateO->hours = Bin2Bcd(pDateI->hour);
   pDateO->mday = Bin2Bcd(pDateI->mday);
   pDateO->month = Bin2Bcd(pDateI->month);
   pDateO->year = Bin2Bcd(pDateI->year - 2000);
   pDateO->day_of_week = Bin2Bcd(pDateI->wday);
}
