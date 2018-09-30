
//#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "includes.h"
#include "sms_cmd.h"

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

   DP_GSM(" [OK] SMS: \"%s\" \"%s\"\r\n", pSMS->tn.buf, pSMS->txt.buf);
   DP_GSM(" [OK] SMS ANSWER: \"%s\" \r\n", g_asInpDataFrameBuffer);

   SendTXTSMS(g_asInpDataFrameBuffer, (char*)pSMS->tn.buf);   //Отправляем ответное СМС.

   memset(g_asInpDataFrameBuffer, 0, sizeof(g_asInpDataFrameBuffer));

   return 0;
}

/*
Return:
-1 ошибка чтения СМС
0 нет СМС
1 есть СМС
*/
int CheckSmsCommand(void)
{
   u64 mask;
   SMS_RESPONSE ret = PDU_SMGL(&mask);

   if (ret < 0)
   {
      DP_GSM("D_SMS ERR: ");
      switch (ret)
      {
      case UNSPECIFIED_ERROR:
         DP_GSM("UNSPECIFIED ERROR\r\n");
         return -1;

      case SIM_PIN_REQUIRED:
         DP_GSM("SIM PIN REQUIRED\r\n");
         return -1;

      case OPERATION_NOT_ALLOWED:
         DP_GSM("OPERATION NOT ALLOWED\r\n");
         return -1;

      case PARESER_FREEZES:
         DP_GSM("PARESER FREEZES\r\n");
         return -1;

      case DMA_OVERFLOW:
         DP_GSM("DMA OVERFLOW\r\n");
         ReStartDmaGsmUsart();   //Перезапускаем DMA.
         g_bDmaGsmFail = FALSE;   //решили проблему с DMA - сбрасывем флаг
         PDU_SMGD(6);   // Delete all SMS
         break;
      }
   }

   return (ret == SMS_TRUE);

#if 0
    for(n=0; n<64 && ret>0; n++) 
    {
        if((mask & bit) != 0) 
        {
          ret--;
              
          sms_cmd.number = n+1;
          sms_cmd.txt.buf = (uint8_t*)g_aucOutDataFrameBuffer;
          if(g_bDmaGsmFail == FALSE) {    //Проверим переполнение DMA.
             PDU_SMGR(&sms_cmd, 5);         
             if(!(SMS_Exec(&sms_cmd))){
               res = 1;
             }
          }else{
             res = 1;
          }
        }
        bit <<= 1;
    }

    if(g_bDmaGsmFail == TRUE)                             //если мы приняли длинную смс и вылетели с переполнением DMA
    {   
        ReStartDmaGsmUsart();                           //Перезапускаем DMA.        
        g_bDmaGsmFail = FALSE;                            //решили проблему с DMA - сбрасывем флаг
        res = 1; 
        sms_cmd.number = 1;
    }
    if(res)     //Если имеются СМС, то удалим их.
    {
      while(sms_cmd.number)
      {
        osDelay(SLEEP_MS_1000);
        
        uint8_t cnt = MC_COUNT;
        while(cnt) {
            if(!(PDU_SMGD(sms_cmd.number))) {
              break;
            }
            osDelay(SLEEP_MS_100);
            cnt--;
        }
        
        sms_cmd.number--;
        if(!(cnt)) {
           return -1;
        }
      }
    }
    if(g_bDmaGsmFail == TRUE){
      return -1;
    }
#endif
}

void SendMessage(char* ptrTel)
{
   sprintf(g_asInpDataFrameBuffer, "FindMe вышел на связь. Причина выхода: нажата тревожная кнопка.");

   DP_GSM("D_SMS MESSAGE: <<<\r\n");
   DP_GSM(g_asInpDataFrameBuffer);
   DP_GSM(">>>\r\n");
   SendTXTSMS(g_asInpDataFrameBuffer, ptrTel);
}

_Bool gEventTrue = 0;

_Bool getEventTrue(void)
{
   return gEventTrue;
}

void setEventTrue(void)
{
   gEventTrue = 1;
}

void resetEventTrue(void)
{
   gEventTrue = 0;
}

_Bool alarmTrue(void)
{
   char strNumUserTel[SIZE_TEL];
   if (GetUserTel(strNumUserTel) == 0 || getEventTrue())
      return 0;

   if ((GetMaskMessageUser() & fBUTTON_CALL_TEL) || (GetMaskMessageUser() & fBUTTON_MESS_TEL))
   {
      if (GetStatusReset() == BUTTON_RESET)
      {
         DPS("D_Alarm == TRUE\r\n");
         return 1;
      }
   }
   return 0;
}

/* Сформируем и отправим СМС пользователю о ожидании регистрации */
void sendSmsForUser911(char* ptrTel)
{
   if (strlen(ptrTel) > MIN_SIZE_NUMBER_SMS)
   {
      extern const char strMsgRusSmsRegUser[];
      DP_GSM(" [OK] SMS ANSWER FOR USER FM911: \"%s\" \r\n", strMsgRusSmsRegUser);
      SendTXTSMS((char*)strMsgRusSmsRegUser, ptrTel);   //Отправляем ответное СМС.
   }
}