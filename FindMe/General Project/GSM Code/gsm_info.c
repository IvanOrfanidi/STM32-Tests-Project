//#include <stdio.h>
#include <string.h>

#include "gsm_info.h"

GSM_INFO data_smond;
extern char g_asRxBuf[];
GSM_PARAM gsm_param = { 99, 99, 0, 0, 0, 0, 0, 0 };

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

int gsm_get_location(GSM_INFO* gsm, int second);   //Получение gsm станций для SIM800
int gsm_get_lbs_active(GSM_INFO* ptr, int second);
static void bubbleSortLbs(GSM_INFO*, uint32_t, GSM_INFO*, uint32_t);

GSM_INFO g_stGsm;
TS_MOND g_save_base_station;
TS_STATION g_stLbsCnetscan[SIZE_LBS];

/*
Сохраняем данные gsm lbs по текущему оператору полученные командой CENG.
Input: структура с gsm данными.
Return: no.
*/
void setInfoLbsData(const GSM_INFO* pGsm)
{
   if (pGsm->count)
   {
      g_stGsm.inf = &g_save_base_station;
      memcpy(&g_stGsm, pGsm, sizeof(GSM_INFO));
   }
}

/*
Получаем ранее сохраненые данные gsm lbs по текущему оператору полученные командой CENG.
Input: структура с gsm данными.
Return:
0 - no data;
1 - data OK.
*/
_Bool getInfoLbsData(GSM_INFO* pGsm)
{
   if (g_stGsm.count)
   {
      memcpy(pGsm, &g_stGsm, sizeof(GSM_INFO));
      return 1;
   }
   return 0;
}

/*
Сохраняем данные gsm lbs по множеству операторов полученные командой CNETSCAN.
Input: структура с gsm данными.
Return: no.
*/
void setAllLbsData(const GSM_INFO* pLbsCnetscan)
{
   memset(&g_stLbsCnetscan, 0, sizeof(g_stLbsCnetscan));
   DP_GSM("LBS from \"CNETSCAN:\"\r\n");
   loop(pLbsCnetscan->count)
   {
      g_stLbsCnetscan[i].rxlev = pLbsCnetscan->inf->station[i].rxlev;
      g_stLbsCnetscan[i].mcc = pLbsCnetscan->inf->station[i].mcc;
      g_stLbsCnetscan[i].mnc = pLbsCnetscan->inf->station[i].mnc;
      g_stLbsCnetscan[i].lac = pLbsCnetscan->inf->station[i].lac;
      g_stLbsCnetscan[i].cell = pLbsCnetscan->inf->station[i].cell;

      DP_GSM("N%d) ", i);
      DP_GSM("MCC:%d ", g_stLbsCnetscan[i].mcc);
      DP_GSM("MNC:%d ", g_stLbsCnetscan[i].mnc);
      DP_GSM("LAC:%04X ", g_stLbsCnetscan[i].lac);
      DP_GSM("CELL:%04X ", g_stLbsCnetscan[i].cell);
      DP_GSM("RXLEV:%d\r\n", g_stLbsCnetscan[i].rxlev);
   }
}

/*
Получаем ранее сохраненые данные gsm lbs по множеству операторов полученные командой CNETSCAN.
Input: структура с gsm данными.
Return:
0 - no data;
1 - data OK.
*/
_Bool getAllLbsData(GSM_INFO* pLbsCnetscan)
{
   pLbsCnetscan->count = 0;
   DP_GSM("LBS from \"CNETSCAN:\"\r\n");
   loop(SIZE_LBS)
   {
      if ((!(g_stLbsCnetscan[i].rxlev)) && (!(g_stLbsCnetscan[i].mcc)) && (!(g_stLbsCnetscan[i].mnc)) &&
          (!(g_stLbsCnetscan[i].lac)) && (!(g_stLbsCnetscan[i].cell)))
      {
         break;   //Если данные по станциям lbs нулевый значит массив данных закончился
      }

      pLbsCnetscan->inf->station[i].rxlev = g_stLbsCnetscan[i].rxlev;
      pLbsCnetscan->inf->station[i].mcc = g_stLbsCnetscan[i].mcc;
      pLbsCnetscan->inf->station[i].mnc = g_stLbsCnetscan[i].mnc;
      pLbsCnetscan->inf->station[i].lac = g_stLbsCnetscan[i].lac;
      pLbsCnetscan->inf->station[i].cell = g_stLbsCnetscan[i].cell;

      DP_GSM("N%d) ", i);
      DP_GSM("MCC:%d ", g_stLbsCnetscan[i].mcc);
      DP_GSM("MNC:%d ", g_stLbsCnetscan[i].mnc);
      DP_GSM("LAC:%04X ", g_stLbsCnetscan[i].lac);
      DP_GSM("CELL:%04X ", g_stLbsCnetscan[i].cell);
      DP_GSM("RXLEV:%d\r\n", g_stLbsCnetscan[i].rxlev);

      pLbsCnetscan->count = i;
   }
   return (pLbsCnetscan->count != 0);
}

