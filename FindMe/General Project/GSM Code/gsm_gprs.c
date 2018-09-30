

#include "includes.h"
#include "gsm_gprs.h"

static _Bool configUserApn(_Bool);

// инициализация gprs профилей для соединения с сервером
RET_INFO profile_gprs_connect(_Bool bTypeProf)
{
   GSM_INFO data_profile_gprs_connect;
   data_profile_gprs_connect.msg[0].str = STR_NULL;
   // подключиться к сети GPRS
   if (GsmModemConnectGprsService())
   {
      DPS("D_GSM_ERR: GPRS not activate!\r\n");
      return ERR_GPRS_ACTIVATE;
   }

   //получим mobile country code
   uint8_t count = MC_COUNT;
   while (!(strlen(data_profile_gprs_connect.msg[0].str)))
   {
      mc_get("at+cimi", M_STRING, &data_profile_gprs_connect, 1, 4);
      osDelay(100);
      count--;
      if (!(count))
         break;
   }

   memcpy(g_stRam.stSim.acMobCountCode, data_profile_gprs_connect.msg[0].str, 5);

   if (configUserApn(bTypeProf))
   {
      return RET_GPRS_OK;
   }

   if (data_profile_gprs_connect.m_type != M_STRING)
   {
      return ERR_APN;
   }

   /* Russian ---------------------------- */
   // MTS RUS
   if (memcmp(data_profile_gprs_connect.msg[0].str, "25001", 5) == 0)
   {
      DP_GSM("D__CIMI: %s\r\n", data_profile_gprs_connect.msg[0].str);
      SetupInternetConnectionProfile("mts", "mts", "internet.mts.ru", bTypeProf);   // PARAM: user, passwd, apn.
   }
   // MegaFon RUS
   else if (memcmp(data_profile_gprs_connect.msg[0].str, "25002", 5) == 0)
   {
      DP_GSM("D__CIMI: %s\r\n", data_profile_gprs_connect.msg[0].str);
      SetupInternetConnectionProfile("gdata", "gdata", "internet", bTypeProf);   // PARAM: user, passwd, apn.
   }
   // Beeline RUS
   else if (memcmp(data_profile_gprs_connect.msg[0].str, "25099", 5) == 0)
   {
      DP_GSM("D__CIMI: %s\r\n", data_profile_gprs_connect.msg[0].str);
      SetupInternetConnectionProfile(
         "beeline", "beeline", "internet.beeline.ru", bTypeProf);   // PARAM: user, passwd, apn.
   }

   // Tele2 RUS
   else if (memcmp(data_profile_gprs_connect.msg[0].str, "25020", 5) == 0)
   {
      DP_GSM("D__CIMI: %s\r\n", data_profile_gprs_connect.msg[0].str);
      SetupInternetConnectionProfile("", "", "internet.tele2.ru", bTypeProf);   // PARAM: user, passwd, apn.
   }
   /* ------------------------------------ */

   // Азейбарджан Azercell
   else if (memcmp(data_profile_gprs_connect.msg[0].str, "40001", 5) == 0)
   {
      DP_GSM("D__CIMI: %s\r\n", data_profile_gprs_connect.msg[0].str);
      SetupInternetConnectionProfile("", "", "internet", bTypeProf);   // PARAM: user, passwd, apn.
   }
   // Mexico Telcel
   else if (memcmp(data_profile_gprs_connect.msg[0].str, "334020", 5) == 0)
   {
      DP_GSM("D__CIMI: %s\r\n", data_profile_gprs_connect.msg[0].str);
      SetupInternetConnectionProfile(
         "webgprs", "webgprs2002", "internet.itelcel.com", bTypeProf);   // PARAM: user, passwd, apn.
   }
   //кзахстан
   else if (memcmp(data_profile_gprs_connect.msg[0].str, "40102", 5) == 0)
   {
      DP_GSM("D__CIMI: %s\r\n", data_profile_gprs_connect.msg[0].str);
      SetupInternetConnectionProfile("", "", "soho", bTypeProf);   // PARAM: user, passwd, apn.
   }
   else if (memcmp(data_profile_gprs_connect.msg[0].str, "40101", 5) == 0)
   {
      DP_GSM("D__CIMI: %s\r\n", data_profile_gprs_connect.msg[0].str);
      SetupInternetConnectionProfile(
         "\"@internet.beeline\"", "beeline", "internet.beeline.kz", bTypeProf);   // PARAM: user, passwd, apn.
   }
   //киргизия NurTelecom LLC
   else if (memcmp(data_profile_gprs_connect.msg[0].str, "43709", 5) == 0 ||
            memcmp(data_profile_gprs_connect.msg[0].str, "43705", 5) == 0)
   {
      DP_GSM("D__CIMI: %s\r\n", data_profile_gprs_connect.msg[0].str);
      SetupInternetConnectionProfile("", "", "internet", bTypeProf);   // PARAM: user, passwd, apn.
   }
   else if (memcmp(data_profile_gprs_connect.msg[0].str, "43701", 5) == 0)
   {
      DP_GSM("D__CIMI: %s\r\n", data_profile_gprs_connect.msg[0].str);
      SetupInternetConnectionProfile("", "", "internet.beeline.kg", bTypeProf);   // PARAM: user, passwd, apn.
   }
   else if (memcmp(data_profile_gprs_connect.msg[0].str, "43705", 5) == 0)
   {
      DP_GSM("D__CIMI: %s\r\n", data_profile_gprs_connect.msg[0].str);
      SetupInternetConnectionProfile("", "", "internet.kg", bTypeProf);   // PARAM: user, passwd, apn.
   }
   //белорусия
   else if (memcmp(data_profile_gprs_connect.msg[0].str, "25702", 5) == 0)
   {
      DP_GSM("D__CIMI: %s\r\n", data_profile_gprs_connect.msg[0].str);
      SetupInternetConnectionProfile("mts", "mts", "mts", bTypeProf);   // PARAM: user, passwd, apn.
   }

   /* China ------------------------------ */
   // China Mobile
   else if (memcmp(data_profile_gprs_connect.msg[0].str, "46000", 5) == 0)
   {
      DP_GSM("D__CIMI: %s\r\n", data_profile_gprs_connect.msg[0].str);
      SetupInternetConnectionProfile("", "", "cmnet", bTypeProf);   // PARAM: user, passwd, apn.
   }
   // China Unicom
   else if (memcmp(data_profile_gprs_connect.msg[0].str, "46001", 5) == 0)
   {
      DP_GSM("D__CIMI: %s\r\n", data_profile_gprs_connect.msg[0].str);
      SetupInternetConnectionProfile("", "", "uinet", bTypeProf);   // PARAM: user, passwd, apn.
   }
   // China Unicom
   else if (memcmp(data_profile_gprs_connect.msg[0].str, "46006", 5) == 0)
   {
      DP_GSM("D__CIMI: %s\r\n", data_profile_gprs_connect.msg[0].str);
      SetupInternetConnectionProfile("", "", "uinet", bTypeProf);   // PARAM: user, passwd, apn.
   }
   /* ------------------------------------ */

   /* Estonia ---------------------------- */
   else if (memcmp(data_profile_gprs_connect.msg[0].str, "24801", 5) == 0)
   {
      DP_GSM("D__CIMI: %s\r\n", data_profile_gprs_connect.msg[0].str);
      SetupInternetConnectionProfile("", "", "internet.emt.ee", bTypeProf);   // PARAM: user, passwd, apn.
   }
   /* ------------------------------------ */

   /* Latvia ----------------------------- */
   else if (memcmp(data_profile_gprs_connect.msg[0].str, "24702", 5) == 0)
   {
      DP_GSM("D__CIMI: %s\r\n", data_profile_gprs_connect.msg[0].str);
      SetupInternetConnectionProfile("wap", "wap", "internet.tele2.lv", bTypeProf);   // PARAM: user, passwd, apn.
   }
   else if (memcmp(data_profile_gprs_connect.msg[0].str, "24701", 5) == 0)
   {
      DP_GSM("D__CIMI: %s\r\n", data_profile_gprs_connect.msg[0].str);
      SetupInternetConnectionProfile("", "", "okarte.lmt.lv", bTypeProf);   // PARAM: user, passwd, apn.
   }
   /* ------------------------------------ */

   /* Ukraine ---------------------------- */
   // украина МТС
   else if (memcmp(data_profile_gprs_connect.msg[0].str, "25501", 5) == 0)
   {
      DP_GSM("D__CIMI: %s\r\n", data_profile_gprs_connect.msg[0].str);
      SetupInternetConnectionProfile("", "", "telematika", bTypeProf);   // PARAM: user, passwd, apn.
   }
   // украина Киевстар
   else if (memcmp(data_profile_gprs_connect.msg[0].str, "25503", 5) == 0)
   {
      DP_GSM("D__CIMI: %s\r\n", data_profile_gprs_connect.msg[0].str);
      SetupInternetConnectionProfile("", "", "www.kyivstar.net", bTypeProf);   // PARAM: user, passwd, apn.
   }
   // украина Life
   else if (memcmp(data_profile_gprs_connect.msg[0].str, "25506", 5) == 0)
   {
      DP_GSM("D__CIMI: %s\r\n", data_profile_gprs_connect.msg[0].str);
      SetupInternetConnectionProfile("", "", "internet", bTypeProf);   // PARAM: user, passwd, apn.
   }
   /* ------------------------------------ */

   /* Brazil ----------------------------- */
   // Бразилия Vivo S.A.1
   else if (memcmp(data_profile_gprs_connect.msg[0].str, "72406", 5) == 0)
   {
      DP_GSM("D__CIMI: %s\r\n", data_profile_gprs_connect.msg[0].str);
      SetupInternetConnectionProfile("vivo", "vivo", "zap.vivo.com.br", bTypeProf);   // PARAM: user, passwd, apn.
   }
   // Бразилия Vivo S.A.2
   else if (memcmp(data_profile_gprs_connect.msg[0].str, "72410", 5) == 0)
   {
      DP_GSM("D__CIMI: %s\r\n", data_profile_gprs_connect.msg[0].str);
      SetupInternetConnectionProfile("vivo", "vivo", "zap.vivo.com.br", bTypeProf);   // PARAM: user, passwd, apn.
   }
   // Бразилия Vivo S.A.3
   else if (memcmp(data_profile_gprs_connect.msg[0].str, "72411", 5) == 0)
   {
      DP_GSM("D__CIMI: %s\r\n", data_profile_gprs_connect.msg[0].str);
      SetupInternetConnectionProfile("vivo", "vivo", "zap.vivo.com.br", bTypeProf);   // PARAM: user, passwd, apn.
   }
   // Бразилия Vivo S.A.4
   else if (memcmp(data_profile_gprs_connect.msg[0].str, "72423", 5) == 0)
   {
      DP_GSM("D__CIMI: %s\r\n", data_profile_gprs_connect.msg[0].str);
      SetupInternetConnectionProfile("vivo", "vivo", "zap.vivo.com.br", bTypeProf);   // PARAM: user, passwd, apn.
   }
   // Бразилия TIM1
   else if (memcmp(data_profile_gprs_connect.msg[0].str, "72402", 5) == 0)
   {
      DP_GSM("D__CIMI: %s\r\n", data_profile_gprs_connect.msg[0].str);
      SetupInternetConnectionProfile("tim", "tim", "tim.br", bTypeProf);   // PARAM: user, passwd, apn.
   }
   // Бразилия TIM2
   else if (memcmp(data_profile_gprs_connect.msg[0].str, "72403", 5) == 0)
   {
      DP_GSM("D__CIMI: %s\r\n", data_profile_gprs_connect.msg[0].str);
      SetupInternetConnectionProfile("tim", "tim", "tim.br", bTypeProf);   // PARAM: user, passwd, apn.
   }
   // Бразилия TIM3
   else if (memcmp(data_profile_gprs_connect.msg[0].str, "72404", 5) == 0)
   {
      DP_GSM("D__CIMI: %s\r\n", data_profile_gprs_connect.msg[0].str);
      SetupInternetConnectionProfile("tim", "tim", "tim.br", bTypeProf);   // PARAM: user, passwd, apn.
   }
   // Бразилия Claro BR1
   else if (memcmp(data_profile_gprs_connect.msg[0].str, "72405", 5) == 0)
   {
      DP_GSM("D__CIMI: %s\r\n", data_profile_gprs_connect.msg[0].str);
      SetupInternetConnectionProfile("claro", "claro", "claro.com.br", bTypeProf);   // PARAM: user, passwd, apn.
   }
   /* ------------------------------------ */

   /* Saudi Arabia ----------------------- */
   // Саудовская Аравия STC
   else if (memcmp(data_profile_gprs_connect.msg[0].str, "42001", 5) == 0)
   {
      DP_GSM("D__CIMI: %s\r\n", data_profile_gprs_connect.msg[0].str);
      SetupInternetConnectionProfile("", "", "jawalnet.com.sa", bTypeProf);   // PARAM: user, passwd, apn.
   }
   // Саудовская Аравия MOBILY
   else if (memcmp(data_profile_gprs_connect.msg[0].str, "42003", 5) == 0)
   {
      DP_GSM("D__CIMI: %s\r\n", data_profile_gprs_connect.msg[0].str);
      SetupInternetConnectionProfile("", "", "web2", bTypeProf);   // PARAM: user, passwd, apn.
   }
   /* ------------------------------------ */
   else
   {
      DPS("D_GSM_ERR: SIM not find apn!");
      return ERR_APN;
   }

   return RET_GPRS_OK;
}

