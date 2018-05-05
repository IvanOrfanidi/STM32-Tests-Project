#include "includes.h"
#include "gsm_mc52iT.h"

int gsm_read(char* pBuf, int size, uint32_t wait)
{
   int n;
   int Len;
   for (n = 0; n < size;)
   {
      Len = USART_Rx_Len(UART_GSM);
      if (Len)   //допускается переполнение только при чтении смс поэтому добавлен второй флаг
      {
         USART_Read(UART_GSM, pBuf + n, 1);
         n++;
      }
      else
      {
         if (GSM_STATUS_ON == 0)
         {
            // GSM модуль выключился во время работы
            return -1;
         }
         if (isEndWaitPeriod(wait * 10))
            break;
      }
   }
   return n;
}

int gsm_gets(char* pBuf, int size, uint32_t wait)
{
   int n;

   for (n = 0; n < size;)
   {
      int Len = USART_Rx_Len(UART_GSM);

      if (Len > 0)
      {
         USART_Read(UART_GSM, pBuf + n, 1);
         if (pBuf[n] != '\r')
         {
            if (pBuf[n] == '\n')
            {
               if (n > 0)
                  break;
            }
            else
            {
               n++;
            }
         }
      }
      else
      {
         if (GSM_STATUS_ON == 0)
         {
            // GSM модуль выключился во время работы
            return -1;
         }

         if (isEndWaitPeriod(wait * 10))
            break;
      }
   }

   if (n < size)
      pBuf[n] = 0;

   return n;
}

void gsm_write(const char* pBuf, int size, uint32_t pause)
{
   if (pause != 0)
   {
      for (int n = 0; n < size; n++)
      {
         USART_Write(UART_GSM, &pBuf[n], 1);
         isEndWaitPeriod(pause);
      }
   }
   else
   {
      USART_Write(UART_GSM, pBuf, size);
   }
}

void mc_send(const char* pCmd, char* pPrm, uint32_t pause)
{
   int size_mc = 0;
   // выполняем команду
   if (pCmd != 0)
   {
      size_mc = strlen(pCmd);
      if (size_mc > 0)
      {
         gsm_write(pCmd, size_mc, pause);

         if (pPrm != 0)
         {
            size_mc = strlen(pPrm);
            if (size_mc > 0)
            {
               gsm_write(pPrm, size_mc, pause);
            }
         }
         gsm_write("\r", 1, pause);
      }
   }
}

RET_INFO WakeupGSM(_Bool State)
{
#define MAX_CFUN 1;
   RET_INFO ret = ERR_CMD;

   uint8_t cnt = MAX_CFUN;
   if (State)
   {
      while ((ret != RET_OK) && (cnt))
      {
         ret = mc("at+cfun=1", 4, MC_COUNT);
         cnt--;
      }
   }
   else
   {
      while ((ret != RET_OK) && (cnt))
      {
         ret = mc("at+cfun=0", 4, MC_COUNT);
         cnt--;
      }
   }

   return ret;
}

