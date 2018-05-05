#include <stdio.h>
#include <string.h>

#include "gsm_info.h"

GSM_INFO data_smond;
extern char g_asRxBuf[];
// extern unsigned int uiRxSize;

typedef struct
{
   int IndexOperator;
   int IndexRN;

   int IndexMcc;
   int IndexMnc;
   int IndexRxlen;
   int IndexCellid;
   int IndexLac;
} T_VAR;

int get_opInfo(const char* cmd, GSM_INFO* gsm, int second)
{
   int ret = ERR_CMD;
   int count = 3;
   unsigned int uiRxSize;

   memset(&g_base_station, 0, sizeof(TS_MOND));

   for (; count > 0; count--)
   {
      // выполняем команду
      if (ret == ERR_CMD)
      {
         mc_send(cmd, 0, 0);
         ret = 0;
      }

#ifdef GSM_MODULE_SIM800
      data_smond.inf = &g_base_station;
      uiRxSize = gsm_get_location(&data_smond, second);
      if (uiRxSize == 0)
      {
         memcpy(gsm, &data_smond, sizeof(GSM_INFO));
         return RET_OK;
      }
      else
      {
         if (uiRxSize == -1)
         {
            ret = ERR_CMD;
         }
      }
#else
      do
      {
         uiRxSize = gsm_parser(cmd, &data_smond, g_asRxBuf, RX_BUFFER_SIZE, second);
         if (uiRxSize == 0)
         {   // истекло время ожидания ответа на команду
            GSM_DC(cmd, 't');
            return (ERR_TIMEOUT);
         }

         switch (data_smond.m_type)
         {
         case M_COPS:
         case M_SMOND:
            if (data_smond.m_echo != 0)
            {
               memcpy(gsm, &data_smond, sizeof(GSM_INFO));
            }
            break;

         case M_OK:
            // команда выполнена
            GSM_DC(cmd, '0');
            return RET_OK;

         case M_ERROR:
         case M_CME_ERROR:
         case M_ABORTED:
            // получен ответ неверная команда
            GSM_DC(cmd, '4');
            ret = ERR_CMD;
            break;

         case M_SYSSTART:
            // GSM модуль перезагрузился во время выполнения команды
            return (ERR_RESTART);
         }

      } while (ret == 0);
#endif
      osDelay(1000);
   }

   return ERR_CMD;
}

