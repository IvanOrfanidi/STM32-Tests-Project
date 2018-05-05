
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "includes.h"
#include "gsm_sms.h"

u8 ucSmsFormat = 0;   //Формат номера откуда пришла СМС и куда её переслать(интернациональный, национальный)

int state_sms = 0;

#define END 0x1A
int SMS_Exec(SMS_INFO* pSMS);

int my_strcpy(char* dst, const char* src)
{
   int n = 0;

   if (dst != 0 && src != 0)
   {
      while (src[n] != 0)
      {
         dst[n] = src[n];
         n++;
      }
      dst[n] = src[n];
   }

   return n;
}

int koi8_to_ucs2(char* out, u8* inp, u8 len)
{
   u8 n = 0;
   u16 m = 0;
   u16 uc = 0;

   for (; n < len; n++)
   {
      if (inp[n] == 0)
         break;

      uc = inp[n];

      if (uc == 'ё')
         uc = 0x0451;
      else if (uc == 'Ё')
         uc = 0x0401;
      else if (uc >= 0x00C0)
      {
         uc = uc + 0x00410 - 0x00C0;
      }

      sprintf(out + m, "%04X", uc);
      m += 4;
   }
   out[m] = 0;

   return m;
}

u8 get_D16(u8* pBuf)
{
   u8 ret = 0;

   if (pBuf[0] >= '0' && pBuf[0] <= '9')
   {
      ret = pBuf[0] - '0';
   }
   else if (pBuf[0] >= 'A' && pBuf[0] <= 'F')
   {
      ret = pBuf[0] - 'A' + 10;
   }

   ret *= 16;

   if (pBuf[1] >= '0' && pBuf[1] <= '9')
   {
      ret += (pBuf[1] - '0');
   }
   else if (pBuf[1] >= 'A' && pBuf[1] <= 'F')
   {
      ret += (pBuf[1] - 'A' + 10);
   }

   return ret;
}

int ucs2_to_koi8(u8* out, u8* inp, int size)
{
   int n, m;
   u32 cc = 0;
   u32 cs = 0;

   for (n = 0, m = 0; n < size; n++)
   {
      cc = get_D16(inp + m);
      m += 2;
      cs = get_D16(inp + m);
      m += 2;

      if (cc != 0)
      {
         out[n] = cs + 0xB0;
      }
      else
      {
         out[n] = cs;
      }
   }
   out[n] = 0;

   return n;
}

int PDU_UnPackTN(SMS_TN* tn, u8* pInp, u8 size)
{
   u8 n;

   for (n = 0; n < size;)
   {
      /*if(pInp[n] < '0' || pInp[n] > '9') {
          if(pInp[n] != 'F') return 0;
      } else*/
      {
         tn->buf[n + 1] = pInp[n];
      }
      n++;

      /*if(pInp[n] < '0' || pInp[n] > '9') return 0;*/
      tn->buf[n - 1] = pInp[n];
      n++;
   }

   tn->buf[size] = 0;
   tn->size = size;

   return size;
}

int PDU_PackTN(char* out, SMS_TN* tn)
{
   u8 n;

   for (n = 0; n < tn->size; n += 2)
   {
      out[n] = tn->buf[n + 1];
      out[n + 1] = tn->buf[n];
   }

   if ((tn->size & 1) != 0)
   {
      out[tn->size - 1] = 'F';
   }

   return n;
}

int PDU_SetHeader(char* buf, SMS_TN* tn)
{
   int len = 0;

   if (tn->size <= 0)
      return -1;

   sprintf(buf, "001100%02X%02X", tn->size, ucSmsFormat);   // header = 8 + tn_size = (2)
   len = PDU_PackTN(buf + 10, tn);
   if (len <= 0)
      return -1;
   return (len + 10);
}

int PDU_TextUnCode7(u8* pOut, u8* pInp, int size)
{
   int i, n;
   u8 m = 0, c0 = 0, c1 = 0;

   for (i = 0, n = 0; i < size; i++, n += 2)
   {
      c1 = get_D16(pInp + n);

      pOut[i] = ((c1 << m) | (c0 >> (8 - m))) & 0x7F;

      if (m < 6)
      {
         m++;
      }
      else
      {
         i++;
         pOut[i] = (c1 >> 1) & 0x7F;
         m = 0;
      }

      c0 = c1;
   }
   pOut[i] = 0;

   return i;
}

RET_INFO PDU_SMGD(uint8_t sms_number)
{
   sprintf(g_asCmdBuf, "%s%d", AT_CMGD, sms_number);

   if (mc(g_asCmdBuf, 4, MC_COUNT) == OK)
   {
      return RET_OK;
   }

   return ERR_CMD;
}