int profile_socket(char* pNameServ, uint8_t nProf)
{
   if ((g_stRam.stConnect.bGprsProfActivate == FALSE) && (ActivationConnectToGprs(nProf)))
   {
      return 1;
   }
   g_stRam.stConnect.bGprsProfActivate = TRUE;
   g_stRam.stSim.bGprsTrue = ON;
   return 0;
}

RET_INFO ProfileSocketInit(char* pNameSocket, uint8_t nProf)
{
   // Подставляем имя сервера.
   if (profile_socket(pNameSocket, nProf))
   {
      return ERR_GPRS_ACTIVATE;
   }

   return RET_OK;
}

// 1 Профиль, 2 - колво попыток, 3 - задержка в сек.
int mc_siso(u8 nProf, uint8_t count, uint8_t second)
{
   RET_INFO ret_gsm = RET_OK;
   memset(g_asCmdBuf, 0, sizeof(g_asCmdBuf));
   sprintf(g_asCmdBuf, "at^siso=%d", nProf);

   for (; count > 0; count--)
   {
      mc_send(g_asCmdBuf, NULL, 0);
      ret_gsm = isOK(g_asCmdBuf, second);
      if (ret_gsm == RET_OK)
         return RET_OK;
      if (ret_gsm == ERR_TIMEOUT)
         return ERR_GPRS_TIMEOUT;
   }

   return ERR_GPRS_ACTIVATE;
}

