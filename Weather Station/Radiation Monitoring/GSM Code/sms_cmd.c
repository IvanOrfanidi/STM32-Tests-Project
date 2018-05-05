
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "includes.h"
#include "sms_cmd.h"

SMS_INFO sms_cmd;
const char strMsgGpsFail[] = "PASS OK\r\nGPS=?\r\nGPS NOT VALID\r\nOK\r\n";

int SMS_Exec(SMS_INFO* pSMS)
{
   uint16_t i;
   uint16_t j;

   //уберем пробелы и символы перевода строки '\r' '\n' и  приводим к символам нижнего регистра.
   for (i = 0, j = 0; i < pSMS->txt.size; i++)
   {
      if (pSMS->txt.buf[i] == ' ' || pSMS->txt.buf[i] == '\n' || pSMS->txt.buf[i] == '\r')
         continue;
      pSMS->txt.buf[j] = pSMS->txt.buf[i];
      j++;
   }
   pSMS->txt.buf[j] = 0;   //Конец строки.
   pSMS->txt.size = strlen((char*)pSMS->txt.buf);

   //Разбираем команду и формируем ответ.
   memset(g_asInpDataFrameBuffer, 0, sizeof(g_asInpDataFrameBuffer));

   parse_cmd(pSMS->txt.buf, (u8*)g_asInpDataFrameBuffer, INTERF_SMS, 0, 0);
   pSMS->txt.buf = (u8*)g_asInpDataFrameBuffer;

   pSMS->txt.size = strlen((char const*)pSMS->txt.buf);

   DP_GSM(" [OK] SMS: \"%s\" \"%s\"\r", pSMS->tn.buf, pSMS->txt.buf);
   DP_GSM(" [OK] SMS ANSWER: \"%s\" \r", g_asInpDataFrameBuffer);

   SendTXTSMS((u8*)g_asInpDataFrameBuffer, (char*)pSMS->tn.buf);   //Отправляем ответное СМС.

   memset(g_asInpDataFrameBuffer, 0, sizeof(g_asInpDataFrameBuffer));

   return 0;
}

RET_INFO CheckSmsCommand(void)
{
   int n;
   int ret;
   u64 mask;
   u64 bit = 1;
   SMS_INFO sms_cmd;
   int res = 0;
   // sms_cmd.number = 1;
   g_bDmaGsmFailAccept = TRUE;
   ret = PDU_SMGL(&mask);
   g_bDmaGsmFailAccept = FALSE;

   if (ret == M_DMA_ERR * -1)
   {   //Если SMS слишком длинная и вызвало переполнение DMA буфера.
      ret = 1;
   }
   // ret=1;
   for (n = 0; n < 64 && ret > 0; n++)
   {
      if ((mask & bit) != 0)
      {
         ret--;

         sms_cmd.number = n + 1;
         sms_cmd.txt.buf = (u8*)g_asOutDataFrameBuffer;
         if (g_bDmaGsmFail == FALSE)
         {   //Проверим переполнение DMA.
            PDU_SMGR(&sms_cmd, 5);
            if (!(SMS_Exec(&sms_cmd)))
            {
               res = 1;
            }
         }
         else
         {
            res = 1;
         }
      }
      bit <<= 1;
   }

   if (g_bDmaGsmFail == TRUE)   //если мы приняли длинную смс и вылетели с переполнением DMA
   {
      ReStartDmaGsmUsart();   //Перезапускаем DMA.
      g_bDmaGsmFail = FALSE;   //решили проблему с DMA - сбрасывем флаг
   }
   if (res)   //Если имеются СМС, то удалим их.
   {
      while (sms_cmd.number)
      {
         osDelay(SLEEP_MS_1000);

         while (PDU_SMGD(sms_cmd.number) != RET_OK)
         {
            osDelay(SLEEP_MS_10);
         }

         sms_cmd.number--;
      }
   }
   if (g_bDmaGsmFail == TRUE)
   {
      //  return ERR_DMA;
   }
   return RET_OK;
}