int PDU_SMGL(u64* mask)
{
   int ret = 0;
   unsigned int uiRxSize;
   GSM_INFO data_gsm_sms;
   const char cmd[10] = AT_SMGL;
   *mask = 0;
   // выполняем команду
   mc_send(cmd, 0, 10);

   for (;;)
   {
      // ждём заголовок
      uiRxSize = gsm_parser(cmd, &data_gsm_sms, g_asRxBuf, RX_BUFFER_SIZE, 20);
      if (uiRxSize == 0)
      {
         // timeout
         GSM_DC(cmd, 't');
         break;
      }

#ifdef GSM_MODULE_SIM800
      if (data_gsm_sms.m_type == M_SMGL)
      {
         osDelay(1000);
         uiRxSize = gsm_parser(cmd, &data_gsm_sms, g_asRxBuf, RX_BUFFER_SIZE, 20);
         if ((data_gsm_sms.m_type == M_STRING) && (data_gsm_sms.msg[0].size))
         {
            if (g_bDmaGsmFail == TRUE)
            {
               return M_DMA_ERR * -1;
            }

            u16 len;
            u16 len_1;
            u16 len_2;
            SMS_INFO sms;
            sms.number = 1;
            sms.txt.buf = (u8*)g_asOutDataFrameBuffer;

            // вычисляем смещение до поля с длинной номера пользователя
            len_1 = (get_D16((u8*)g_asRxBuf) + 2) * 2;
            // вычисляем длину номера
            len_2 = get_D16((u8*)g_asRxBuf + len_1);
            // вычисляем смещение до признака кодирования
            len = len_1 + len_2 + 6;
            if ((len_2 & 1) != 0)
               len++;

            // определяем формат номера национальный/интернациональный
            ucSmsFormat = get_D16((uint8_t*)g_asRxBuf + len_1 + 2);

            // декодируем телефонный номер
            if (PDU_UnPackTN(&sms.tn, (u8*)g_asRxBuf + len_1 + 4, len_2) == 0)
            {
               // игнорируем не определённый номер
               DP_GSM("SMS: Error telephone number\r");
               return 0;
            }

            sms.tn.size = len_2;
            sms.txt.size = get_D16((u8*)g_asRxBuf + len + 16);
            if ((get_D16((u8*)g_asRxBuf + len) & 0x08) != 0)
            {
               // 16-битный код
               sms.txt.code = 1;
               sms.txt.size <<= 1;

               // копируем 16-битный код
               // sms.txt.size = ucs2_to_koi8(sms.txt.buf, (u8*)g_asRxBuf + len + 18, sms.txt.size);
               memcpy(sms.txt.buf, g_asRxBuf + len + 18, sms.txt.size);
               sms.txt.buf[sms.txt.size] = 0;
            }
            else
            {
               // 7-битный код
               sms.txt.code = 0;

               // декодируем 7-битный код
               PDU_TextUnCode7(sms.txt.buf, (u8*)g_asRxBuf + len + 18, sms.txt.size);
            }

            SMS_Exec(&sms);
            PDU_SMGD(6);   // Delete all SMS
            return 0;
         }
      }
#endif
      if ((data_gsm_sms.m_type == M_SMGL) || (data_gsm_sms.m_type == M_P_CMGL))
      {
         if (data_gsm_sms.msg[0].var < 63)
         {
            *mask |= (u64)1 << (data_gsm_sms.msg[0].var - 1);
         }
         ret++;
      }
      if (data_gsm_sms.m_type == M_OK)
      {
         GSM_DC(cmd, '0');
         break;
      }
      if (data_gsm_sms.m_type == M_ERROR || data_gsm_sms.m_type == M_ABORTED)
      {
         GSM_DC(cmd, '4');
         break;
      }
      if (data_gsm_sms.m_type == M_CME_ERROR)
      {
         GSM_DC(cmd, '5');
         break;
      }
      if (g_bDmaGsmFail == TRUE)
      {
         return M_DMA_ERR * -1;
      }
   }

   return ret;
}