// выполнить команду и получить в ответ данные
// данные приходят сразу за командой и состоят из одной строки
int mc_get(const char* pCmd, u8 m_type, GSM_INFO* pOut, u8 count, u32 second)
{
   GSM_INFO out_mc;
   memset(&out_mc, 0, sizeof(out_mc));
   int size_mc = 0;
   int state_mc = -1;
   int n_size = 0;
   unsigned int uiRxSize = 0;
   memset(pOut, 0, sizeof(GSM_INFO));

   for (; count > 0; count--)
   {
      // выполняем команду
      mc_send(pCmd, 0, 0);
      // osDelay(100);
      for (;;)
      {
         uiRxSize = gsm_parser(pCmd, &out_mc, g_asRxBuf + size_mc, sizeof(g_asRxBuf) - size_mc, second);
         if (uiRxSize == 0)
         {
            // истекло время ожидания ответа на команду
            // GSM_DC(pCmd, 't');
            DS_GSM("[t] ", pCmd);
            return ERR_TIMEOUT;
            break;
         }

         if (state_mc < 0)
         {
            if (m_type == out_mc.m_type || m_type == M_OTHER)
            {
               // копируем принятые данные
               switch (out_mc.m_type)
               {
               case M_CSQ:
               case M_CGATT:
               case M_COPS:
               case M_CREG:
               case M_SCID:
               case M_SISI:
               case M_SISW:
               case M_0_CONNECT_OK:
               case M_1_CONNECT_OK:
               case M_0_SEND_OK:
               case M_1_SEND_OK:
               case M_RECEIVE:
               case M_SISR:
               case M_SCTM:
               case M_STRING:
               case M_SPIC:
               case M_FTPGET:
                  for (n_size = 0; n_size < out_mc.count; n_size++)
                  {
                     size_mc += out_mc.msg[n_size].size + 1;
                  }
               case M_SIM_PIN:
               case M_SIM_PUK:
               case M_SIM_READY:
               case M_SIND_NITZ:
                  memcpy(pOut, &out_mc, sizeof(GSM_INFO));
                  state_mc++;
                  break;
               }
            }
            else if (m_type == M_OTHER_PIN)
            {
               // копируем принятые данные
               switch (out_mc.m_type)
               {
               case M_SIM_PIN:
               case M_SIM_PUK:
               case M_SIM_READY:
                  pOut->m_type = out_mc.m_type;
                  state_mc++;
                  break;
               }
            }
         }

         if (out_mc.m_type == M_OK)
         {
            // команда выполнена
            // GSM_DC(pCmd, '0');
            DS_GSM("[+] ", pCmd);

            if (state_mc == 0)
            {
               // данные не получены
               return ERR_TIMEOUT;
            }

            // данные не получены
            return ERR_CMD;
            break;
         }

         if (out_mc.m_type == M_ERROR || out_mc.m_type == M_ABORTED)
         {
            // получен ответ неверная команда
            // GSM_DC(pCmd, '4');
            DS_GSM("[-] ", pCmd);
            return ERR_CMD;
            break;
         }
         if (out_mc.m_type == M_SYSSTART)
         {
            // GSM модуль перезагрузился во время выполнения команды
            return ERR_RESTART;
         }
         // ответ не распознан
      }
   }

   return out_mc.m_type;
}

RET_INFO isOK(const char* pCmd, uint32_t wait)
{
   unsigned int uiRxSize = 0;
   GSM_INFO data_mc;
   memset(&data_mc, 0, sizeof(data_mc));
   for (;;)
   {
      uiRxSize = gsm_parser(pCmd, &data_mc, g_asRxBuf, sizeof(g_asRxBuf), wait);
      if (uiRxSize == 0)
      {
         // истекло время ожидания ответа на команду
         // GSM_DC(pCmd, 't');
         DS_GSM("[t] ", pCmd);
         return ERR_TIMEOUT;
      }

      switch (data_mc.m_type)
      {
      case M_OK:
         // команда выполнена
         // GSM_DC(pCmd,'0');
         DS_GSM("[+] ", pCmd);
         return RET_OK;
      case M_CME_ERROR:
      case M_ERROR:
         // получен ответ не верная команда
         // GSM_DC(pCmd, '4');
         DS_GSM("[-]", pCmd);
         return ERR_CMD;
      case M_ABORTED:
         // получен ответ команда прервана
         // GSM_DC(pCmd, '4');
         DS_GSM("[-] ", pCmd);
         return ERR_ABORTED;
      case M_0_CLOSE_OK:
         DS_GSM("[+] ", pCmd);
         return RET_CLOSE0;   //Закрытие соединения
      case M_1_CLOSE_OK:
         DS_GSM("[+] ", pCmd);
         return RET_CLOSE1;   //Закрытие соединения
      case M_SYSSTART:
         // GSM модуль перезагрузился во время выполнения команды
         return ERR_RESTART;
      }
      // ответ не распознан
   }
}