// соединяемся с сервером по gprs
RET_INFO profile_activate(PROF_CONNECT_SERVER nProf, const char* pNameServ)
{
   int err_gprs = 0;
   GSM_INFO data_profile_gprs_connect;
   err_gprs = 0;
   int count_gprs = GetGsmGprsTimeout();
   int iRxSize = 0;

   if (nProf == PROF_AGPS_SERVER)
   {   //Если соединение для SIM800 и метод HTTP GET, то выйдем без открытия, откроем его по другому.
      return RET_GPRS_OK;
   }

   memset(g_asCmdBuf, 0, sizeof(g_asCmdBuf));
   sprintf(g_asCmdBuf, "%s%d", AT_CIPSTATUS, nProf);

   mc_get(g_asCmdBuf, M_STRING, &data_profile_gprs_connect, 3, 4);

   /* Проверяем если девайс уже подключен к серверу */
   _Bool ser_connect_true = FALSE;
   if (strstr(data_profile_gprs_connect.msg[0].str, "CONNECTING"))
   {
      ser_connect_true = TRUE;
   }
   if (strstr(data_profile_gprs_connect.msg[0].str, "CONNECTED"))
   {
      ser_connect_true = TRUE;
   }

   if (ser_connect_true)
   {   // если подключен, то делаем дисконект
      profile_deactivate(nProf);
      portTickType xLastWakeTimerDelay = xTaskGetTickCount();
      vTaskDelayUntil(&xLastWakeTimerDelay, (SLEEP_MS_1000 / portTICK_RATE_MS));
   }

   char strIP[40] = { 0 };
   char strPORT[6] = { 0 };
   char* pFind = strstr(pNameServ, ":");
   if (!(pFind > 0))
      return ERR_SERVER_CONNECT;   //Если не задан сервер.
   strcpy(strPORT, ++pFind);
   size_t len = strlen(pNameServ);
   loop(len)
   {
      if (pNameServ[i] == ':')
         break;
      strIP[i] = pNameServ[i];
   }
   sprintf(g_asCmdBuf, "at+cipstart=%d,\"TCP\",\"%s\",", nProf, strIP);
   strcat(g_asCmdBuf, strPORT);
   mc(g_asCmdBuf, 3, 1);

   int timeout_ans = GetGsmGprsTimeout();   // ожидание открытия соединения

   loop(timeout_ans)
   {
      iRxSize = gsm_parser(0, &data_profile_gprs_connect, g_asRxBuf, RX_BUFFER_SIZE, 1);
      if (i >= timeout_ans || iRxSize < 0)
      {
         err_gprs = err_gprs * 1;
         break;
      }
      switch (data_profile_gprs_connect.m_type)
      {
      case M_0_CONNECT_OK:
         DP_GSM("__0, CONNECT OK\r\n");
         return RET_GPRS_OK;
         break;
      case M_1_CONNECT_OK:
         DP_GSM("__1, CONNECT OK\r\n");
         return RET_GPRS_OK;
         break;
      case M_0_CONNECT_FAIL:
         DP_GSM("__0, CONNECT FAIL\r\n");
         return ERR_SERVER_CONNECT;
         break;
      case M_1_CONNECT_FAIL:
         DP_GSM("__1, CONNECT FAIL\r\n");
         return ERR_SERVER_CONNECT;
         break;
      case M_SISW:
         if (data_profile_gprs_connect.msg[0].var == nProf && data_profile_gprs_connect.msg[1].var == 1)
         {
            // соединение установлено, модем готов к передаче данных
            DP_GSM("__SISW: %d,%d\r\n", data_profile_gprs_connect.msg[0].var, data_profile_gprs_connect.msg[1].var);
            if (!(nProf == 2))
            {
               return RET_GPRS_OK;
            }
         }
         break;
      case M_SISR:
         if (data_profile_gprs_connect.msg[0].var == nProf && data_profile_gprs_connect.msg[1].var == 1)
         {
            // соединение установлено, модем готов к передаче данных
            DP_GSM("__SISR7: %d,%d\r\n", data_profile_gprs_connect.msg[0].var, data_profile_gprs_connect.msg[1].var);
            if (nProf == 2)
            {
               return RET_FTP_OK;
            }
         }
         break;
      case M_SIS:
         if (data_profile_gprs_connect.msg[0].var == nProf && data_profile_gprs_connect.msg[1].var == 0)
         {
            // ошибка при соединении с сервером (хост не найден, соединение закрыто удаленным хостом и т.д.)
            if (data_profile_gprs_connect.msg[2].var >= 0 && data_profile_gprs_connect.msg[2].var <= 2000)
            {
               // return -1;
               return ERR_SERVER_CONNECT;
            }
         }
         break;
      case M_RDY:
      case M_SIM_PIN:
         // GSM модуль перезагрузился во время выполнения команды
         return ERR_RESTART;
      }

      portTickType xLastWakeTimerDelay = xTaskGetTickCount();
      vTaskDelayUntil(&xLastWakeTimerDelay, (SLEEP_MS_1000 / portTICK_RATE_MS));
   }
   return ERR_TIMEOUT;
}

