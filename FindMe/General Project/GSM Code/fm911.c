
#include "includes.h"
#include "fm911.h"
#include "gsm_general.h"

extern char g_aucBufDownHttpFirm[];
#define SERVER_TRUE_VALUE g_aucBufDownHttpFirm

/* Смена сервера с FM уход на iON */
void changeServerFm2iON(void)
{
   deviceDefConfig();   //сброс к дефолтным настройкам девайса
   SetModeProtocol(ION_FM);   //меняем протокол
   SetUseTypeServ(FIRST_SERVER);   //меняем сервер
   SetTypeRegBase911(TYPE_WU_START);   //меняем тип соединения

   resetConnectError();
   if (GetSleepTimeStandart() > MAX_VAL_SLEEP_TIME_STANDART)
   {   // Проверка переполнения заданных режимов пробуждений при уходе с 911.
      SetSleepTimeStandart(DEF_TIME_SLEEP_STANDART_DEVICE);
   }
   if (GetSleepTimeFind() > MAX_VAL_SLEEP_TIME_FIND)
   {   // Проверка переполнения заданных режимов пробуждений при уходе с 911.
      SetSleepTimeFind(DEF_TIME_SLEEP_FIND_DEVICE);
   }
   memset(SERVER_TRUE_VALUE, USER_CONFIG_DEVICE_PACKET, MAX_PARAM_CONFIG_VALUE);
   setGsmStep(GSM_PROFILE_GPRS_CONNECT);
}

/* Режим работы по протоколу Findme2 на сермер 911 ---------------------------*/
static int (*ptrGsmFindme911Scheduler[])(char* pOut, int OffsetData) = {
   FrameBuildSystemFm911,    FrameBuildGpsDataFm911,    FrameBuildGsmDataFm911,
   FrameBuildAccelDataFm911, FrameBuildDeviceDataFm911,
   FrameBuildLogDataFm911,   //Лог работы устройства
};

void stepGsmFindme911DataReady(TGsmStatus* pGsmStatus)
{
   DP_GSM("D_GSM STEP: stepGsmFindme911DataReady\r\n");
   ledStatus(NORMAL_RUN);

   pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;

   T_TYPE_CONNECT eTypeConnect = GetTypeConnectFm911();

   int LenDataGprs = 0;
   uint8_t sum = 0;
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;
   int iRegStatus = 1;
   if (g_stRam.stSim.eRegStatus == ROAMING_NET)
   {
      iRegStatus = 5;
   }

   /* Формируем единый пакет для сервера 911 */
   DP_GSM("\r\nD_PACKET FINDME 911\r\n");

   //Формируем обертку пакета
   char strFmVersia[20];
   getNameFmVersia(strFmVersia);

   char strIMEI[SIZE_IMEI] = { '\0' };
   if (!(GetImeiFirstGsm(strIMEI)))
   {
      GetStrIMEI(strIMEI);
      strcpy(strIMEI, strIMEI);
   }

   char strFIRST_SCID[SIZE_SCID] = { '\0' };
   GetScidCurentFirstSim(strFIRST_SCID);

   sprintf(g_aucOutDataFrameBuffer,
           "$%s;%c;%s;%s,%i,%s;",
           strFmVersia,   // загружаем версию ПО
           eTypeConnect,   // устанавливаем причину соединения
           strIMEI,   // загружаем IMEI GSM
           strFIRST_SCID,   // загружаем SCID SIM-карты
           iRegStatus,   // роуминг/не роуминг
           g_stRam.stSim.acMobCountCode   // код оператора мобильной связи;
   );
   LenDataGprs += strlen(g_aucOutDataFrameBuffer);

   /* Тасовщик функций формирования пакетов FM911 */
   for (uint8_t count_packet = 0; count_packet < (sizeof(ptrGsmFindme911Scheduler) / sizeof(void*)); count_packet++)
   {
      memset(g_asInpDataFrameBuffer, 0, sizeof(g_asInpDataFrameBuffer));
      int res = (*ptrGsmFindme911Scheduler[count_packet])(g_asInpDataFrameBuffer, 0); /* FM911 Scheduler */
      if (res < 0)
      {
         pGsmStatus->eGsmStep = GSM_OFF;
         return;
      }

      LenDataGprs += res;
      if (LenDataGprs >= (sizeof(g_aucOutDataFrameBuffer) - sizeof("*FF\r\n")))
      {
         DP_GSM("D_ERROR SIZE PACKET FINDME 911\r\n");
         break;
      }

      strcat(g_aucOutDataFrameBuffer, g_asInpDataFrameBuffer);
   }

   /* подсчет контрольной суммы */
   sum = gsm_cs(g_aucOutDataFrameBuffer, strlen(g_aucOutDataFrameBuffer));
   /* записываем контрольную сумму */
   sprintf(g_aucOutDataFrameBuffer + strlen(g_aucOutDataFrameBuffer), "*%02X\r\n", sum);
   LenDataGprs = strlen(g_aucOutDataFrameBuffer);

   DP_GSM("\r\nD_GPRS FM911 TX->:\r\n");
   for (int i = 0; i < LenDataGprs; i++)
   {
      GSM_DPD(&g_aucOutDataFrameBuffer[i], 1);
   }
   DP_GSM("\r\n");

   uint8_t mask_ready = 0;
   if (socket_send(0, g_aucOutDataFrameBuffer, LenDataGprs, &mask_ready) == RET_GPRS_SEND_OK)
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_ACKNOW_DATA_911;
   }
   else
   {
      SetServerErr(GetServerErr() + 1);
      pGsmStatus->eGsmStep = GSM_OFF;
   }
}