/*
Получение дополнительных lbs в обработанном виде для сервера 911.
Input: структура с gsm данными.
Return: количество lbs.
*/
uint32_t additionOtherLbsForFm911(GSM_INFO* pLbsCeng)
{
   GSM_INFO stLbsCnetscan;   // дополнительная структура с множеством данных lbs полученных сnetscan
   TS_MOND base_station;
   stLbsCnetscan.inf = &base_station;
   getAllLbsData(&stLbsCnetscan);   // получаем ствнции lbs полученных сnetscan

   DP_GSM("LBS from \"CENG:\"\r\n");
   loop(SIZE_LBS_FOR_FM911)
   {
      /* икуственно создаем нулевые lbs, чтобы выйти на максимальное количество БС что можно отдать серверу 911 */
      /* если БС слишком мало, то отфильтруем их далее */
      if (i > pLbsCeng->count)
      {
         pLbsCeng->inf->station[i].rxlev = 0;
      }
      DP_GSM("N%d) ", i);
      DP_GSM("MCC:%d ", pLbsCeng->inf->station[i].mcc);
      DP_GSM("MNC:%d ", pLbsCeng->inf->station[i].mnc);
      DP_GSM("LAC:%04X ", pLbsCeng->inf->station[i].lac);
      DP_GSM("CELL:%04X ", pLbsCeng->inf->station[i].cell);
      DP_GSM("RXLEV:%d\r\n", pLbsCeng->inf->station[i].rxlev);
   }
   pLbsCeng->count = SIZE_LBS_FOR_FM911;

   /* Сортируем массив. На выходе получаем массив текущего оператора
     и в нем отсортированные данные по множеству операторов. */
   bubbleSortLbs(pLbsCeng, pLbsCeng->count, &stLbsCnetscan, stLbsCnetscan.count);

   DP_GSM("LBS the sorted:\r\n");
   loop(pLbsCeng->count)
   {
      if (!(pLbsCeng->inf->station[i].rxlev))
      {   //Фильтруем нулевые данные если всех станций слишком мало.
         pLbsCeng->count = i;
         break;
      }
      DP_GSM("N%d) ", i);
      DP_GSM("MCC:%d ", pLbsCeng->inf->station[i].mcc);
      DP_GSM("MNC:%d ", pLbsCeng->inf->station[i].mnc);
      DP_GSM("LAC:%04X ", pLbsCeng->inf->station[i].lac);
      DP_GSM("CELL:%04X ", pLbsCeng->inf->station[i].cell);
      DP_GSM("RXLEV:%d\r\n", pLbsCeng->inf->station[i].rxlev);
   }

   return pLbsCeng->count;
}

/*
Получение дополнительных lbs в обработанном виде для сервера iON.
Input: структура с gsm данными.
Return: количество lbs.
*/
uint32_t additionOtherLbsForIon(GSM_INFO* pLbsCeng)
{
   GSM_INFO stLbsCnetscan;   // дополнительная структура с множеством данных lbs полученных сnetscan
   TS_MOND base_station;
   stLbsCnetscan.inf = &base_station;
   getAllLbsData(&stLbsCnetscan);   // получаем ствнции lbs полученных сnetscan
   uint8_t index = 255;
   static uint8_t index_offset = 0;
   uint16_t mcc;
   uint8_t mnc;

   /* находим все станции что нет в списке CENG */
   for (uint8_t n = 0; n < pLbsCeng->count; n++)
   {
      index = 255;
      for (uint8_t i = index_offset; i < stLbsCnetscan.count; i++)
      {
         if (stLbsCnetscan.inf->station[i].mcc != 0xFFFF && stLbsCnetscan.inf->station[index].mnc != 0xFF)
         {   //Проверка, что уже обрабатывали эти данные

            if (pLbsCeng->inf->station[n].mcc != stLbsCnetscan.inf->station[i].mcc ||
                pLbsCeng->inf->station[n].mnc != stLbsCnetscan.inf->station[i].mnc)
            {
               mnc = stLbsCnetscan.inf->station[i].mnc;
               mcc = stLbsCnetscan.inf->station[i].mcc;
               index = i;
               break;
            }
         }
      }
      if (index != 255)
      {
         break;
      }
   }

   if (index == 255)
   {
      pLbsCeng->count = 0;
      index_offset = 0;
      return 0;
   }

   uint32_t count;
   for (count = 0; count < SIZE_LBS; count++, index++)
   {
      if (index > stLbsCnetscan.count)
         break;
      if (stLbsCnetscan.inf->station[index].mcc == mcc && stLbsCnetscan.inf->station[index].mnc == mnc)
      {
         pLbsCeng->inf->station[count].rxlev = stLbsCnetscan.inf->station[index].rxlev;
         pLbsCeng->inf->station[count].mcc = stLbsCnetscan.inf->station[index].mcc;
         pLbsCeng->inf->station[count].mnc = stLbsCnetscan.inf->station[index].mnc;
         pLbsCeng->inf->station[count].lac = stLbsCnetscan.inf->station[index].lac;
         pLbsCeng->inf->station[count].cell = stLbsCnetscan.inf->station[index].cell;
      }
      else
      {
         break;
      }
   }

   index_offset = index;

   pLbsCeng->count = count;
   return count;
}