// отослать данные по gprs
int socket_send(u8 nProf, char* data_buf, int data_size, u8* mask_ready)
{
   int count_gprs = 0;
   int state_gprs = 0;
   int timeout;
   int iRxSize = 0;
   GSM_INFO data_profile_gprs_connect;

   *mask_ready = 0;
   memset(g_asCmdBuf, 0, sizeof(g_asCmdBuf));
   osDelay(100);
   for (count_gprs = 1; count_gprs > 0;)
   {
      osDelay(100);
      if (state_gprs == 0)
      {
         // даём команду о передаче сообщение по TCP/IP соединению
         sprintf(g_asCmdBuf, "%s%d,%d", AT_SEND, nProf, data_size);   // at^sisw=
         strcat(g_asCmdBuf, ADD_SEND_PAR);
         mc_send(g_asCmdBuf, NULL, 0);
         state_gprs = 1;
      }
      if (state_gprs == 1)
      {
         iRxSize = gsm_parser(g_asCmdBuf, &data_profile_gprs_connect, g_asRxBuf, RX_BUFFER_SIZE, 1);
         if (iRxSize <= 0)
         {
            GSM_DC(g_asCmdBuf, 't');
            return (ERR_GPRS_SEND);
         }
         if ((data_profile_gprs_connect.m_type == M_ERROR) || (data_profile_gprs_connect.m_type == M_ABORTED) ||
             (data_profile_gprs_connect.m_type == M_CME_ERROR))
         {
            osDelay(100);
            count_gprs--;
            state_gprs = 0;
            GSM_DC(g_asCmdBuf, '4');
            if (!(count_gprs))
            {
               return (ERR_GPRS_SEND);
            }
         }
         if (data_profile_gprs_connect.m_type == M_RDY)
         {
            // GSM модуль перезагрузился во время выполнения команды
            GSM_DC(g_asCmdBuf, 't');
            return (ERR_RESTART);
         }
         if (data_profile_gprs_connect.m_echo != 0 && data_profile_gprs_connect.m_type == M_SISW &&
             data_profile_gprs_connect.msg[0].var == nProf)
         {
            DP_GSM("D__SISW: %d,%d,%d\r\n",
                   data_profile_gprs_connect.msg[0].var,
                   data_profile_gprs_connect.msg[1].var,
                   data_profile_gprs_connect.msg[2].var);
            break;
         }

         if (data_profile_gprs_connect.m_type == M_PROMPT && data_profile_gprs_connect.msg[0].var == nProf)
         {
            DP_GSM("D__CIPSEND: %d,%d,%d\r\n",
                   data_profile_gprs_connect.msg[0].var,
                   data_profile_gprs_connect.msg[1].var,
                   data_profile_gprs_connect.msg[2].var);
            break;
         }

         if (data_profile_gprs_connect.m_type == M_SIS)
         {
            // ошибка при соединении с сервером (хост не найден, соединение закрыто удаленным хостом и т.д.)
            DP_GSM("D_[i]_SIS: %d,%d,%d\n",
                   data_profile_gprs_connect.msg[0].var,
                   data_profile_gprs_connect.msg[1].var,
                   data_profile_gprs_connect.msg[2].var);
            if (data_profile_gprs_connect.msg[0].var == nProf && data_profile_gprs_connect.msg[1].var == 0)
            {
               if (data_profile_gprs_connect.msg[2].var >= 0 && data_profile_gprs_connect.msg[2].var <= 2000)
               {
                  return (ERR_SERVER_CONNECT);
               }
            }
         }

         if ((data_profile_gprs_connect.m_type == M_0_CLOSED || data_profile_gprs_connect.m_type == M_1_CLOSED) &&
             data_profile_gprs_connect.msg[0].var == nProf)
         {
            // Удаленный сервер закрыл соединение
            DP_GSM("D__CLOSED");
            return ERR_SERVER_CONNECT;
         }
      }
   }

   // передаем данные
   gsm_write((char*)data_buf, data_size, 0);   // 3 параметр - таймаут на отправку данных

#if OUT_DEBUG_DATA_SER == 1
   DP_GSM("D_GPRS TX-> ");
   for (int iRxCount = 0; iRxCount < data_size; iRxCount++)
   {
      DP_GSM("%02X ", data_buf[iRxCount]);
   }
   DP_GSM("\r\n");
#endif

   for (timeout = 100; timeout > 0; timeout--)
   {
      iRxSize = gsm_parser(0, &data_profile_gprs_connect, g_asRxBuf, RX_BUFFER_SIZE, GetGsmGprsTimeout());
      if (iRxSize <= 0)
      {
         GSM_DC(g_asCmdBuf, '4');
         return (ERR_GPRS_SEND);
      }

      if (data_profile_gprs_connect.m_type == M_RDY)
      {
         // GSM модуль перезагрузился во время выполнения команды
         GSM_DC(g_asCmdBuf, 't');
         return (ERR_RESTART);
      }

      if ((data_profile_gprs_connect.m_type == M_OK) || (iRxSize))
      {
         GSM_DC(g_asCmdBuf, '0');
         break;
      }

      if ((data_profile_gprs_connect.m_type == M_0_CLOSED || data_profile_gprs_connect.m_type == M_1_CLOSED) &&
          data_profile_gprs_connect.msg[0].var == nProf)
      {
         // Удаленный сервер закрыл соединение
         DP_GSM("D__CLOSED");
         return ERR_SERVER_CONNECT;
      }
   }
   if (!(timeout > 0))
   {
      DP_GSM("D__ABORTED");
      return ERR_ABORTED;
   }

   // ждём сообщение об окончании передачи данных
   for (count_gprs = GetAnswerTimeout(); /*WAIT_SEND_OK*/ count_gprs > 0; count_gprs--)
   {
      iRxSize = gsm_parser(g_asCmdBuf, &data_profile_gprs_connect, g_asRxBuf, RX_BUFFER_SIZE, 1);

      if (data_profile_gprs_connect.m_type == M_SISW && data_profile_gprs_connect.msg[0].var == nProf)
      {
         DP_GSM("D__SISW: %d,%d\r\n", data_profile_gprs_connect.msg[0].var, data_profile_gprs_connect.msg[1].var);
         if (data_profile_gprs_connect.msg[1].var == 1)
         {
            *mask_ready |= TX_READY;
            return RET_GPRS_SEND_OK;
         }
      }

      if ((data_profile_gprs_connect.m_type == M_0_SEND_OK || data_profile_gprs_connect.m_type == M_1_SEND_OK) &&
          data_profile_gprs_connect.msg[0].var == nProf)
      {
         DP_GSM("D__SEND OK: %d,%d\r\n", data_profile_gprs_connect.msg[0].var, data_profile_gprs_connect.msg[1].var);
         *mask_ready |= TX_READY;
         return RET_GPRS_SEND_OK;
      }

      if (data_profile_gprs_connect.m_type == M_SISR && data_profile_gprs_connect.msg[0].var == nProf)
      {
         DP_GSM("D__SISR6: %d,%d\r\n", data_profile_gprs_connect.msg[0].var, data_profile_gprs_connect.msg[1].var);
         if (data_profile_gprs_connect.msg[1].var == 1)
         {
            *mask_ready |= RX_READY;
         }
      }

      if ((data_profile_gprs_connect.m_type == M_0_CLOSED || data_profile_gprs_connect.m_type == M_1_CLOSED) &&
          data_profile_gprs_connect.msg[0].var == nProf)
      {
         // Удаленный сервер закрыл соединение
         DP_GSM("D__CLOSED");
         return ERR_SERVER_CONNECT;
      }

      if (data_profile_gprs_connect.m_type == M_RDY)
      {
         // GSM модуль перезагрузился во время выполнения команды
         return ERR_RESTART;
      }

      // ret_gsm = mc_sisi(nProf, &data_profile_gprs_connect);
      // if(ret_gsm < 0) break;
   }
   return ERR_GPRS_SEND;
}