int PDU_PartOfThePack7(char* out, SMS_INFO* inp, u8 id, u8 total, u8 part)
{
   u8 txt_size_m = 7;
   int txt_size_n = 0, txt_size_i = 0;

   txt_size_i = PDU_SetHeader(out, &inp->tn);
   /*
   char msg_buf[10];
   sprintf(msg_buf, "TXTSIZEI=%d\r\n", txt_size_i);
   DP_GSM(msg_buf);
   */

   if (txt_size_i <= 0)
      return -1;

   out[2] = '5';   // признак длинного смс

   sprintf(out + txt_size_i, "0000AD%02X050003%02X%02X%02X", inp->txt.size + 7, id, total, part);
   txt_size_i += 20;

   sprintf(out + txt_size_i,
           "%02X",
           ((inp->txt.buf[txt_size_n] >> (txt_size_m - 1)) | (inp->txt.buf[txt_size_n] << (8 - txt_size_m))) & 0xFF);
   txt_size_i += 2;
   txt_size_m = 1;
   txt_size_n++;

   for (;;)
   {
      if (txt_size_n >= inp->txt.size)
         break;

      sprintf(out + txt_size_i,
              "%02X",
              ((inp->txt.buf[txt_size_n] >> (txt_size_m - 1)) | (inp->txt.buf[txt_size_n + 1] << (8 - txt_size_m))) &
                 0xFF);

      txt_size_i += 2;
      txt_size_n++;

      if (txt_size_m >= 7)
      {
         txt_size_m = 1;
         txt_size_n++;
      }
      else
      {
         txt_size_m++;
      }
   }

   out[txt_size_i] = 0;

   return txt_size_i;
}

int PDU_SMGR(SMS_INFO* sms, u8 second)
{
   i8 count = 1;
   state_sms = 0;
   GSM_INFO data_gsm_sms;
   unsigned int uiRxSize;

   sprintf(g_asCmdBuf, "at^smgr=%d", sms->number);

   for (; count > 0; count--)
   {
      switch (state_sms)
      {
      case 0:
         // выполняем команду
         mc_send(g_asCmdBuf, 0, 10);
         state_sms = 1;

      case 1:
         // ждём заголовок
         uiRxSize = gsm_parser(g_asCmdBuf, &data_gsm_sms, g_asRxBuf, RX_BUFFER_SIZE, second);
         if (uiRxSize == 0)
         {
            // timeout
            GSM_DC(g_asCmdBuf, 't');
            state_sms = 0;
            break;
         }

         if (data_gsm_sms.m_type == M_SMGR)
         {
            u16 len;
            u16 len_1;
            u16 len_2;

            // принимаем daнные
            uiRxSize = gsm_gets(g_asRxBuf, RX_BUFFER_SIZE, second);
            if (uiRxSize == 0)
            {
               // timeout
               GSM_DC(g_asCmdBuf, 't');
               state_sms = 0;
               break;
            }

            // вычисляем смещение до поля с длинной номера пользователя
            len_1 = (get_D16((u8*)g_asRxBuf) + 2) * 2;
            // вычисляем длину номера
            len_2 = get_D16((u8*)g_asRxBuf + len_1);
            // вычисляем смещение до признака кодирования
            len = len_1 + len_2 + 6;
            if ((len_2 & 1) != 0)
               len++;

            // определяем формат номера национальный/интернациональный
            ucSmsFormat = get_D16((uint8_t*)g_asRxBuf + len_1 + 2);
            DP_GSM("SMS: Format %02X\r", ucSmsFormat);

            // декодируем телефонный номер
            if (PDU_UnPackTN(&sms->tn, (u8*)g_asRxBuf + len_1 + 4, len_2) == 0)
            {
               // игнорируем не определённый номер
               DP_GSM("SMS: Error telephone number\r");
               state_sms = 2;
               count = 0;
               break;
            }

            sms->tn.size = len_2;
            sms->txt.size = get_D16((u8*)g_asRxBuf + len + 16);
            if ((get_D16((u8*)g_asRxBuf + len) & 0x08) != 0)
            {
               // 16-битный код
               sms->txt.code = 1;
               sms->txt.size <<= 1;

               // копируем 16-битный код
               // sms->txt.size = ucs2_to_koi8(sms->txt.buf, (u8*)g_asRxBuf + len + 18, sms->txt.size);
               memcpy(sms->txt.buf, g_asRxBuf + len + 18, sms->txt.size);
               sms->txt.buf[sms->txt.size] = 0;
            }
            else
            {
               // 7-битный код
               sms->txt.code = 0;

               // декодируем 7-битный код
               PDU_TextUnCode7(sms->txt.buf, (u8*)g_asRxBuf + len + 18, sms->txt.size);
            }
            state_sms = 2;
         }
         else if (data_gsm_sms.m_type == M_OK)
         {
            GSM_DC(g_asCmdBuf, '0');
            return 0;
         }
         else if (data_gsm_sms.m_type == M_ERROR || data_gsm_sms.m_type == M_ABORTED)
         {
            GSM_DC(g_asCmdBuf, '4');
            // return -1;
            osDelay(SLEEP_MS_1000);
         }
         break;

      case 2:
         // ждём  OK
         gsm_parser(g_asCmdBuf, &data_gsm_sms, g_asRxBuf, RX_BUFFER_SIZE, second);
         if (data_gsm_sms.m_type == M_OK)
         {
            GSM_DC(g_asCmdBuf, '0');
            return 0;
         }

         GSM_DC(g_asCmdBuf, 't');
         state_sms = 0;
         break;
      }
   }

   return -1;
}