/* Обработка полученного ответа от сервера Fm911 */
void stepGsmFindme911GprsAckData(TGsmStatus* pGsmStatus)
{
   DP_GSM("D_GSM STEP: stepGsmFindme911GprsAckData\r\n");
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;

   /* Если девайс меняет сервер, то возвращаем протокол и не парсим ответку */
   if (GetTypeConnectFm911() == TYPE_GOODBYE)
   {
      changeServerFm2iON();
   }
   else
   {
      /* Во всех режимах кроме TYPE_GOODBYE парсим ответ от сервера 911 */
      T_ANS_SRV_FM911 ans_srv = GprsAckDataFm911();   // получаем тип ответа от сервера 911.
      if (ans_srv != ANS_OK)
      {
         /* Если режим регистрации юзера и от сервера получен ответ ERROR, то удалим все СМС */
         if (GetTypeRegBase911() == TYPE_REG_USER && ANS_ERROR == ans_srv)
         {
            PDU_SMGD(6);   // удаление дефектной СМС если такая имелась.
            pGsmStatus->eGsmStep = SLEEP_DOWN;
            return;
         }
         pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_DEACTIVATE;
         return;
      }
   }

   /* Выходы девайса с 'нестандартными' выходами */
   /* Если девайс выходил с пониженной температурой или не получен ответ от сервера */
   if (GetTypeConnectFm911() == TYPE_ERR_COLD || GetTypeConnectFm911() == TYPE_ERR_CONSRV)
   {
      SetTypeRegBase911(TYPE_WU_START);
   }

   /* Все ОК, данные отправлены и получен ответ */
   DP_GSM("D_FM911 CLOSE CONNECT\r\n");
   profile_deactivate(PROF_FIRST_SERVER);
   SetCountReConnect(0);

   if (GetTypeRegBase911() == TYPE_REG_USER || GetTypeRegBase911() == TYPE_REG_TO_BASE)
   {
      pGsmStatus->eGsmStep = SLEEP_DOWN;
      return;
   }

   if (GetTypeConnectFm911() == TYPE_MOVE_START)
   {
      DP_GSM("D_FM WAIT ACCEL STOP\r\n");
      pGsmStatus->eGsmStep = GSM_SLEEP_WAIT_ACCEL_STOP;
      SetStatusReset(WAKE_UP_STOP);
      return;
   }
   pGsmStatus->eGsmStep = GSM_PROFILE_END_SMS;
}