int socket_read(u8 nProf, char* pBuf, int size)
{
   int len_socket_read = 0;
   int sec_gprs = 10;   // Добавить настройку этого параметра.
   int rd_lenght_socket_read = 0;
   RET_INFO ret_gsm = RET_OK;

   pBuf[0] = 0;
   // memset(g_strRxBufferUSART2, 0, sizeof(g_strRxBufferUSART2));
   for (;;)
   {
      // читаем ответ сервера
      for (;;)
      {
         // Читаем заголовок от команды at^sisr
         // memset(g_asRxBuf, 0, sizeof(g_asRxBuf));
         int rd_size_socket_read = gsm_parser(g_asCmdBuf, &g_stDataGsmInfo, g_asRxBuf, RX_BUFFER_SIZE, sec_gprs);
         if (rd_size_socket_read <= 0)
         {
            return -1;
         }

         if (g_stDataGsmInfo.m_type == M_SISR && g_stDataGsmInfo.msg[0].var == nProf)
         {
            if (g_stDataGsmInfo.m_echo != 0)
            {
               DP_GSM("D_[a]_SISR5: %d,%d\r\n", g_stDataGsmInfo.msg[0].var, g_stDataGsmInfo.msg[1].var);

               len_socket_read = g_stDataGsmInfo.msg[1].var;
               if (len_socket_read <= 0 && rd_lenght_socket_read == 0)
               {
                  GSM_DPD("D_GPRS: Data do not exist", strlen("GPRS: Data do not exist"));
                  return len_socket_read;
               }
               else
               {
                  // Принимаем данные
                  ret_gsm = (RET_INFO)gsm_read((char*)pBuf + rd_lenght_socket_read, len_socket_read, sec_gprs);
                  if (ret_gsm > 0)
                  {
                     rd_lenght_socket_read += ret_gsm;
                  }
                  pBuf[rd_lenght_socket_read] = 0;
               }
            }
            else
            {
               DP_GSM("D_[i]_SISR4: %d,%d\n", g_stDataGsmInfo.msg[0].var, g_stDataGsmInfo.msg[1].var);
               if (len_socket_read <= 0 && rd_lenght_socket_read == 0)
               {
                  GSM_DPD("D_GPRS: Data do not exist\r\n", strlen("GPRS: Data do not exist\r\n"));
               }
               if (g_stDataGsmInfo.msg[1].var == 1)
               {
                  // GSM_DPD("SRV: Socket closed\r\n", strlen("SRV: Socket closed\r\n"));        return
                  // rd_lenght_socket_read;
               }
               if (g_stDataGsmInfo.msg[1].var == 2)
               {
                  GSM_DPD("D_GPRS: Socket closed\r\n", strlen("GPRS: Socket closed\r\n"));
                  return -2;
               }
            }
         }
         /* Обработчик SIM800 GPRS */
         if (g_stDataGsmInfo.m_type == M_RECEIVE && g_stDataGsmInfo.msg[0].var == nProf)
         {
            DP_GSM("D_[a]_RECEIVE: %d,%d\r\n", g_stDataGsmInfo.msg[1].var, g_stDataGsmInfo.msg[2].var);
            len_socket_read = g_stDataGsmInfo.msg[2].var;
            if (!(len_socket_read > 0))
            {   //А данных нет :(
               GSM_DPD("D_GPRS: Data do not exist", strlen("SRV: Data do not exist"));
               return 0;
            }
            // Принимаем данные
            ret_gsm = (RET_INFO)gsm_read((char*)pBuf + rd_lenght_socket_read, len_socket_read, sec_gprs);
            if (ret_gsm > 0)
            {
               rd_lenght_socket_read += ret_gsm;
            }
            pBuf[rd_lenght_socket_read] = 0;
            //Ни какого OK мы не ждем.
            return rd_lenght_socket_read;
         }

         /* Обработчик SIM800 HTTP GET */
         if (g_stDataGsmInfo.m_type == M_HTTPACTION && g_stDataGsmInfo.msg[0].var == GET)
         {
            if (g_stDataGsmInfo.msg[1].var != 200)
            {
               // Connect Error
               DP_GSM("D_HTTP ERR: %i\r\n", g_stDataGsmInfo.msg[1].var);
               return g_stDataGsmInfo.msg[1].var * -1;
            }
            GSM_DPD("D_HTTP OK\r\n", strlen("D_HTTP OK\r\n"));
            len_socket_read = g_stDataGsmInfo.msg[2].var;
            return len_socket_read;
         }

         /* Обработчик SIM800 HTTP READ */
         if (g_stDataGsmInfo.m_type == M_HTTPREAD)
         {
            len_socket_read = g_stDataGsmInfo.msg[0].var;
            if (!(len_socket_read > 0))
            {   //А данных нет :(
               GSM_DPD("D_HTTP: Data do not exist", strlen("D_HTTP: Data do not exist"));
               return 0;
            }
            // Принимаем данные
            ret_gsm = (RET_INFO)gsm_read((char*)pBuf + rd_lenght_socket_read, len_socket_read, sec_gprs);
            if (ret_gsm == 0)
            {
               rd_lenght_socket_read += len_socket_read;
            }
            else
            {
               rd_lenght_socket_read = ret_gsm;
            }
            pBuf[rd_lenght_socket_read] = 0;
            break;
         }

         /* for BGS2 */
         if (g_stDataGsmInfo.m_type == M_OK)
         {
            GSM_DC(g_asCmdBuf, '0');
            // if(len_socket_read <= 0) return rd_lenght_socket_read;
            if ((len_socket_read <= 0 || rd_lenght_socket_read >= size))
            {
               return rd_lenght_socket_read;
            }
            if (rd_lenght_socket_read)
            {
               return rd_lenght_socket_read;
            }
            break;
         }
      }
   }
}

