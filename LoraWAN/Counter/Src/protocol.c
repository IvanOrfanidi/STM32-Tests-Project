
#include "protocol.h"
#include "rtc.h"
#include "adc.h"
#include "eeprom.h"
#include "lptim.h"
#include <stdlib.h>
#include <string.h>

static uint64_t bit_packing(uint64_t, uint64_t, uint8_t);
static uint64_t Date2Pac(const RTC_DateTypeDef*);
static void incrementDate(T_Archive_Date*);

/*
Сохранение данных в архив (1 раз в час)
@param None
@retval None
*/
void saveDataArchive(void)
{
   /* Archive structur */
   EEP_ArchiveTypeDef stArchive;

   /* Time and Date structur */
   RTC_TimeTypeDef stTime;
   RTC_DateTypeDef stDate;

   /* Get current date */
   extern RTC_HandleTypeDef hrtc;
   HAL_RTC_GetTime(&hrtc, &stTime, RTC_FORMAT_BIN);
   HAL_RTC_GetDate(&hrtc, &stDate, RTC_FORMAT_BIN);

   stArchive.Date.Hours = stTime.Hours;   // час измерения

   // дата измерения
   stArchive.Date.Month = stDate.Month;
   stArchive.Date.Date = stDate.Date;
   stArchive.Date.Year = stDate.Year;
   stArchive.Value.TampState = getTamper1State();   // сотояние тампера
   stArchive.Value.Counter = getLtimCountVal();   // кол-во импульсов

   extern _Bool __DEBUG__;
   if (__DEBUG__)
   {
      printf("Archive Time: %02d:%02d:%02d ", stTime.Hours, stTime.Minutes, stTime.Seconds);
      printf("Archive Date: %02d\\%02d\\%02d\r\n", stDate.Date, stDate.Month, stDate.Year);
      printf("Archive Tamper State: %d\r\n", stArchive.Value.TampState);
      printf("Archive Counter Value = %ld\r\n", stArchive.Value.Counter);
   }

   saveArchiveEeprom(&stArchive);
}

/*
Формирование пакта архива данных (по требованию сервера).
@param структура времени нужной записи(час);
@param структура даты нужной записи;
@param количество записей(до 6);
@param- указатель на приемник данных;
@param размер для приема данных, нужен для проверки размера.

@retval количество байт в пакете (-1 - не корректный размер).
*/
int buildDataArchive(const RTC_TimeTypeDef* pTime,
                     const RTC_DateTypeDef* pDate,
                     uint8_t size,
                     uint8_t* pOutData,
                     uint8_t len)
{
   if (size > MAX_SIZE_PACK_ARCH)
   {
      extern _Bool __DEBUG__;
      if (__DEBUG__)
      {
         printf("Error size archive packet\r\n");
      }
      return ERR_SIZE_COUNT_PACK;   // превышение размера данных для чтения
   }

   if (!(size))
      size = 1;   // 0 — интерпретируется как 1.

   uint8_t tempBuf[SIZE_TEMP_BUF] = { 0 };
   uint8_t n = 0;
   uint64_t data = 0xC000000000000000;
   data = bit_packing(data, pTime->Hours, 57);
   data = bit_packing(data, Date2Pac(pDate), 40);
   data = bit_packing(data, 0, 39);
   data = bit_packing(data, 0, 8);
   uint8_t* ptr_data = (uint8_t*)&data;
   for (uint8_t i = 0, j = (sizeof(data) - 1); i < (sizeof(data) - 1); i++)
   {
      tempBuf[i] = ptr_data[j--];
   }

   EEP_ArchiveTypeDef stArchive;
   stArchive.Date.Hours = pTime->Hours;
   stArchive.Date.Date = pDate->Date;
   stArchive.Date.Month = pDate->Month;
   stArchive.Date.Year = pDate->Year;

   /* Найдем каждую запись в архиве */
   for (uint8_t count_pack = 0; count_pack < size; count_pack++)
   {
      /* ищем запись */
      if (findArchiveEeprom(&stArchive))
      {
         extern _Bool __DEBUG__;
         if (__DEBUG__)
         {
            printf("The entry is not found\r\n");
         }
         if (!(n))
            n = 7;
         break;   // запись не найдена
      }

      extern _Bool __DEBUG__;
      if (__DEBUG__)
      {
         printf("Archive Date Record N%d:  %02dh  %02d\\%02d\\20%02d\r\n",
                (count_pack + 1),
                stArchive.Date.Hours,
                stArchive.Date.Month,
                stArchive.Date.Date,
                stArchive.Date.Year);
         printf(
            "Tamper State: %d, Counter Value: %0.0f\r\n", stArchive.Value.TampState, (float)stArchive.Value.Counter);
      }
      uint64_t data = 0;
      if (!(count_pack))
      {
         data = 0xC000000000000000;
      }

      RTC_DateTypeDef stDate;
      stDate.Month = stArchive.Date.Month;
      stDate.Date = stArchive.Date.Date;
      stDate.Year = stArchive.Date.Year;

      data = bit_packing(data, stArchive.Date.Hours, 57);
      data = bit_packing(data, Date2Pac(&stDate), 40);
      data = bit_packing(data, stArchive.Value.TampState, 39);
      data = bit_packing(data, stArchive.Value.Counter, 8);

      uint8_t* ptr_data = (uint8_t*)&data;
      for (uint8_t i = 0, j = (sizeof(data) - 1); i < (sizeof(data) - 1); i++)
      {
         if (n > len)
         {
            extern _Bool __DEBUG__;
            if (__DEBUG__)
            {
               printf("Error the size of the output buffer\r\n");
            }
            return ERR_SIZE_OUT_BUF;   // превышение размера выходного буфера
         }
         tempBuf[n++] = ptr_data[j--];
      }

      if (count_pack < size)
      {
         incrementDate(&stArchive.Date);
      }
   }

   /* */
   tempBuf[n] = (uint8_t)convert_meas(getMeasVin());

   for (uint8_t i = 0, j = n; i <= n; i++)
   {
      pOutData[i] = tempBuf[j--];
   }

   extern _Bool __TEST__;
   if (__TEST__)
   {
      for (int i = n; i >= 0; i--)
      {
         printf(" %02X", pOutData[i]);
      }
      printf("\r\n");
   }

   return ++n;   //количество байт в пакете.
}