u16 GSM_specialSymbol(u8 sym)
{
   if (sym == '{')
      return 0x1B28;
   if (sym == '}')
      return 0x1B29;
   if (sym == '[')
      return 0x1B3C;
   if (sym == ']')
      return 0x1B3E;
   if (sym == '^')
      return 0x1B14;
   if (sym == '\\')
      return 0x1B2F;
   if (sym == '|')
      return 0x1B40;
   if (sym == '~')
      return 0x1B3D;

   return sym;
}

int PDU_Pack7(char* out, SMS_INFO* inp)
{
   char m = 1;
   int n = 0, i = 0;

   i = PDU_SetHeader(out, &inp->tn);
   if (i <= 0)
      return -1;

   sprintf(out + i, "0000AD%02X", inp->txt.size);
   i += 8;

   for (;;)
   {
      if (n >= inp->txt.size)
         break;

      sprintf(out + i, "%02X", ((inp->txt.buf[n] >> (m - 1)) | (inp->txt.buf[n + 1] << (8 - m))) & 0xFF);

      i += 2;
      n++;

      if (m >= 7)
      {
         m = 1;
         n++;
      }
      else
      {
         m++;
      }
   }

   out[i] = 0;

   return i;
}

int PDU_PartOfThePackUcs2(char* out, SMS_INFO* inp, u8 id, u8 total, u8 part)
{
   u16 header_len = 0;
   u16 size = 0;
   u8 tmp;

   header_len = PDU_SetHeader(out, &inp->tn);
   if (header_len <= 0)
      return -1;

   size = koi8_to_ucs2(out + header_len + 20, inp->txt.buf, 67);
   if (size <= 0)
      return -1;

   out[2] = '5';   // признак длинного смс

   //допишем заголовок
   tmp = out[header_len + 20];
   sprintf(out + header_len, "0008AD%02X050003%02X%02X%02X", size / 2 + 6, id, total, part);
   out[header_len + 20] = tmp;

   out[header_len + 20 + size] = 0;

   return (header_len + 20 + size);
}

int PDU_CMGS(char* buf, int ms_size)
{
   int res = -1;
   char ch;
   char state = 0;
   GSM_INFO data_gsm_sms1;
   unsigned int uiRxSize;

   sprintf(g_asCmdBuf, "at+cmgs=%d", ((ms_size - 2) >> 1));

   // выполняем команду
   mc_send(g_asCmdBuf, 0, 10);
   uint16_t timeout_cmd = 60000;
   // ждём приглашение "> "
   for (;;)
   {
      timeout_cmd--;
      if (!(timeout_cmd))
      {
         return M_ERROR;
      }

      if (gsm_read(&ch, 1, 5) == 0)
         return -1;

      if (state == 0)
      {
         if (ch == '>')
            state = 1;
      }
      else
      {
         if (ch == ' ')
            break;
      }
   }

   // передаем текст SMS-сообщения
   buf[ms_size] = END;
   ms_size++;
   gsm_write(buf, ms_size, 2);

   memset(g_asCmdBuf, 0, sizeof(g_asCmdBuf));
   memset(g_asRxBuf, 0, sizeof(g_asRxBuf));

   // игнорим информационное сообщение и ждём OK
   for (;;)
   {
      osDelay(100);
      uiRxSize = gsm_parser(g_asCmdBuf, &data_gsm_sms1, g_asRxBuf, RX_BUFFER_SIZE, 10);
      if (uiRxSize == 0)
      {
         // timeout
         DPS("SMS t");
         state = 0;
         break;
      }

      if (data_gsm_sms1.m_type == M_OK)
      {
         DPS("SMS 0");
         break;
      }
      if (data_gsm_sms1.m_type == M_ERROR || data_gsm_sms1.m_type == M_ABORTED)
      {
         DPS("SMS 4");
         break;
      }
   }

   osDelay(100);
   return res;
}