RET_INFO modem_setting_start(void)
{
   // mc("at", 4);
   // не установлена скорость работы
   mc_send("at+ipr=115200", NULL, 0);
   if (isOK("at+ipr=115200", 2) == RET_OK)
   {
      return RET_OK;
   }
   return ERR_POWER;
}

RET_INFO modem_on(void)
{
   GSM_INFO data_mc;
   memset(&data_mc, 0, sizeof(data_mc));

   // WakeupGSM(1);

   gsm_parser(0, &data_mc, g_asRxBuf, sizeof(g_asRxBuf), 10);

   if (data_mc.m_type == M_SYSSTART)
   {
      DP_GSM("D_SYSSTART\r\n");
      return RET_OK;
   }

   if (data_mc.m_type == M_RDY)
   {
      DP_GSM("D_RDY\r\n");
      osDelay(2000);
      return RET_OK;
   }

   if (data_mc.m_type == M_SIM_READY)
   {
      DP_GSM("D_CPIN: READY\r\n");
      osDelay(2000);
      return RET_OK;
   }

   if (data_mc.m_type == M_CALL_READY)
   {
      DP_GSM("D_CALL READY\r\n");
      osDelay(2000);
      return RET_OK;
   }

   // mc(ATZ, 5, MC_COUNT);
   return ERR_TIMEOUT;
}

RET_INFO modem_second_precmd(void)
{
   //Конфигурим LED под мигание.
   mc(AT_CONFIG_LED, 5, MC_COUNT);
   osDelay(10);

#if defined(MULTI_IP_CONNECTION_ENABLE) && defined(AT_CIPMUX)   // Multi-IP Connection
   if (mc(AT_CIPMUX, 5, MC_COUNT) != RET_OK)
   {
      return ERR_TIMEOUT;
   }
   osDelay(10);
#endif

   // AT+CNETSCAN=1
   mc(AT_CNETSCAN1, 5, MC_COUNT);   // Show lac and bsic information
   osDelay(10);

   // AT+CIPHEAD=1
   mc(AT_CIPHEAD1, 5, MC_COUNT);   //включить сообщение о количестве байт принятых по GPRS перед самими данными
   osDelay(10);

#ifdef SMS_PDU
   osDelay(10);
   // PDU режим работы с смс
   if (mc("at+cmgf=0", 5, MC_COUNT) != RET_OK)
   {
      return ERR_TIMEOUT;
   }
#endif

#ifdef SMS_TEXT
   osDelay(10);
   // текстовый режим работы с смс
   if (mc("at+cmgf=1", 5, MC_COUNT) != RET_OK)
   {
      return ERR_TIMEOUT;
   }
#endif

#ifdef CLI_ENABLE
   osDelay(10);
   //включить определение входящего номера
   if (mc("at+clip=1", 5, MC_COUNT) != RET_OK)
   {
      return ERR_TIMEOUT;
   }
#endif

   osDelay(10);
   if (mc("at&w", 4, MC_COUNT) != RET_OK)
   {
      return ERR_TIMEOUT;
   }

   return RET_OK;
}