/*
Формирование пакта данных по расписанию (1 раз в сутки по умолчанию).
@param указатель на приемник данных;
@param размер для приема данных, нужен для проверки размера.

@retval количество байт в пакете (-1 - не корректный размер).
*/
int buildDataTimer(uint8_t* pOutData, uint8_t len)
{
   /* Time and Date structur */
   RTC_TimeTypeDef stTime;
   RTC_DateTypeDef stDate;

   if (len != SIZE_TIMER_PACKET)
   {
      extern _Bool __DEBUG__;
      if (__DEBUG__)
      {
         printf("Error the size of the output buffer\r\n");
      }
      return ERR_SIZE_OUT_BUF;
   }

   /* Get current date */
   extern RTC_HandleTypeDef hrtc;
   HAL_RTC_GetTime(&hrtc, &stTime, RTC_FORMAT_BIN);
   HAL_RTC_GetDate(&hrtc, &stDate, RTC_FORMAT_BIN);

   /* Data packing */
   uint64_t data = 0;
   data = bit_packing(data, stTime.Hours, 57);
   data = bit_packing(data, Date2Pac(&stDate), 40);
   data = bit_packing(data, getTamper1State(), 39);
   data = bit_packing(data, getLtimCountVal(), 8);
   data = bit_packing(data, convert_meas(getMeasVin()), 0);

   uint8_t* ptr_data = (uint8_t*)&data;
   for (uint8_t i = 0; i < SIZE_TIMER_PACKET; i++)
   {
      pOutData[i] = ptr_data[i];
   }

   extern _Bool __DEBUG__;
   if (__DEBUG__)
   {
      printf("Current Date: %02dh ", stTime.Hours);
      printf("%02dm %02dd 20%02dy\r\n", stDate.Month, stDate.Date, stDate.Year);
   }

   /* Data Timer Packet */
   extern _Bool __TEST__;
   if (__TEST__)
   {
      for (int i = SIZE_TIMER_PACKET - 1; i >= 0; i--)
      {
         printf(" %02X", pOutData[i]);
      }
      printf("\r\n");
   }

   return SIZE_TIMER_PACKET;
}