/*
Пузырьковая сортировка структур lbs данных.
Input: две структуры lbs и их размеры(количество заполненных lbs).
Return: no.
*/
static void bubbleSortLbs(GSM_INFO* pBig, uint32_t len_big, GSM_INFO* pLittle, uint32_t len_little)
{
   for (uint32_t n = 0; n < len_big; n++)
   {
      for (uint32_t i = 0; i < len_little; i++)
      {
         if (pBig->inf->station[n].rxlev < pLittle->inf->station[i].rxlev)
         {
            /* Проверка на одинаковые GSM станций чтобы не включать их в список */
            if (pBig->inf->station[n].cell != pLittle->inf->station[i].cell ||
                pBig->inf->station[n].lac != pLittle->inf->station[i].lac ||
                pBig->inf->station[n].mcc != pLittle->inf->station[i].mcc ||
                pBig->inf->station[n].mnc != pLittle->inf->station[i].mnc)
            {
               swap(pBig->inf->station[n].rxlev, pLittle->inf->station[i].rxlev);
               swap(pBig->inf->station[n].mcc, pLittle->inf->station[i].mcc);
               swap(pBig->inf->station[n].mnc, pLittle->inf->station[i].mnc);
               swap(pBig->inf->station[n].lac, pLittle->inf->station[i].lac);
               swap(pBig->inf->station[n].cell, pLittle->inf->station[i].cell);
            }
         }
      }
   }
}

uint8_t GetQualityGsm(void)
{
   return gsm_param.rssi;
}

/* Информация по всем видимым базовым станциям */
int getAllLbsInfo(GSM_INFO* gsm, int second)
{
   memset(&g_base_station, 0, sizeof(TS_MOND));
   mc_send(AT_CNETSCAN1, NULL, 0);

   data_smond.inf = &g_base_station;
   mc_send(AT_SMOND, NULL, 0);
   if (gsm_get_location(&data_smond, second))
   {
      return ERR_CMD;
   }
   data_smond.inf->ta = 255;
   memcpy(gsm, &data_smond, sizeof(GSM_INFO));
   return RET_OK;
}

char* findStartCeng(char* ptr, int second)
{
   char* pStartCeng = NULL;
   portTickType xLastWakeTimerDelay = xTaskGetTickCount();
   vTaskDelayUntil(&xLastWakeTimerDelay, (1000 / portTICK_RATE_MS));
   for (int n = 0; n < second; n++)
   {
      loop(GSM_RX_BUFFER_SIZE)
      {
         if (ptr[i] == '\0')
         {
            ptr[i] = '.';
         }
         else
         {
            break;
         }
      }
      pStartCeng = strstr(ptr, "+CENG: 0,");
      if (pStartCeng)
         break;
   }
   ptr[GSM_RX_BUFFER_SIZE - 1] = '\0';
   return pStartCeng;
}

int parseCeng(GSM_INFO* pOutSt, char* pIn)
{
   int mcc, mnc, rxlev, lac, cell, bsic, count;
   DP_GSM("LBS from \"CENG:\"\r\n");
   for (count = 0; count < SIZE_LBS;)
   {
      /* Находим отдельные строки */
      if (strstr(pIn, "+CENG: ") == NULL)
         break;

      if (sscanf(pIn, "+CENG: %1d,\"%4d,%4d,%4X,%4X,%2d,%2d\"", &count, &mcc, &mnc, &lac, &cell, &bsic, &rxlev) != 7)
      {
         break;
      }
      DP_GSM("N%d) ", count);
      DP_GSM("MCC:%d ", mcc);
      DP_GSM("MNC:%d ", mnc);
      DP_GSM("LAC:%04X ", lac);
      DP_GSM("CELL:%04X ", cell);
      DP_GSM("RXLEV:%d\r\n", rxlev);

      pOutSt->inf->station[count].mcc = (u16)mcc;
      pOutSt->inf->station[count].mnc = (u8)mnc;
      pOutSt->inf->station[count].lac = (u16)lac;
      pOutSt->inf->station[count].cell = (u16)cell;
      pOutSt->inf->station[count].rxlev = (u8)rxlev;
      pOutSt->count = count;

      /* find  CR or Null */
      while (*pIn++ != '\n')
      {
         if (*pIn == '\0')
            return count;
      }
   }
   return count;
}

/* Информация по активным базовым станциям */
int getActiveLbsInfo(GSM_INFO* gsm, int second)
{
   memset(&g_base_station, 0, sizeof(TS_MOND));
   data_smond.inf = &g_base_station;

   /* Расширеная информация по базовым станциям */
#if UART_GSM == 1
   char* pRxBufferGSM = (char*)(g_aucRxBufferUSART1);
#endif

#if UART_GSM == 2
   char* pRxBufferGSM = (char*)(g_aucRxBufferUSART2);
#endif

#if UART_GSM == 3
   char* pRxBufferGSM = (char*)(g_aucRxBufferUSART3);
#endif

   /* Расшириная информация о активной базовой станции к которой подключен GSM */
   uint8_t timeout = MC_COUNT;
   while (timeout--)
   {
      if (mc("at+ceng=3,1", 3, 3) == RET_OK)
         break;
   }

   timeout = MC_COUNT;
   gsm->count = 0;
   while (!(gsm->count))
   {
      mc_send(AT_CENG, NULL, 0);

      /* найдем начало сообщения */
      char* pStartCeng = findStartCeng(pRxBufferGSM, second);
      if (!(pStartCeng))
      {
         DP_GSM("[t] %s\r\n", AT_CENG);
         portTickType xLastWakeTimerDelay = xTaskGetTickCount();
         vTaskDelayUntil(&xLastWakeTimerDelay, (1000 / portTICK_RATE_MS));
      }
      else
      {
         DP_GSM("[+] %s\r\n", AT_CENG);
         gsm->count = parseCeng(&data_smond, pStartCeng);
      }
      if (!(timeout))
         break;
      timeout--;
   }
   memcpy(gsm, &data_smond, sizeof(GSM_INFO));
   return 0;
}