RET_INFO modem_precmd(void)
{
   GSM_INFO data_mc;
   // включить эхо
   if (mc("ate1", 4, MC_COUNT) != RET_OK)
   {
      return ERR_TIMEOUT;
   }
   osDelay(10);
   /*
   if(mc("at+cfun=1", 4, MC_COUNT) != RET_OK) {
      return ERR_TIMEOUT;
   }
   osDelay(10);
   if(mc("at+csclk=0", 4, MC_COUNT) != RET_OK) {
      return ERR_TIMEOUT;
   }
   osDelay(10);
   */
   //Уровень информации об ошибке. 0, — отключено. Будет просто писать ERROR. 1, — код ошибки. Будет возвращать цифровой
   //код ошибки. 2, — описание ошибки.
   if (mc("at+cmee=2", 4, MC_COUNT) != RET_OK)
   {
      return ERR_TIMEOUT;
   }
   osDelay(10);

   //Получим идентификатор GSM Модема. (GSM900 / GSM1800 / UMTS2100 /LTE1800 / LTE2600)
   mc_get("at+gmm", M_STRING, &data_mc, 3, 3);
   if (data_mc.m_type == M_STRING)
   {
      data_mc.msg[0].str[sizeof(g_stRam.stGsm.strGsmModemIdentification) - 1] = 0;
      sprintf(g_stRam.stGsm.strGsmModemIdentification, "%s", data_mc.msg[0].str);
      DS_GSM("D_GMM: ", g_stRam.stGsm.strGsmModemIdentification);
   }

   //Получить версию ПО GSM Модема.
   g_stRam.stGsm.eGsmType = GSM_ERROR_TYPE;
   mc_get("at+gmr", M_STRING, &data_mc, 3, 3);
   if (data_mc.m_type == M_STRING)
   {
      char* FindRev = strstr((char*)data_mc.msg[0].str, "REVISION ");
      if (FindRev > 0)
      {
         if (strlen(data_mc.msg[0].str) - strlen("REVISION ") < sizeof(g_stRam.stGsm.strGsmModemSoftware))
         {
            memcpy(g_stRam.stGsm.strGsmModemSoftware,
                   FindRev + strlen("REVISION "),
                   strlen(data_mc.msg[0].str) - strlen("REVISION "));
            g_stRam.stGsm.eGsmType = CINTERION;
            DS_GSM("D_GMR:", g_stRam.stGsm.strGsmModemSoftware);
         }
         else
         {
            DS_GSM("D_GMR: ERROR ", data_mc.msg[0].str);
         }
      }

      FindRev = strstr((char*)data_mc.msg[0].str, "Revision:");
      if (FindRev > 0)
      {
         if (strlen(data_mc.msg[0].str) - strlen("Revision:") < sizeof(g_stRam.stGsm.strGsmModemSoftware))
         {
            memcpy(g_stRam.stGsm.strGsmModemSoftware,
                   FindRev + strlen("Revision:"),
                   strlen(data_mc.msg[0].str) - strlen("Revision:"));
            g_stRam.stGsm.eGsmType = SIMCOM;
            DS_GSM("D_GMR: ", g_stRam.stGsm.strGsmModemSoftware);
         }
         else
         {
            DS_GSM("D_GMR: ERROR ", data_mc.msg[0].str);
         }
      }
   }

   g_stRam.stGsm.ucIMEI = GetGsmIMEI(g_stRam.stGsm.strStrIMEI);   // IMEI GSM-модуля

   return RET_OK;
}

// выполнить команду
// 1 - имя команды
// 2 - таймаут команды
// 3 - количество попыток ввода команды
RET_INFO mc(const char* pCmd, u32 second, int count)
{
   RET_INFO ret;

   if (!(strlen(pCmd)))
   {
      return RET_OK;
   }

   if (!(count))
   {
      count = MC_COUNT;
   }

   for (; count > 0; count--)
   {
      // выполняем команду
      mc_send(pCmd, 0, 0);

      // ждём  OK
      ret = isOK(pCmd, second);
      switch (ret)
      {
      case RET_OK:
         return ret;
      case RET_CLOSE0:
         return ret;
      case RET_CLOSE1:
         return ret;
      }
   }
   return ret;
}

int ServerConnect(char* pNameServ, u8 usProf)
{
#ifdef GSM_MODULE_SIM800
   return 0;
#endif
   int size;
   if ((usProf == PROF_HTTP_GET_SERVER) || (usProf == PROF_HTTP_POST_SERVER))
   {
      return 0;   //Здесь выходим, ссылку на файл подставим позже.
   }
   //Подставляем адрес только для профиля GPRS.
   sprintf(g_asCmdBuf, "at^siss=%d,address,socktcp://", usProf);
   size = strlen(g_asCmdBuf);
   memcpy(g_asCmdBuf + size, pNameServ, strlen(pNameServ));
   mc(g_asCmdBuf, 4, MC_COUNT);
   return 0;
}