/*
Формирование пакта данных по изменению тамперного входа.
@param указатель на приемник данных;
@param размер для приема данных, нужен для проверки размера.

@retval количество байт в пакете (-1 - не корректный размер).
*/
int buildDataTamper(uint8_t* pOutData, uint8_t len)
{
   if (len != SIZE_TAMPER_PACKET)
   {
      extern _Bool __DEBUG__;
      if (__DEBUG__)
      {
         printf("Error the size of the output buffer\r\n");
      }
      return ERR_SIZE_OUT_BUF;   // -1
   }

   int n = 0;
   pOutData[n++] = 0x40;

   /* Date & time packing(in unix timestamp) */
   n += packingCurrSecUnix(&pOutData[n]);

   /* Data packing */
   uint64_t data = 0;
   data = bit_packing(data, getTamper1State(), 39);
   data = bit_packing(data, getLtimCountVal(), 8);
   data = bit_packing(data, convert_meas(getMeasVin()), 0);

   uint8_t* ptr = (uint8_t*)&data;
   for (int i = (sizeof(data) - 4); i >= 0; i--)
   {
      pOutData[n++] = ptr[i];
   }

   /* Data Tamper Packet */
   extern _Bool __TEST__;
   if (__TEST__)
   {
      for (uint8_t i = 0; i < SIZE_TAMPER_PACKET; i++)
      {
         printf(" %02X", pOutData[i]);
      }
      printf("\r\n");
   }

   return SIZE_TAMPER_PACKET;
}

/*
Упаковщик данных для пакета.
@param исходный пакет;
@param данные для упаковки;
@param смещение в байтах, в пакете.

@retval готовый выходной пакет.
*/
static uint64_t bit_packing(uint64_t data_out, uint64_t data_inp, uint8_t bit_offset)
{
   uint64_t data = data_inp;
   data <<= bit_offset;
   data_out |= data;
   return data_out;
}

/*
Преобразование напряжения батарейки к протокольной форме передачи на сервер.
Значение напряжения в диапазоне от 0В до 0,6В. 0 = 0В — 255 = 0,6В
(формула: 3 +  (значение питания батареи * 0,0023529) )
@param напряжение в мВ.

@retval значение напряжения в протокольной форме.
*/
uint64_t convert_meas(uint16_t vin)
{
#define K 2.3529F
   return (uint64_t)((vin - 3000) / K);
}

/*
Преобразование даты к формату единого числа месяц, число, год(30 декабря 2028г = 123028 = 0x01E094)
@param структура даты.

@retval число в формате месяц, число, год.
*/
static uint64_t Date2Pac(const RTC_DateTypeDef* ptrDate)
{
   return (ptrDate->Month * 10000 + ptrDate->Date * 100 + ptrDate->Year);
}

/*
Увеличение даты на один час с обработкой переполнения дней.
@param структура даты архива.

@retval None.
*/
static void incrementDate(T_Archive_Date* pDate)
{
   RTC_DateTypeDef stDate;
   stDate.Date = pDate->Date;
   stDate.Month = pDate->Month;
   stDate.Year = pDate->Year;
   stDate.WeekDay = 0;

   RTC_TimeTypeDef stTime;
   memset(&stTime, 0, sizeof(stTime));
   stTime.Hours = pDate->Hours;

   Sec2Date(&stTime, &stDate, (Date2Sec(&stTime, &stDate) + 3600U));
   pDate->Date = stDate.Date;
   pDate->Month = stDate.Month;
   pDate->Year = stDate.Year;
   pDate->Hours = stTime.Hours;
}

/*
Преобразование текущей даты к протокольной форме передачи на сервер.
@param указатель на приемник данных.

@retval число заполненых байт.
*/
int packingCurrSecUnix(uint8_t* ptr)
{
   /* Get current date */
   extern RTC_HandleTypeDef hrtc;

   /* Time and Date structur */
   RTC_TimeTypeDef stTime;
   RTC_DateTypeDef stDate;
   HAL_RTC_GetTime(&hrtc, &stTime, RTC_FORMAT_BIN);
   HAL_RTC_GetDate(&hrtc, &stDate, RTC_FORMAT_BIN);
   uint32_t cur_sec = Date2Sec(&stTime, &stDate);

   extern _Bool __DEBUG__;
   if (__DEBUG__)
   {
      printf("Unix Timestamp: %d\r\n", cur_sec);
   }

   uint8_t* p = (uint8_t*)&cur_sec;

   /* packing current sec UNIX*/
   int n = 0;
   for (int i = (sizeof(cur_sec) - 1); i >= 0; i--)
   {
      ptr[n++] = p[i];
   }
   return n;
}