//Функция дозвона на пользовательский номер
int CallEventTel(const char* pTelUser)
{
   GSM_INFO data_mc;
   char cmd[SIZE_TEL + 5];
   char srtTelUser[SIZE_TEL] = { '\0' };

   if (pTelUser[0] == '8')
   {   //Если у нас первая цифра '8', то просто скопируем номер,
      strcpy(srtTelUser, pTelUser);
   }
   else
   {
      //иначе подставим '+' как международный код страны.
      size_t len = strlen(pTelUser);
      loop(len)
      {
         srtTelUser[i + 1] = pTelUser[i];
      }
      srtTelUser[0] = '+';
   }
   sprintf(cmd, "atd%s;", srtTelUser);
   mc_send(cmd, NULL, 0);

   /* ловим что трубку повесили */
   while (1)
   {
      if (gsm_parser(cmd, &data_mc, g_asRxBuf, RX_BUFFER_SIZE, 5) < 0)
      {
         break;
      }
      if (data_mc.m_type == M_NO_CARRIER || data_mc.m_type == M_NO_ANSWER || data_mc.m_type == M_BUSY)
      {
         break;   //Не получилось дозвониться, не подняли трубку или сбросили вызов.
      }
   }

   return 0;
}

int gsm_get_location(GSM_INFO* ptr, int second)
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
   u32 val = 0;
   // Find "Operator" and "\r\n"
   char* pFindCrLf = NULL;
   char* pFindOperator = NULL;
   char* pFindEcho = strstr(pRxBufferGSM, "cnetscan");
   T_VAR stVarible;
   memset(&stVarible, 0, sizeof(stVarible));

   while (1)
   {
      if (pFindEcho <= NULL)
      {   // find Echo
         pFindEcho = strstr(pRxBufferGSM, "cnetscan");
      }
      if ((!(stVarible.IndexOperator)) && (g_bDmaGsmFail))
      {
         for (int i = 0; i < GSM_RX_BUFFER_SIZE; i++)
         {
            if (pRxBufferGSM[i] == '\0')
            {
               pRxBufferGSM[i] = '.';
            }
            else
            {
               break;
            }
         }
      }
      pFindOperator = strstr(&pRxBufferGSM[stVarible.IndexOperator], "Operator:");
      if (pFindOperator)
      {
         count++;
         stVarible.IndexOperator += *pFindOperator + strlen("Operator");
         pFindCrLf = strstr(&pRxBufferGSM[stVarible.IndexRN], "\r\n");
         if (!(pFindCrLf))
         {
            break;
         }
         else
         {
            stVarible.IndexRN += *pFindCrLf + strlen("\r\n");
         }
      }
      else
      {
         portTickType xLastWakeTimerDelay = xTaskGetTickCount();
         vTaskDelayUntil(&xLastWakeTimerDelay, (1000 / portTICK_RATE_MS));
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

   if (count > SIZE_LBS)
   {
      count = SIZE_LBS;
   }

   GSM_DPD((char*)pRxBufferGSM, DBG_TX_BUFFER_SIZE);
   DP_GSM("\r\n");

   //Есть LBS
   memset(&stVarible, 0, sizeof(stVarible));
   char* pFindParam = NULL;
   for (uint8_t Index = 0; Index < count; Index++)
   {
      char temp[6];
      // find MCC
      pFindParam = strstr(&pRxBufferGSM[stVarible.IndexMcc], "MCC:");
      if (pFindParam <= NULL)
      {
         count = Index;
         break;
      }
      pFindParam += sizeof("MCC:") - 1;
      stVarible.IndexMcc += *pFindParam;

      loop(sizeof(temp))
      {
         if (*pFindParam == ',')
         {
            temp[i] = 0;
            break;
         }
         temp[i] = *pFindParam;
         pFindParam++;
      }
      size_t len = strlen(temp);
      loop(len)
      {
         val = calculate(0, 10, temp[i], val);
      }
      ptr->inf->station[Index].mcc = (u16)val;   //
      val = 0;

      // find MNC
      pFindParam = strstr(&pRxBufferGSM[stVarible.IndexMnc], "MNC:");
      if (pFindParam <= NULL)
      {
         count = Index;
         break;
      }
      pFindParam += sizeof("MNC:") - 1;
      stVarible.IndexMnc += *pFindParam;
      loop(sizeof(temp))
      {
         if (*pFindParam == ',')
         {
            temp[i] = 0;
            break;
         }
         temp[i] = *pFindParam;
         pFindParam++;
      }

      len = strlen(temp);
      loop(len)
      {
         val = calculate(0, 10, temp[i], val);
      }
      ptr->inf->station[Index].mnc = (u16)val;   // atof(temp);
      val = 0;

      // find Rxlev
      pFindParam = strstr(&pRxBufferGSM[stVarible.IndexRxlen], "Rxlev:");
      if (pFindParam <= NULL)
      {
         count = Index;
         break;
      }
      pFindParam += sizeof("Rxlev:") - 1;
      stVarible.IndexRxlen += *pFindParam;
      loop(sizeof(temp))
      {
         if (*pFindParam == ',')
         {
            temp[i] = 0;
            break;
         }
         temp[i] = *pFindParam;
         pFindParam++;
      }

      len = strlen(temp);
      loop(len)
      {
         val = calculate(0, 10, temp[i], val);
      }
      ptr->inf->station[Index].rxlev = (u8)val;
      val = 0;

      // find Cellid
      pFindParam = strstr(&pRxBufferGSM[stVarible.IndexCellid], "Cellid:");
      if (pFindParam <= NULL)
      {
         count = Index;
         break;
      }
      pFindParam += sizeof("Cellid:") - 1;
      stVarible.IndexCellid += *pFindParam;
      loop(sizeof(temp))
      {
         if (*pFindParam == ',')
         {
            temp[i] = 0;
            break;
         }
         temp[i] = *pFindParam;
         pFindParam++;
      }

      len = strlen(temp);
      loop(len)
      {
         val = calculate(0, 16, temp[i], val);
      }
      ptr->inf->station[Index].cell = (u16)val;
      val = 0;

      // find Lac
      pFindParam = strstr(&pRxBufferGSM[stVarible.IndexLac], "Lac:");
      if (pFindParam <= NULL)
      {
         count = Index;
         break;
      }
      pFindParam += sizeof("Lac:") - 1;
      stVarible.IndexLac += *pFindParam;
      loop(sizeof(temp))
      {
         if (*pFindParam == ',')
         {
            temp[i] = 0;
            break;
         }
         temp[i] = *pFindParam;
         pFindParam++;
      }

      len = strlen(temp);
      loop(len)
      {
         val = calculate(0, 16, temp[i], val);
      }
      ptr->inf->station[Index].lac = (u16)val;
      val = 0;

      // Find Operator
      pFindParam = strstr(&pRxBufferGSM[stVarible.IndexMcc], "Operator:");
      if (pFindParam <= NULL)
      {
         count = Index;
         break;
      }
      if (pFindParam)
      {
         pFindParam += sizeof("MCC:") - 1;
         stVarible.IndexMcc += *pFindParam;
         stVarible.IndexMnc += *pFindParam;
         stVarible.IndexLac += *pFindParam;
         stVarible.IndexCellid += *pFindParam;
         stVarible.IndexRxlen += *pFindParam;
      }

      /*DP_GSM("N%d) MCC:%03d MNC:%02d LAC:%04X SELL:%04X RXLEV:%02d\r\n", Index+1,
            ptr->inf->station[Index].mcc,
            ptr->inf->station[Index].mnc,
            ptr->inf->station[Index].lac,
            ptr->inf->station[Index].cell,
            ptr->inf->station[Index].rxlev);*/
   }

   ptr->count = count;   //Количество базовых станций
   ReStartDmaGsmUsart();   //Перезапускаем DMA.
   g_bDmaGsmFail = FALSE;
   return 0;
}

int lbsInfo2buffer(GSM_INFO* pLbsCeng, uint32_t time, char* pOut, int iOffset, _Bool bAddLbs)
{
   int n = 0;
   uint8_t bitFree = 8;

   if (pLbsCeng->count > 0)
   {
      // rt_buf.lbs_time = time; //
      u8 ta = pLbsCeng->inf->ta;   // TA
      n += bit_packing(pOut + n + iOffset, time, &bitFree, 32);   // время

      pOut[iOffset + n++] = 5 * pLbsCeng->count + 4;   // длина блока
      n += bit_packing(pOut + n + iOffset, ta, &bitFree, 7);   // ТА
      n += bit_packing(pOut + n + iOffset, pLbsCeng->inf->station[0].mcc, &bitFree, 10);
      n += bit_packing(pOut + n + iOffset, pLbsCeng->inf->station[0].mnc, &bitFree, 10);
      n += bit_packing(pOut + n + iOffset, 0, &bitFree, 5);   // резерв

      DP_GSM("MCC:%03d MNC:%02d\r\n", pLbsCeng->inf->station[0].mcc, pLbsCeng->inf->station[0].mnc);

      loop(pLbsCeng->count)
      {
         n += bit_packing(pOut + n + iOffset, pLbsCeng->inf->station[i].lac, &bitFree, 16);
         n += bit_packing(pOut + n + iOffset, pLbsCeng->inf->station[i].cell, &bitFree, 16);
         n += bit_packing(pOut + n + iOffset, pLbsCeng->inf->station[i].rxlev, &bitFree, 8);
         DP_GSM("LAC:%04X SELL:%04X RXLEV:%02d\r\n",
                pLbsCeng->inf->station[i].lac,
                pLbsCeng->inf->station[i].cell,
                pLbsCeng->inf->station[i].rxlev);
      }
   }

   /* добавляем доп пакет данных lbs полученных из сnetscan */
   while (bAddLbs)
   {
      if (!(additionOtherLbsForIon(pLbsCeng)))
      {
         break;   // нечего больше добавлять
      }

      pOut[iOffset + n++] = 5 * pLbsCeng->count + 4;   // длина блока
      n += bit_packing(pOut + n + iOffset, 0, &bitFree, 7);   // ТА
      n += bit_packing(pOut + n + iOffset, pLbsCeng->inf->station[0].mcc, &bitFree, 10);
      n += bit_packing(pOut + n + iOffset, pLbsCeng->inf->station[0].mnc, &bitFree, 10);
      n += bit_packing(pOut + n + iOffset, 0, &bitFree, 5);   // резерв

      DP_GSM("MCC:%03d MNC:%02d\r\n", pLbsCeng->inf->station[0].mcc, pLbsCeng->inf->station[0].mnc);

      loop(pLbsCeng->count)
      {
         n += bit_packing(pOut + n + iOffset, pLbsCeng->inf->station[i].lac, &bitFree, 16);
         n += bit_packing(pOut + n + iOffset, pLbsCeng->inf->station[i].cell, &bitFree, 16);
         n += bit_packing(pOut + n + iOffset, pLbsCeng->inf->station[i].rxlev, &bitFree, 8);
         DP_GSM("LAC:%04X SELL:%04X RXLEV:%02d\r\n",
                pLbsCeng->inf->station[i].lac,
                pLbsCeng->inf->station[i].cell,
                pLbsCeng->inf->station[i].rxlev);
      }
   }

   return n;
}

RET_INFO GetCellularNetwork(void)
{
   GSM_INFO out_check;
   uint32_t uiTimeFind;
   portTickType xLastWakeTimerDelay;
   uint8_t ucStepFindGsm = 0;

   uiTimeFind = xTaskGetTickCount() + (GetGsmFindTimeout() * configTICK_RATE_HZ);
   while (xTaskGetTickCount() < uiTimeFind)
   {
      /* ищем gsm сеть */
      if ((ucStepFindGsm == 0) && (check_csq(&out_check) != ERR_TIMEOUT))
      {
         ucStepFindGsm = 1;
         uint8_t rssi = GetQualityGsm();
#define LOW_GSM_SIGNAL 10
         if (rssi < LOW_GSM_SIGNAL)
         {   // Low signal strength
            uint16_t usTemp1, usTemp2, usTemp3, usTemp4;
            GetGsmPwrErr(&usTemp1, &usTemp2, &usTemp3, &usTemp4);
            usTemp3++;   //низкий уровень сигнала
            SetGsmPwrErr(usTemp1, usTemp2, usTemp3, usTemp4);
         }
      }
      /* получаем регистрацию в сети */
      if ((ucStepFindGsm == 1) && (check_creg(&out_check) != ERR_GSMNET))
      {
         return (RET_INFO)(out_check.msg[1].var + 0x30);
      }
      xLastWakeTimerDelay = xTaskGetTickCount();
      vTaskDelayUntil(&xLastWakeTimerDelay, (SLEEP_MS_1000 / portTICK_RATE_MS));
   }

   uint16_t usTemp1, usTemp2, usTemp3, usTemp4;
   GetGsmPwrErr(&usTemp1, &usTemp2, &usTemp3, &usTemp4);
   usTemp4++;   //не зарегался в сети
   SetGsmPwrErr(usTemp1, usTemp2, usTemp3, usTemp4);
   usTemp4 = GetGsmFind() + 1;
   SetGsmFind(usTemp4);
   return ERR_GSMNET;
}

static void csq_save(const GSM_INFO* out)
{
   if (out->m_type == M_CSQ)
   {
      switch (out->count)
      {
      case 2:
         gsm_param.ber = out->msg[1].var;
      case 1:
         gsm_param.rssi = out->msg[0].var;
      }
   }
}

int check_csq(GSM_INFO* out)
{
   mc_get("at+csq", M_CSQ, out, 1, 1);
   if (out->m_type == M_CSQ)
   {
      DP_GSM("___CSQ: %d\r\n", out->msg[0].var);
   }
   if (out->msg[0].var != 99)
   {
      SetCSQ(out->msg[0].var);
   }
   else
   {
      SetCSQ(0);
   }

   if ((out->msg[0].var != 99) && (out->msg[0].var != 0))
   {
      csq_save(out);
      return RET_OK;
   }

   csq_save(out);
   return ERR_TIMEOUT;
}

// дождаться авторизации в сети получить статус авторизации
// '1' - домашняя сеть, '5' - роуминг.
int check_creg(GSM_INFO* out)
{
   mc_get("at+creg?", M_CREG, out, 1, 1);
   if (out->m_type == M_CREG)
   {
      DP_GSM("__CREG: %d,%d ", out->msg[0].var, out->msg[1].var);
      switch (out->count)
      {
      case 4:
         gsm_param.netCellId = out->msg[3].var;
      case 3:
         gsm_param.netLac = out->msg[2].var;
      case 2:
         gsm_param.regStatus = out->msg[1].var;
      case 1:
         gsm_param.urcMode = out->msg[0].var;
      }
      switch (out->msg[1].var)
      {
      case 0:
         DP_GSM("(error)");
         break;
      case 1:
         DP_GSM("(home)");
         break;
      case 2:
         DP_GSM("(find)");
         break;
      case 3:
         DP_GSM("(ban)");
         break;
      case 4:
         DP_GSM("(no info)");
         break;
      case 5:
         DP_GSM("(rouming)");
         break;
      }
      DP_GSM("\r\n");
      if (out->msg[1].var == 1 || out->msg[1].var == 5)
      {
         // зарегистрирован
         gsm_param.regStatus = out->msg[1].var;
         return out->msg[1].var;
      }
      if (out->msg[1].var != 2)
      {
         // в регистрации отказано
         // break;
      }
   }
   DP_GSM("D_GSM FIND ERR\r\n");
   return ERR_GSMNET;
}

void SimPwr(VALUE eVal)
{
   static _Bool flag_pwr_sim = TRUE;
   if (eVal == ON && flag_pwr_sim)
   {
      SetGsmFunctional(MIN_FUNCTIONALITI);
      DP_GSM("D_SIM ON\r\n");
      SIM_ON;   //Подаем питание на SIM Card 1
      portTickType xLastWakeTimerDelay = xTaskGetTickCount();
      vTaskDelayUntil(&xLastWakeTimerDelay, (1000 / portTICK_RATE_MS));   //подождем немного, чтобы сеть устаканилась
      SetGsmFunctional(FULL_FUNCTIONALITI);
      flag_pwr_sim = FALSE;
   }

   if ((eVal == OFF) && (!(flag_pwr_sim)))
   {
      flag_pwr_sim = TRUE;
      DP_GSM("D_SIM OFF\r\n");
      SIM_OFF;   //Снимаем питание на SIM Card 1
   }
}

/*
Генератор пин кода на заводскую СИМ
input1: SCID сим карты по которому будет генерится пин код;
input2: Сгенерированный пин код, изменяемое значение.
return: Указатель на сгенерированный пинкод
*/
char* generPinSim(const char* pSour, char* pGenPin)
{
   uint32_t uiSum = 0;
   char strPinFirstSim[10] = { '\0' };
   size_t len = strlen(pSour);
   loop(len)
   {
      uiSum += pSour[i];
   }
   sprintf(strPinFirstSim, "%d", uiSum);
   memset(pGenPin, 0, SIZE_PIN_CODE);
   for (uint8_t i = 0; i < 4; i++)
   {
      if (strPinFirstSim[i])
      {
         pGenPin[i] = strPinFirstSim[i];
      }
      else
      {
         pGenPin[i] = '0';
      }
   }
   return pGenPin;
}

int LockSimCard(void)
{
   GSM_INFO data_mc;
   memset(&data_mc, 0, sizeof(data_mc));
   SIM_CARD eNumSim = GetNumSim();
   if (eNumSim != FIRST_SIMCARD_OK)
      return -1;

   uint8_t cnt = 0;
   /*  Спросим количество попыток ввода pin */
   while (data_mc.m_type != M_SPIC)
   {
      mc_get(AT_SPIC, M_SPIC, &data_mc, 3, 4);
      if (cnt > MC_COUNT)
         return -2;
   }
   if (data_mc.msg[0].var < 3)
      return -2;

   /* Включаем запрос пин кода сим карты */
   sprintf(g_asCmdBuf, "at+clck=\"SC\",1,\"%s\"", DEFAULT_PINCODE_SIM);
   if (mc(g_asCmdBuf, 10, 1) != RET_OK)
   {
      return -3;
   }

   char strPin[SIZE_PIN_CODE];
   char strFIRST_SCID[SIZE_SCID];
   GetScidCurentFirstSim(strFIRST_SCID);
   generPinSim(strFIRST_SCID, strPin);   // генерируем пин код.
   /* Изменяем пин код */
   sprintf(g_asCmdBuf, "at+cpwd=\"SC\",\"%s\",\"%s\"", DEFAULT_PINCODE_SIM, strPin);
   if (mc(g_asCmdBuf, 10, 1) != RET_OK)
   {
      return -4;
   }
   return 0;
}

SIM_CARD GetStatusAllSim(void)
{
#if (TWO_SIMCARD)
   GSM_INFO data_mc;
#   define MAX__NUM_OBTAIN_STAT_SIM 5
   for (uint8_t i = 0; i < MAX__NUM_OBTAIN_STAT_SIM; i++)
   {
      memset(g_asRxBuf, 0, sizeof(g_asRxBuf));
      memset(&data_mc, 0, sizeof(data_mc));
      mc_get("at+cdsds?", M_STRING, &data_mc, 5, 1);
      // Вернет что то типа +CDSDS: SIM1,1,0 \r \n */
      char* pFindSIM = strstr(data_mc.msg[0].str, "SIM");
      if (pFindSIM)
      {
         if (pFindSIM[5] == '1' && pFindSIM[7] == '1')
         {
            return FIRST_AND_SECOND_SIMCARD_OK;
         }
         else if (pFindSIM[5] == '0' && pFindSIM[7] == '0')
         {
            return ERROR_ALL_SIMCARD;
         }
      }
      portTickType xLastWakeTimerDelay = xTaskGetTickCount();
      vTaskDelayUntil(&xLastWakeTimerDelay, (1000 / portTICK_RATE_MS));
   }
#endif
   return ERROR_CMD;
}

/*
Возвращает номер СИМ катры с какой работает GSM модем
   ERROR_CMD =                          -4,
   ERROR_ALL_SIMCARD =                  -3,
   SECOND_SIMCARD_ERROR =               -2,
   FIRST_SIMCARD_ERROR =                -1,
   GET_NUM_SIM =                        0,
   FIRST_SIMCARD_OK =                   1,
   SECOND_SIMCARD_OK =                  2,
   FIRST_AND_SECOND_SIMCARD_OK =        3,
*/
SIM_CARD GetNumSim(void)
{
#if (TWO_SIMCARD)
   GSM_INFO data_mc;

   loop(MC_COUNT)
   {
      memset(&data_mc, 0, sizeof(data_mc));
      mc_get("at+cdsds?", M_STRING, &data_mc, 5, 1);
      /* Вернет что то типа +CDSDS: SIM1,1,0 \r \n */
      char* pFindSIM = strstr(data_mc.msg[0].str, "SIM1");
      if (pFindSIM)
      {
         if (pFindSIM[5] == '1')
         {   // Проверяем статус выбранной СИМ карты(подключена/удалена)
            DP_GSM("D_FIRST SIMCARD\r\n");
            return FIRST_SIMCARD_OK;
         }
         else
         {
            DP_GSM("D_ERR FIRST SIMCARD REMOVED\r\n");
            return FIRST_SIMCARD_ERROR;
         }
      }
      pFindSIM = strstr(data_mc.msg[0].str, "SIM2");
      if (pFindSIM)
      {
         if (pFindSIM[7] == '1')
         {   // Проверяем статус выбранной СИМ карты(подключена/удалена)
            DP_GSM("D_SECOND SIMCARD\r\n");
            return SECOND_SIMCARD_OK;
         }
         else
         {
            DP_GSM("D_ERR SECOND SIMCARD REMOVED\r\n");
            return SECOND_SIMCARD_ERROR;
         }
      }
      osDelay(100);
   }

   return ERROR_CMD;
#endif   // TWO_SIMCARD

   return FIRST_SIMCARD_OK;
}

/*
Устанавливаем СИМ карту с которой будет работать GSM модем
при передачи в функцию параметра GET_NUM_SIM функция не переключает СИМ карты,
а возвращает номер на какой она работает
*/
SIM_CARD SelectNumSim(SIM_CARD eNumSim)
{
#if (TWO_SIMCARD)
   static SIM_CARD num_sim = GET_NUM_SIM;
   if (eNumSim == FIRST_SIMCARD_OK)
   {   //Выбор первой SIM карты
      DP_GSM("D_SELECT FIRST SIMCARD\r\n");
      mc("at+cdsds=1", 5, MC_COUNT);
      num_sim = CURRENT_FIRST_SIMCARD;
   }
   else if (eNumSim == SECOND_SIMCARD_OK)
   {
      DP_GSM("D_SELECT SECOND SIMCARD\r\n");
      mc("at+cdsds=2", 5, MC_COUNT);   //Выбор второй SIM карты
      num_sim = CURRENT_SECOND_SIMCARD;
   }
   return num_sim;
#else
   return 1;
#endif   // TWO_SIMCARD
}

M_INFO getSimStatus(char* ptr)
{
   SimPwr(ON);

   char strIMEI[SIZE_IMEI] = { '\0' };
   if (SelectNumSim(GET_NUM_SIM) == CURRENT_FIRST_SIMCARD)
   {   //
      SetIMEI(readGsmIMEI(strIMEI));   // IMEI GSM-модуля
      SetStrIMEI(strIMEI);
   }
   ledStatus(FIND_SIM);   //Поиск SIM-карты
   M_INFO eSimStatus = SimCardInit();

   ptr[0] = '\0';
   GetScidFirstSim(ptr);
   char strCURENT_SCID[SIZE_SCID] = { '\0' };
   GetScidCurentFirstSim(strCURENT_SCID);
   return eSimStatus;
}

_Bool switchingEventSim(M_INFO eSimStatus)
{
   /* Если есть проблемы со второй симкой, но у нас есть уведомления, то сбросим уведомление */
   if (alarmTrue() && GetNumSim() == SECOND_SIMCARD_ERROR && eSimStatus != M_SIM_READY)
   {
      setEventTrue();
      return TRUE;
   }
   return FALSE;
}

_Bool UseSecondSim = FALSE;   // Флаг переключения на использования второй СИМ карты.
void setUseSecondSim(void)
{
#if (TWO_SIMCARD)
   if (getFlagSimSecondInstld()   // если установлен флаг что резервная сим карта была установлена
       /*&& GetCountReConnect()*/   // проверим переоткладывание(должно быть не нулевое)
       && GetTypeConnectFm911() != TYPE_REG_USER   // если не режим регистрации пользователя
       && GetTypeConnectFm911() != TYPE_REG_TO_BASE)
   {   // если не режим регистрации в БД
      g_stRam.stDevice.CountSimFirstError = MAX_COUNT_SIM_FIRST_ERROR + 1;
      DP_GSM("D_Switching to the second SIM card!\r\n");
      UseSecondSim = TRUE;
   }
#endif
}

/*
 Получаем флаг перехода на резервную SIM карту.
  TRUE - выполнен переход на резервную SIM карту.
  FALSE - функционирование по прежнему на основоной SIM.
*/
_Bool getUseSecondSim(void)
{
#if (TWO_SIMCARD)
   DP_GSM("D_SecondSim: ");
   if (UseSecondSim && getFlagSimSecondInstld()
       /*&&  GetCountReConnect()*/)
   {
      DP_GSM("TRUE\r\n");
      return TRUE;
   }
   else
   {
      DP_GSM("FALSE\r\n");
      return FALSE;
   }
#else
   return FALSE;
#endif
}

/*
 Проверка последнего сеанса на резервной СИМ карте.
  TRUE - время последнего сеанса истекло, необходим выход на резервной СИМ карте.
  FALSE - время сеанса не истекло и выход на резервной СИМ карте не требуется.
*/
_Bool expireTimeContactSecSim(void)
{
   /* Если режимы резистрации или не установлен флаг использования резервной SIM (flagSimSecondInstld),
  то не фиксим и не проверяем время выхода на резервной SIM */
   if (GetTypeConnectFm911() == TYPE_REG_TO_BASE || GetTypeConnectFm911() == TYPE_REG_USER ||
       getFlagSimSecondInstld() == FALSE)
   {
      return FALSE;
   }

   /* проверка последнего сеанса на резервной СИМ карте */
   if (time() > MAX_TIME_CONTACT_SIM_SECOND + getTimeContactSecondSim())
   {
      DPS("D_Expire Time == TRUE\r\n");
      return TRUE;
   }

   return FALSE;
}