int ActivationConnectToGprs(u8 usProf)
{
#ifdef GSM_MODULE_SIM800
   GSM_INFO data_mc;
   for (uint8_t i = 0; i < 5; i++)
   {
      memset(&data_mc, 0, sizeof(GSM_INFO));
      osDelay(1000);
      u8 n;
      for (n = MC_COUNT; n > 0; n--)
      {
         // активируем контекст SIM800
         if (mc("at+ciicr", 10, 1) == RET_OK)
         {
            break;
         }
         osDelay(1000);
      }

      if (n < 1)
      {
         return 1;
      }
      memset(&data_mc, 0, sizeof(GSM_INFO));
      // проверяем назначенный IP адрес
      mc_get("at+cifsr", M_STRING, &data_mc, 1, 1);
      if (strlen(data_mc.msg[0].str))
      {
         DS_GSM("IP: ", data_mc.msg[0].str);
         return 0;
      }
      osDelay(1000);
   }

   return 1;
#else
   if ((usProf == PROF_HTTP_GET_SERVER) || (usProf == PROF_HTTP_POST_SERVER))
   {
      sprintf(g_asCmdBuf, "at^siss=%d,srvType,\"Http\"", usProf);   // Select service type HTTP.
      mc(g_asCmdBuf, 4, MC_COUNT);
      sprintf(g_asCmdBuf, "at^siss=%d,conId,\"0\"", usProf);   // Select connection profile 0
      mc(g_asCmdBuf, 4, MC_COUNT);
      if (usProf == PROF_HTTP_GET_SERVER)
      {
         sprintf(g_asCmdBuf, "at^siss=%d,hcMethod,0", usProf);   // Select "GET" method.
      }
      else
      {
         sprintf(g_asCmdBuf, "at^siss=%d,hcMethod,1", usProf);   // Select "POST" method.
      }
      mc(g_asCmdBuf, 4, MC_COUNT);
      return 0;
   }

   sprintf(g_asCmdBuf, "at^siss=%d,srvtype,socket", usProf);
   mc(g_asCmdBuf, 4, MC_COUNT);
   sprintf(g_asCmdBuf, "at^siss=%d,alphabet,1", usProf);
   mc(g_asCmdBuf, 4, MC_COUNT);
   sprintf(g_asCmdBuf, "at^siss=%d,conid,0", usProf);
   mc(g_asCmdBuf, 4, MC_COUNT);
   return 0;
#endif
}

// INPUT PARAM: user, passwd, apn.
void SetupInternetConnectionProfile(const char* pUser, const char* pPasswd, const char* pApn, _Bool bTypeProf)
{
   char strTempBuf[100];

#ifdef GSM_MODULE_SIM800
   if (bTypeProf)
   {
      sprintf(strTempBuf, "at+sapbr=3,1,\"APN\",\"%s\"", pApn);
      mc(strTempBuf, 4, MC_COUNT);

      sprintf(strTempBuf, "at+sapbr=3,1,\"USER\",\"%s\"", pUser);
      mc(strTempBuf, 4, MC_COUNT);

      sprintf(strTempBuf, "at+sapbr=3,1,\"PWD\",\"%s\"", pPasswd);
      mc(strTempBuf, 4, MC_COUNT);
   }
   else
   {
      strcpy(strTempBuf, "at+cstt=\"");   //Вводим имя точки APN
      strcat(strTempBuf, pApn);
      strcat(strTempBuf, "\",");

      if (strlen(pUser))
      {   //Вводим пользователя точки APN
         strcat(strTempBuf, "\"");
         strcat(strTempBuf, pUser);
         strcat(strTempBuf, "\",");
      }

      if (strlen(pPasswd))
      {   //Вводим пароль  точки APN
         strcat(strTempBuf, "\"");
         strcat(strTempBuf, pPasswd);
         strcat(strTempBuf, "\"");
      }
      mc(strTempBuf, 4, MC_COUNT);
   }

#else

   if (strlen(pUser))
   {   //Вводим пользователя точки APN
      strcpy(strTempBuf, "at^sics=0,user,\"");
      strcat(strTempBuf, pUser);
      strcat(strTempBuf, "\"");
      mc(strTempBuf, 4, MC_COUNT);
   }

   if (strlen(pPasswd))
   {   //Вводим пароль  точки APN
      strcpy(strTempBuf, "at^sics=0,passwd,\"");
      strcat(strTempBuf, pPasswd);
      strcat(strTempBuf, "\"");
      mc(strTempBuf, 4, MC_COUNT);
   }

   if (strlen(pApn))
   {   //Вводим имя точки APN
      strcpy(strTempBuf, "at^sics=0,apn,\"");
      strcat(strTempBuf, pApn);
      strcat(strTempBuf, "\"");
      mc(strTempBuf, 4, MC_COUNT);
   }

#endif
}