/* Регистрация пользователя 911.
Крутимся в цикле и ждем СМС, отправляем ответку на СМС и
отправляем данные о пользователи на сервер 911 */
void stepGsmFindme911WaitRegUser(TGsmStatus* pGsmStatus)
{
   DP_GSM("D_GSM STEP: stepGsmFindme911WaitRegUser\r\n");
   static TUserConfigFm911 stUserConfigFm911 = { '\0', '\0' };

   pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;

   int res = 0;
   ledStatus(LOW_PWR1);
   static _Bool code = 0;   // кодировка сообщения (UCS2 - русские символы, LAT - латиница)

   DP_GSM("D_WAIT REGISTRATION USER SMS\r\n");

   // strcpy(stUserConfigFm911.strNameDevice, "Name1234"); strcpy(stUserConfigFm911.strNumUserTel, "79119008502"); Res =
   // 1;
   if ((!(strlen(stUserConfigFm911.strNameDevice))) &&
       (!(strlen(stUserConfigFm911.strNumUserTel))))   //Если не задан пользователь или пароль
   {
      int delay = time() + TIME_WAIT_SMS_REG_USER;
      while (delay > time())
      {
         SystemUpdateMonitor();   //Обновляем мониторинг зависание FreeRTOS
         DP_GSM("\r\nD_WAIT USER SMS: %d\r\n", (delay - time()));
         res = CheckSmsRegUserFm911(stUserConfigFm911.strNumUserTel, stUserConfigFm911.strNameDevice, &code);
         if (res)
         {
            break;   //Регистрация удалась
         }
      }
   }
   else
   {
      res = 1;   //Имя девайса и номер телефона уже были зареганы и сюда свалились по ошибки отправки данных на сервер
   }

   if (!(res))
   {
      /* Регистрация не удалась, далее просто уснем до следующего нажатие на кнопку */
      DP_GSM("D_FAILED TO REGISTER USER\r\n");
      DP_GSM("D_FM911 CLOSE CONNECT\r\n");
      profile_deactivate(PROF_FIRST_SERVER);
      DP_GSM("D_FM SLEEP DEVICE\r\n");
      SetCountReConnect(0);
      pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;
      pGsmStatus->eGsmStep = SLEEP_DOWN;
      return;
   }

   /* Регистрация удалась и получено Имя девайса и Номер пользователя.
   Сформируем и отправим данные на сервер */
   SetUserTel(stUserConfigFm911.strNumUserTel);   // сохраним телефон пользователя

   /* Open connect to server */
   char strAddrServ[SIZE_SERV] = { 0 };
   GetAddrSecondServ(strAddrServ);
   profile_activate(PROF_FIRST_SERVER, strAddrServ);

   uint8_t mask_ready = 0;
   int LenDataGprs = 0;
   uint8_t sum = 0;
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;
   int iRegStatus = 1;
   if (g_stRam.stSim.eRegStatus == ROAMING_NET)
   {
      iRegStatus = 5;
   }

   /* Формируем единый пакет для сервера 911 */
   DP_GSM("\r\nD_PACKET REG USER FINDME 911\r\n");
   //Формируем обертку пакета
   char strFmVersia[20];
   getNameFmVersia(strFmVersia);

   char strIMEI[SIZE_IMEI];
   GetStrIMEI(strIMEI);
   char strFIRST_SCID[SIZE_SCID] = { 0 };
   GetScidCurentFirstSim(strFIRST_SCID);

   sprintf(g_aucOutDataFrameBuffer,
           "$%s;%c;%s;%s,%i,%s;",
           strFmVersia,   // загружаем версию ПО
           TYPE_REG_USER,   // устанавливаем причину соединения
           strIMEI,   // загружаем IMEI GSM
           strFIRST_SCID,   // загружаем SCID SIM-карты
           iRegStatus,   // роуминг/не роуминг
           g_stRam.stSim.acMobCountCode   // код оператора мобильной связи;
   );
   LenDataGprs += strlen(g_aucOutDataFrameBuffer);

   /* Формируем запрос о регистрации на сервере с номером телефона и именем девайса */
   sprintf(g_asInpDataFrameBuffer,
           "T,\"+%s\";M,%d,\"%s\";",
           stUserConfigFm911.strNumUserTel,
           code,
           stUserConfigFm911.strNameDevice);

   strcat(g_aucOutDataFrameBuffer, g_asInpDataFrameBuffer);
   LenDataGprs += strlen(g_aucOutDataFrameBuffer);

   /* Колбек функций формирования пакетов FM911 */
   for (uint8_t count_packet = 0; count_packet < (sizeof(ptrGsmFindme911Scheduler) / sizeof(void*)); count_packet++)
   {
      memset(g_asInpDataFrameBuffer, 0, sizeof(g_asInpDataFrameBuffer));
      LenDataGprs += (*ptrGsmFindme911Scheduler[count_packet])(g_asInpDataFrameBuffer, 0); /* FM911 Scheduler */
      if (LenDataGprs >= (sizeof(g_aucOutDataFrameBuffer) - sizeof("*FF\r\n")))
      {
         DP_GSM("D_ERROR SIZE PACKET FINDME 911\r\n");
         break;
      }
      strcat(g_aucOutDataFrameBuffer, g_asInpDataFrameBuffer);
   }

   /* подсчет контрольной суммы */
   sum = gsm_cs(g_aucOutDataFrameBuffer, strlen(g_aucOutDataFrameBuffer));
   /* записываем контрольную сумму */
   sprintf(g_aucOutDataFrameBuffer + strlen(g_aucOutDataFrameBuffer), "*%02X\r\n", sum);
   LenDataGprs = strlen(g_aucOutDataFrameBuffer);

   DP_GSM("\r\nD_GPRS FM911 TX->:\r\n");
   for (int i = 0; i < LenDataGprs; i++)
   {
      GSM_DPD(&g_aucOutDataFrameBuffer[i], 1);
   }
   DP_GSM("\r\n");

   if (socket_send(0, g_aucOutDataFrameBuffer, LenDataGprs, &mask_ready) == RET_GPRS_SEND_OK)
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_ACKNOW_DATA_911;
   }
   else
   {
      SetServerErr(GetServerErr() + 1);
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_DEACTIVATE;
   }
}

/******************************************************************************/