RET_INFO wait_rx_ready(uint8_t nProf, uint16_t sec)
{
   int len_socket_read = 0;

   memset(&g_stDataGsmInfo, 0, sizeof(g_stDataGsmInfo));

   // ждём инициативное сообщение at^sisr=0,1(не для SIM800)
   int ret_gsm = gsm_parser(0, &g_stDataGsmInfo, g_asRxBuf, RX_BUFFER_SIZE, sec);
   if (ret_gsm >= 0)
   {
      switch (g_stDataGsmInfo.m_type)
      {
      case M_SISR:
         if (g_stDataGsmInfo.msg[0].var == nProf)
         {
            len_socket_read = g_stDataGsmInfo.msg[1].var;
            DP_GSM("D_[i]_SISR3: %d,%d\n", g_stDataGsmInfo.msg[0].var, g_stDataGsmInfo.msg[1].var);
            if (len_socket_read == 2 || len_socket_read < 0)
            {
               // соединение закрыто данных больше не будет
               DP_GSM("D_[-] GSM: Server closed connection\n");
               return ERR_SERVER_CONNECT;
            }
            else if (len_socket_read == 1)
            {
               // получено инициативное сообщение о приходе данных
               return RET_GPRS_RECEIVE_OK;
            }
         }
         break;

      case M_RECEIVE:   //Пришли данные (SIM800)
         if (g_stDataGsmInfo.msg[0].var == nProf)
         {
            DP_GSM("D_[i]_RECEIVE: %d\n", g_stDataGsmInfo.msg[0].var);
            return RET_GPRS_RECEIVE_OK;
         }
         break;

      case M_SIS:
         if (g_stDataGsmInfo.msg[0].var == nProf && g_stDataGsmInfo.msg[1].var == 0)
         {
            DP_GSM("D_[i]_SIS: %d,%d,%d\n",
                   g_stDataGsmInfo.msg[0].var,
                   g_stDataGsmInfo.msg[1].var,
                   g_stDataGsmInfo.msg[2].var);
            if (g_stDataGsmInfo.msg[2].var >= 0 && g_stDataGsmInfo.msg[2].var <= 2000)
            {
               // произошла ошибка в работе
               DP_GSM("D_[-] GSM: Server closed connection\n");
               return ERR_SERVER_CONNECT;
            }
         }
         break;
      }
   }
   return ERR_SERVER_NO_ANSWER;
}