_Bool GsmModemConnectGprsService(void)
{
   int ret_gsm = 0;
#ifdef GSM_MODULE_SIM800
   GSM_INFO data_mc;
   ret_gsm = 1;
   for (u8 i = 0; i < DEF_GPRS_RECEIVE_COUNT; i++)
   {
      memset(&data_mc, 0, sizeof(data_mc));
      mc_get(AT_CGATT, M_CGATT, &data_mc, 1, 4);
      if (data_mc.m_type == M_CGATT)
      {
         if (data_mc.msg[0].var == 1)
         {
            ret_gsm = 0;
            break;
         }
         else
         {
            osDelay(1000);
            mc("at+cgatt=1", 1, MC_COUNT);
         }
      }
   }
#else
   ret_gsm |= mc("at^sics=0,contype,gprs0", 10, MC_COUNT);
   osDelay(100);
   ret_gsm |= mc("at^sics=0,alphabet,1", 10, MC_COUNT);
   osDelay(100);
#endif

   return ret_gsm;
}

// инициализация SIM
M_INFO SimCardInit(void)
{
   GSM_INFO data_mc;
   memset(&data_mc, 0, sizeof(data_mc));

   if (GetSimSCID(g_stRam.stSim.strUSER_SCID))
   {
      return M_OTHER;
   }

   // Ввод PIN кода
   mc_get("at+cpin?", M_OTHER_PIN, &data_mc, 3, 3);
   return (M_INFO)(data_mc.m_type);
}

M_INFO SimCardPin(void)
{
   GSM_INFO data_mc;
   memset(&data_mc, 0, sizeof(data_mc));
   // Спросим количество попыток ввода pin.
   mc_get(AT_SPIC, M_SPIC, &data_mc, 3, 4);
   if (data_mc.m_type == M_SPIC)
   {
      if (data_mc.msg[0].var < 2)
      {
         return (M_SIM_PUK);
      }
   }

   sprintf(g_asCmdBuf, "at+cpin=1111");
   mc(g_asCmdBuf, 10, 1);

   return M_OK;
}

uint32_t get_gsm_time(void)
{
   RTC_t Date_GSM_RTC;
   GSM_INFO data_mc;
   memset(&data_mc, 0, sizeof(data_mc));
   mc_get("at+cclk?", M_STRING, &data_mc, 3, 3);

   char temp[3];
   memcpy(temp, &data_mc.msg[0].str[23], sizeof(temp));   //Получаем сек.
   Date_GSM_RTC.sec = (int)atof(temp);
   memcpy(temp, &data_mc.msg[0].str[20], sizeof(temp));   //Получаем мин.
   Date_GSM_RTC.min = (int)atof(temp);
   memcpy(temp, &data_mc.msg[0].str[17], sizeof(temp));   //Получаем час.
   Date_GSM_RTC.hour = (int)atof(temp);
   memcpy(temp, &data_mc.msg[0].str[14], sizeof(temp));   //Получаем день.
   Date_GSM_RTC.mday = (int)atof(temp);
   memcpy(temp, &data_mc.msg[0].str[11], sizeof(temp));   //Получаем месяц.
   Date_GSM_RTC.month = (int)atof(temp);
   memcpy(temp, &data_mc.msg[0].str[8], sizeof(temp));   //Получаем год.
   Date_GSM_RTC.year = (int)atof(temp);

   return Date2Sec(&Date_GSM_RTC);
}