int gsm_get_location(GSM_INFO* gsm, int second)
{
#if UART_GSM == 1
   char* pRxBufferGSM = (char*)(g_aucRxBufferUSART1);
#endif

#if UART_GSM == 2
   char* pRxBufferGSM = (char*)(g_aucRxBufferUSART2);
#endif

#if UART_GSM == 3
   char* pRxBufferGSM = (char*)(g_aucRxBufferUSART3);
#endif

   u8 count = 0;
   int iTemp = 0;
   // Find "Operator" and "\r\n"
   char* pFindParam1;
   char* pFindParam2;
   char* pFindEcho = strstr(pRxBufferGSM, "cnetscan");
   T_VAR stVarible;

   memset(&stVarible, 0, sizeof(stVarible));
   while (1)
   {
      if (pFindEcho < 0)
      {   // find Echo
         pFindEcho = strstr(pRxBufferGSM, "cnetscan");
      }
      pFindParam2 = strstr(&pRxBufferGSM[stVarible.IndexOperator], "Operator:");
      if (pFindParam2 > 0)
      {
         count++;
         stVarible.IndexOperator += *pFindParam2 + strlen("Operator");
         pFindParam1 = strstr(&pRxBufferGSM[stVarible.IndexRN], "\r\n");
         if (!(pFindParam1))
         {
            break;
         }
         else
         {
            stVarible.IndexRN += *pFindParam1 + strlen("\r\n");
         }
      }
      else
      {
         osDelay(1000);
         second--;
         if ((!(second)) || (count))
         {
            break;
         }
      }
   }
   //Если не нашли.
   if (!(count))
   {
      osDelay(1000);
      pFindEcho = strstr(pRxBufferGSM, "cnetscan");
      if (pFindEcho)
      {
         return 1;
      }
      else
      {
         return -1;
      }
   }

   //Есть LBS
   memset(&stVarible, 0, sizeof(stVarible));
   for (u8 Index = 0; Index < count; Index++)
   {
      char temp[6];
      // find MCC
      pFindParam1 = strstr(&pRxBufferGSM[stVarible.IndexMcc], "MCC:");
      pFindParam1 += sizeof("MCC:") - 1;
      stVarible.IndexMcc += *pFindParam1;

      for (int i = 0; i < sizeof(temp); i++)
      {
         if (*pFindParam1 == ',')
         {
            temp[i] = 0;
            break;
         }
         temp[i] = *pFindParam1;
         pFindParam1++;
      }

      for (u8 i = 0; i < strlen(temp); i++)
      {
         iTemp = calculate(0, 10, temp[i], iTemp);
      }

      gsm->inf->station[Index].mcc = iTemp;   //
      iTemp = 0;

      // find MNC
      pFindParam1 = strstr(&pRxBufferGSM[stVarible.IndexMnc], "MNC:");
      pFindParam1 += sizeof("MNC:") - 1;
      stVarible.IndexMnc += *pFindParam1;
      for (int i = 0; i < sizeof(temp); i++)
      {
         if (*pFindParam1 == ',')
         {
            temp[i] = 0;
            break;
         }
         temp[i] = *pFindParam1;
         pFindParam1++;
      }

      for (u8 i = 0; i < strlen(temp); i++)
      {
         iTemp = calculate(0, 10, temp[i], iTemp);
      }
      gsm->inf->station[Index].mnc = iTemp;   // atof(temp);
      iTemp = 0;

      // find Rxlev
      pFindParam1 = strstr(&pRxBufferGSM[stVarible.IndexRxlen], "Rxlev:");
      pFindParam1 += sizeof("Rxlev:") - 1;
      stVarible.IndexRxlen += *pFindParam1;
      for (int i = 0; i < sizeof(temp); i++)
      {
         if (*pFindParam1 == ',')
         {
            temp[i] = 0;
            break;
         }
         temp[i] = *pFindParam1;
         pFindParam1++;
      }

      for (u8 i = 0; i < strlen(temp); i++)
      {
         iTemp = calculate(0, 10, temp[i], iTemp);
      }
      gsm->inf->station[Index].rxlev = iTemp;
      iTemp = 0;

      // find Cellid
      pFindParam1 = strstr(&pRxBufferGSM[stVarible.IndexCellid], "Cellid:");
      pFindParam1 += sizeof("Cellid:") - 1;
      stVarible.IndexCellid += *pFindParam1;
      for (int i = 0; i < sizeof(temp); i++)
      {
         if (*pFindParam1 == ',')
         {
            temp[i] = 0;
            break;
         }
         temp[i] = *pFindParam1;
         pFindParam1++;
      }

      for (u8 i = 0; i < strlen(temp); i++)
      {
         iTemp = calculate(0, 16, temp[i], iTemp);
      }
      gsm->inf->station[Index].cell = iTemp;
      iTemp = 0;

      // find Lac
      pFindParam1 = strstr(&pRxBufferGSM[stVarible.IndexLac], "Lac:");
      pFindParam1 += sizeof("Lac:") - 1;
      stVarible.IndexLac += *pFindParam1;
      for (int i = 0; i < sizeof(temp); i++)
      {
         if (*pFindParam1 == ',')
         {
            temp[i] = 0;
            break;
         }
         temp[i] = *pFindParam1;
         pFindParam1++;
      }

      for (u8 i = 0; i < strlen(temp); i++)
      {
         iTemp = calculate(0, 16, temp[i], iTemp);
      }
      gsm->inf->station[Index].lac = iTemp;
      iTemp = 0;
   }

   gsm->count = count;   //Количество базовых станций
   gsm->inf->ta = 0;   //В SIM800 не используется.
   return 0;
}

#ifdef TEMPERATURE_GSM
int gsm_getTemperature(GSM_INFO* pDataGsmInfo)
{
   GSM_INFO out;

   mc("at^sctm=0,1", 5, MC_COUNT);
   mc_get("at^sctm?", M_SCTM, &out, 3, 8);

   if (out.m_type == M_SCTM)
   {
      DP_GSM("D_SCTM: %d,%d,%d\r\n", out.msg[0].var, out.msg[1].var, out.msg[2].var);
      if (out.count == 3)
      {
         pDataGsmInfo->s8Temperatur = out.msg[2].var;
         DP_GSM("D_TMPR: %+d\r\n", pDataGsmInfo->s8Temperatur);
         return 0;
      }
   }

   return -1;
}
#endif