// закрываем gprs соединение
RET_INFO profile_deactivate(uint8_t nProf)
{
   RET_INFO res;
   GSM_INFO data_profile_gprs_connect;
   memset(&data_profile_gprs_connect, 0, sizeof(data_profile_gprs_connect));
   memset(g_asCmdBuf, 0, sizeof(g_asCmdBuf));
   sprintf(g_asCmdBuf, "%s%d", AT_CIPSTATUS, nProf);

   mc_get(g_asCmdBuf, M_STRING, &data_profile_gprs_connect, 3, 4);

   if (strstr(data_profile_gprs_connect.msg[0].str, "CLOSING"))
   {
      return (RET_OK);
   }
   if (strstr(data_profile_gprs_connect.msg[0].str, "CLOSED"))
   {
      return (RET_OK);
   }
   if (strstr(data_profile_gprs_connect.msg[0].str, "INITIAL"))
   {
      return (RET_OK);
   }

   if (data_profile_gprs_connect.m_type == M_SISI)
   {
      if ((data_profile_gprs_connect.msg[0].var == nProf) &&
          ((data_profile_gprs_connect.msg[1].var == CLOSING) || (data_profile_gprs_connect.msg[1].var == DOWN) ||
           (data_profile_gprs_connect.msg[1].var == ALLOCATED)))
      {
         return (RET_OK);
      }
   }

   sprintf(g_asCmdBuf, "%s%d", AT_CIPCLOSE, nProf);   // Close Service
   res = mc(g_asCmdBuf, 5, 1);

   if ((res == RET_OK) || (res == RET_CLOSE1) || (res == RET_CLOSE0) || (res == ERR_CMD))
   {
      return RET_OK;
   }

   return res;
}