void rtc_gsm_settime(const RTC_t* date)
{
   char msg_cclk[40];

   sprintf(msg_cclk,
           "AT+CCLK=\"%02d/%02d/%02d,%02d:%02d:%02d\"",
           date->year,
           date->month,
           date->mday,
           date->hour,
           date->min,
           date->sec);

   mc(msg_cclk, 5, MC_COUNT);
}

// IMEI GSM-модуля
uint64_t GetGsmIMEI(char* pIMEI)
{
   GSM_INFO data_mc;
   while (1)
   {
      memset(&data_mc, 0, sizeof(data_mc));
      mc_get("at+gsn", M_STRING, &data_mc, 3, 3);
      if ((data_mc.m_type == M_STRING) && (isdigit(data_mc.msg[0].str[0])))
      {
         DS_GSM("D_IMEI: ", data_mc.msg[0].str);
         break;
      }
      osDelay(1000);
   }
   strcpy(pIMEI, data_mc.msg[0].str);
   return (uint64_t)atof(data_mc.msg[0].str);
}

int GetSimSCID(char* pSCID)
{
   // SCID SIM-карты
   uint8_t err_scid = 3;
   GSM_INFO data_mc;
   memset(&data_mc, 0, sizeof(data_mc));
   while ((strlen(data_mc.msg[0].str) < 18) && (err_scid))
   {
      memset(&data_mc, 0, sizeof(data_mc));

#ifdef GSM_MODULE_SIM800
      mc_get(AT_SCID, M_STRING, &data_mc, 3, 3);
      if (data_mc.m_type == M_STRING)
      {
         if (isdigit(data_mc.msg[0].str[0]))
         {   //Проверим корректный SCID
            DS_GSM("D_SCID: ", data_mc.msg[0].str);
            strcpy((char*)pSCID, data_mc.msg[0].str);
            return 0;
         }
      }
#else
      mc_get(AT_SCID, M_SCID, &data_mc, 3, 4);
      if (data_mc.m_type == M_SCID)
      {
         DP_GSM("D_SCID: %s\r", data_mc.msg[0].str);

         strcpy((char*)pSCID, data_mc.msg[0].str);
         return 0;
      }
#endif
      osDelay(1000);
      err_scid--;
   }
   return 1;
}

// выключение модема
void modem_off(void)
{
   GSM_INFO data_mc;
   unsigned int uiRxSize = 0;
   char* cmd2 = AT_SMSO;   //"at^smso";

   mc_send(cmd2, 0, 0);
   uiRxSize = gsm_parser(cmd2, &data_mc, g_asRxBuf, RX_BUFFER_SIZE, 8);
   if (uiRxSize == 0)
   {
      // истекло время ожидания ответа на команду
      GSM_DC(cmd2, 't');
   }

   if (data_mc.m_type == M_OK)
   {
      GSM_DC(cmd2, '0');
   }
   if (data_mc.m_type == M_SHUTDOWN)
   {
      DP_GSM("D_^SHUTDOWN\r");
   }
   if (data_mc.m_type == M_ERROR || data_mc.m_type == M_ABORTED)
   {
      GSM_DC(cmd2, '4');
   }
}

// Функция корекции времени
void TimeSynchronizationGSM(void)
{
   RTC_t DateRTC;
   rtc_gettime(&DateRTC);
   rtc_gsm_settime(&DateRTC);
}