int PDU_PackUcs2(char* out, SMS_INFO* inp)
{
   u8 i = 0;
   u8 size = 0;
   char tmp[4];

   i = PDU_SetHeader(out, &inp->tn);
   if (i <= 0)
      return -1;

   strcpy(out + i, "0008AD");   // header              4
   i += 6;
   size = koi8_to_ucs2(out + i + 2, inp->txt.buf, 70);   // txt_conv_to_utf     12
   if (size <= 0)
      return -1;

   sprintf(tmp, "%02X", (size >> 1));   // txt_size            2
   memcpy(out + i, tmp, 2);

   return (size + i + 2);
}

int PDU_SendSms(SMS_INFO* sms, char* sms_buf)
{
   u16 sms_len_max;
   u16 sms_part_len;
   u8 sms_block_id = 1;
   u16 sep_symb_num = 0;
   int sms_size;
   int inp_size;
   int res;
   res = -1;
   SMS_INFO inp;

   sms->txt.code = 0;
   //проверяем есть ли русские символы в смс
   for (u16 i = 0; i < sms->txt.size; i++)
      if (sms->txt.buf[i] > 0x7F && GSM_specialSymbol(sms->txt.buf[i]) < 0x1b00)
      {
         sms->txt.code = 1;
         break;
      }

   if (sms->txt.code != 0)
   {
      // 16-битный код
      sms_len_max = 70;
      sms_part_len = 67;
   }
   else
   {
      // 7-битный код
      sms_part_len = 153;
      sms_len_max = 160;
      //считаем число спец. символов
      for (u16 i = 0; i < sms->txt.size; i++)
         if (GSM_specialSymbol(sms->txt.buf[i]) >= 0x1b00)
            sep_symb_num++;
      sms->txt.buf[sms->txt.size + sep_symb_num] = 0;
      //вставляем спец. символы gsm (начинаются с 0x1b)
      for (i16 i = sms->txt.size - 1 + sep_symb_num; sep_symb_num; i--)
      {
         u16 code = GSM_specialSymbol(sms->txt.buf[i - sep_symb_num]);
         if (code >= 0x1b00)
         {
            sms->txt.buf[i--] = (u8)(code & 0x00FF);
            sms->txt.buf[i] = code >> 8;
            sep_symb_num--;
            sms->txt.size++;
         }
         else
            sms->txt.buf[i] = sms->txt.buf[i - sep_symb_num];
      }
   }

   if (sms->txt.size > sms_len_max)   //Если СМС длинное
   {   //Отпарвка длинного СМС
      char part = 1;
      char total = sms->txt.size / sms_part_len + ((sms->txt.size % sms_part_len) != 0);

      memcpy(&inp.tn, &sms->tn, sizeof(SMS_TN));
      inp.txt.buf = sms->txt.buf;
      inp.txt.size = sms_part_len;

      for (inp_size = sms->txt.size; inp_size > 0; inp.txt.buf += sms_part_len, inp_size -= sms_part_len)
      {
         if (inp_size <= sms_part_len)
            inp.txt.size = inp_size;
         if (sms->txt.code != 0)
            sms_size = PDU_PartOfThePackUcs2(sms_buf, &inp, sms_block_id, total, part++);
         else
            sms_size = PDU_PartOfThePack7(sms_buf, &inp, sms_block_id, total, part++);
         /*
         char msg_buf[10];
         sprintf(msg_buf, "SMSSIZE=%d\r\n", sms_size);
         DP_GSM(msg_buf);
         */
         if (sms_size > 0)
         {
            PDU_CMGS(sms_buf, sms_size);
         }
      }
   }
   else   //Если СМС короткое
   {   //Отпарвка короткого СМС
      if (sms->txt.code != 0)
      {
         sms_size = PDU_PackUcs2(sms_buf, sms);
      }
      else
      {
         sms_size = PDU_Pack7(sms_buf, sms);
      }
      if (sms_size > 0)
      {
         PDU_CMGS(sms_buf, sms_size);
         res = 0;
      }
   }

   return res;
}

int SendTXTSMS(u8* pbuf, char* pTelSMS)
{
   SMS_INFO sms;

   my_strcpy((char*)sms.tn.buf, pTelSMS);
   sms.tn.size = strlen((char*)sms.tn.buf);
   sms.txt.buf = (u8*)g_asInpDataFrameBuffer;
   my_strcpy((char*)sms.txt.buf, (char const*)pbuf);
   sms.txt.size = strlen((char*)sms.txt.buf);
   if (sms.txt.size && sms.tn.size > 7)
   {
      PDU_SendSms(&sms, g_asOutDataFrameBuffer);
      return OK;
   }
   else
      return FAIL;
}