#pragma optimize = none
static _Bool configUserApn(_Bool bTypeProf)
{
   char strApn[LOGIN_PASS_SIZE] = { '\0' };
   char strPassword[LOGIN_PASS_SIZE] = { '\0' };
   char strLogin[LOGIN_PASS_SIZE] = { '\0' };

   /* Взависимости от номера SIM карты проверим и подставим нужный нам user, passwd, apn. */
   if (SelectNumSim(GET_NUM_SIM) == CURRENT_FIRST_SIMCARD)
   {
      /* First SIM Card */
      // даем настройки GPRS APN, strLogin и strPassword.
      char strFIRST_SCID[SIZE_SCID] = { '\0' };   // SCID заводской СИМ карты
      GetScidFirstSim(strFIRST_SCID);

      char strRamFIRST_SCID[SIZE_SCID];   // SCID текущей СИМ
      if (GetScidCurentFirstSim(strRamFIRST_SCID) == 0)
      {
         memset(strRamFIRST_SCID, 0xFF, SIZE_SCID);
      }

      /* Если SCID отличаются, то проверим настройки пользовательские GPRS */
      if ((strcmp(strRamFIRST_SCID, strFIRST_SCID)) && (GetManualModeSimFirst()) && (GetNameUserSimApn(strApn)))
      {
         GetNameUserSimPass(strPassword);
         GetNameUserSimLogin(strLogin);
         SetupInternetConnectionProfile(strLogin, strPassword, strApn, bTypeProf);   // PARAM: user, passwd, apn.
         return TRUE;
      }
   }
   else
   {
      /* Second SIM Card */
      // даем настройки GPRS APN, strLogin и strPassword.
      if (GetManualModeSimSecond() && getNameUserSecondSimApn(strApn))
      {
         getNameUserSecondSimPass(strPassword);
         getNameUserSecondSimLogin(strLogin);
         SetupInternetConnectionProfile(strLogin, strPassword, strApn, bTypeProf);   // PARAM: user, passwd, apn.
         return TRUE;
      }
   }
   return FALSE;
}