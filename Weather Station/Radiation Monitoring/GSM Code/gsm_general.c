#include "includes.h"
#include "gsm_general.h"
#include "flash_archive.h"

TGsmStatus stGsmStatus;

// Основной GSM процесс.
void vGsmTask(void* pvParameters)
{
   portTickType xLastWakeTimerDelay;
   stGsmStatus.eGsmStep = GSM_START;
   stGsmStatus.eNextAckGsmStep = NO_STEP;
   stGsmStatus.eCmdGsmStep = NO_STEP;
   stGsmStatus.uiGsmStepDelay = 0;

   while (1)
   {
      xLastWakeTimerDelay = xTaskGetTickCount();
      vTaskDelayUntil(&xLastWakeTimerDelay, (stGsmStatus.uiGsmStepDelay / portTICK_RATE_MS));

      switch (stGsmStatus.eGsmStep)
      {
      case GSM_OFF:
         StepGsmOff(&stGsmStatus);
         break;
      case GSM_OFF_LOW_PWR:
         StepGsmLowOff(&stGsmStatus);
         break;
      case GSM_START:
         StepGsmStart(&stGsmStatus);
         break;
      case GSM_SETTING_START:
         StepGsmSettingStart(&stGsmStatus);
         break;
      case GSM_CONFIG:
         StepGsmConfig(&stGsmStatus);
         break;
      case GSM_JAMMING_DETECTED:
         StepGsmJammingDetection(&stGsmStatus);
         break;
      case GSM_SIM:
         StepGsmSim(&stGsmStatus);
         break;
      case GSM_SIM_PIN:
         StepGsmSimPin(&stGsmStatus);
         break;
      case GSM_ADDITIONAL_CONFIG:
         StepGsmAdditionalConfig(&stGsmStatus);
         break;
      case GSM_CELLULAR_NETWORK:
         StepGsmCellularNetwork(&stGsmStatus);
         break;
      case GSM_CHECK_SMS:
         StepGsmCheckSMS(&stGsmStatus);
         break;   //Проверим СМС
      case GSM_PROFILE_GPRS_CONNECT:
         StepGsmProfileGprsConnect(&stGsmStatus);
         break;

      case GSM_PROFILE_GPRS_SOCKET:
         StepGsmProfileGprsSocket(&stGsmStatus);
         break;
      case GSM_PROFILE_GPRS_ACTIVATE:
         StepGsmProfileGprsActivate(&stGsmStatus);
         break;
      case GSM_PROFILE_GPRS_DEACTIVATE:
         StepGsmProfileGprsDeactivate(&stGsmStatus);
         break;
      case GSM_DEACTIVATE_CHECK_SMS_ACTIVATE:
         StepDeactivateCheckSmsActivate(&stGsmStatus);
         break;
      case GSM_PROFILE_GPRS_SEND_DATA_INIT:
         StepGsmProfileGprsSendDataInit(&stGsmStatus);
         break;
      case GSM_PROFILE_GPRS_ACKNOW_DATA_INIT:
         StepGsmProfileGprsAcknowDataInit(&stGsmStatus);
         break;
      case GSM_SWITCH_DATA:
         StepGsmSwitchData(&stGsmStatus);
         break;
      case GSM_PROFILE_GPRS_SEND_C_ACK:
         StepGsmSendAckC(&stGsmStatus);
         break;
      case CHECK_SMS:
         StepCheckSMS(&stGsmStatus);
         break;   //Проверим СМС

      /* TRACK ION */
      case WAIT_FLASH_DATA_READY:
         StepWaitFlashDataReady(&stGsmStatus);
         break;   //! ION FM

      case GSM_PROFILE_GPRS_ANS_CMD_SERVER_OK:
         StepGsmSendAnsOkData(&stGsmStatus);
         break;   //Отправляем OK серверу
      case GSM_PROFILE_GPRS_ANS_CMD_SERVER_ERR:
         StepGsmSendFailC(&stGsmStatus);
         break;   //Отправляем ERR серверу
      case GSM_PROFILE_GPRS_ACKNOW_DATA:
         StepGsmProfileGprsAcknowData(&stGsmStatus);
         break;

      /* FIND ME */
      case FIND_ME_DATA_READY:
         StepFindMeDataReady(&stGsmStatus);
         break;   //! FindMe

      case GSM_PROFILE_GPRS_FM_SEND_DATA_REALTIME:
         StepGsmProfileFindMeSendDataRealtime(&stGsmStatus);
         break;
      case GSM_PROFILE_GPRS_FM_SEND_DATA_TRACK:
         StepGsmProfileFindMeSendDataTrack(&stGsmStatus);
         break;
      case GSM_SLEEP_WAIT_GPS:
         StepGsmSleepWaitGps(&stGsmStatus);
         break;

      case GSM_PROFILE_GPRS_SEND_FIRMWARE_STATUS:
         StepGsmProfileGprsSendDataStatusFirmware(&stGsmStatus);
         break;
      case SYNCHRONIZATION_SERVER_TIME_REQUEST:
         StepSynchronizationServerTimeRequest(&stGsmStatus);
         break;
      case GSM_PROFILE_GPRS_SEND_DATA_STATUS_DEVICE:
         StepGsmProfileGprsSendDataStatusDevice(&stGsmStatus);
         break;
      case GSM_PROFILE_GPRS_SEND_DATA_ACCEL_STATUS:
         StepGsmProfileGprsSendDataAccelStatus(&stGsmStatus);
         break;
      case GSM_PROFILE_GPRS_SEND_INFO_DEVICE:
         StepGsmProfileGprsSendDataInfoDevice(&stGsmStatus);
         break;
      case GSM_PROFILE_GPRS_SEND_DATA:
         StepGsmProfileGprsSendData(&stGsmStatus);
         break;   //Отправляем данные либо трека либо реалтайма.
      case GSM_PROFILE_GPRS_SEND_GPIO_DEVICE:
         StepGsmProfileGprsSendDataGpioDevice(&stGsmStatus);
         break;

      /* UPDATE FIRMWARE */
      case GSM_PROFILE_HTTP_SOCKET:
         StepGsmProfileHttpSocket(&stGsmStatus);
         break;
      case GSM_PROFILE_HTTP_DOWNLOAD:
         StepGsmProfileHttpDownload(&stGsmStatus);
         break;
      case GSM_PROFILE_CHECK_FIRMWARE:
         StepGsmProfileCheckFirmware(&stGsmStatus);
         break;
         /********************************/

      case SLEEP_DOWN:
         StepSleep();
      case RESTART_NOW:
         StepRestartNow();
         break;
      }
   }
}

void StepGsmOff(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   g_stRam.stDevice.eLedStatus = LOAD_DEVICE;   //Загрузка устройства
   g_stRam.stSim.eRegStatus = SIM_WAIT;
   g_stRam.stSim.bGprsTrue = FALSE;
   g_stRam.stConnect.bGprsProfActivate = FALSE;
   g_stRam.stConnect.eConnectNumSer = NO_SERVER;
   g_stRam.stDevice.bSendInfoDevice = TRUE;

   if (g_stEepConfig.stDevice.eModeDevice == TRACK_ION)
   {
      if (g_stRam.stDevice.eCurPwrStat == POWER_RUN_MODE)
      {
         pGsmStatus->eGsmStep = GSM_START;
         pGsmStatus->uiGsmStepDelay = SLEEP_MS_2000;
         Sim1Pwr(OFF);
         GSM_State(OFF);
      }
      else
      {
         pGsmStatus->eGsmStep = GSM_OFF_LOW_PWR;
         pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;
      }
   }
   else
   {
      pGsmStatus->eGsmStep = GSM_START;
      Sim1Pwr(OFF);
      GSM_State(OFF);
      g_stRam.stDevice.ucCountGsmConnectFail++;
      if (g_stRam.stDevice.ucCountGsmConnectFail > MAX_GSM_CONNECT_FAIL)
      {
         g_stEepConfig.stDevice.ucCountReConnect++;
         DP_GSM("D_COUNT RECONNECT: %i\r\n", g_stEepConfig.stDevice.ucCountReConnect);
         if (g_stEepConfig.stDevice.ucCountReConnect >= MAX_GSM_CONNECT_FAIL)
         {
            g_stEepConfig.stDevice.ucCountReConnect = MAX_GSM_CONNECT_FAIL;
         }
         pGsmStatus->eGsmStep = SLEEP_DOWN;
         pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;
      }
   }
}

void StepGsmLowOff(TGsmStatus* pGsmStatus)
{
   PWR_STATUS BackPowerStatus;
   RTC_t DateRTC;

   uint32_t delay = 0;
   uint32_t SecRTC = 0;
   uint32_t delay_gsm_off = 0;   // время выключенного gsm модема в секундах
   stAccelAxisData stAccStateBack;
   TAcc_state stAcc_state;
   portTickType xLastWakeTimerDelay;
   Accel_Init();   // Init Accel
   Sim1Pwr(OFF);
   GSM_State(OFF);   //Выкл модем

   /* Получаем алгоритм работы устройства чтобы расчитать время склько будем спать */
   if (g_stEepConfig.stDevice.eModeDevice == TRACK_ION)
   {
      switch (g_stRam.stDevice.eCurPwrStat)
      {
      case POWER_RUN_MODE:
         GSM_DPD("PWR LOW STATUS: ERROR\r", strlen("PWR LOW STATUS: ERROR\r"));
         break;

      case POWER_LOW_PWR1_MODE:
         delay_gsm_off = GsmLowPwr1();
         break;

      case POWER_LOW_PWR2_MODE:
         delay_gsm_off = GsmLowPwr2();
         break;
      }
   }

   /*if(g_stEepConfig.stDevice.eModeDevice == ALARM_FIND) {
      delay_gsm_off = g_stEepConfig.stDevice.uiSleepTimeFind;
   }*/

   BackPowerStatus = g_stRam.stDevice.eCurPwrStat;
   g_stRam.stDevice.bSendInfoDevice = TRUE;

   if (delay_gsm_off)
   {
      rtc_gettime(&DateRTC);
      SecRTC = Date2Sec(&DateRTC);
      delay = SecRTC + delay_gsm_off;   //получаем конечное время пробуждения.

      ReadAxisDataAccel(&stAcc_state);
      stAccStateBack.X = stAcc_state.X;
      stAccStateBack.Y = stAcc_state.Y;
      stAccStateBack.Z = stAcc_state.Z;

#define DEF_ADD_SENS_ACCEL 0x0500
      uint16_t uiDelta = DEF_ADD_SENS_ACCEL + (g_stEepConfig.stAccel.ucSensitivity << 8);

      while (1)
      {
         IWDG_ReloadCounter();

         ReadAxisDataAccel(&stAcc_state);
         DP_GSM("D_AXIS%i", stAcc_state.X);
         DP_GSM(" %i", stAcc_state.Y);
         DP_GSM(" %i\r", stAcc_state.Z);

         if ((stAcc_state.X > stAccStateBack.X + uiDelta) || (stAcc_state.Y > stAccStateBack.Y + uiDelta) ||
             (stAcc_state.Z > stAccStateBack.Z + uiDelta) || (stAcc_state.X < stAccStateBack.X - uiDelta) ||
             (stAcc_state.Y < stAccStateBack.Y - uiDelta) || (stAcc_state.Z < stAccStateBack.Z - uiDelta))
         {
            if (g_stEepConfig.stDevice.eModeDevice == TRACK_ION)
            {
               g_stRam.stAccel.eAccelState = ACC_STATE_MOVE;
               break;
            }
         }

         //Выходим досрочно в случае изменения режима энергопотребления или если обновилась прошивка.
         if ((BackPowerStatus != g_stRam.stDevice.eCurPwrStat) && (g_stEepConfig.stDevice.eModeDevice == TRACK_ION))
         {
            if (g_stRam.stAccel.eAccelState == ACC_STATE_MOVE)
            {
               g_stRam.stAccel.eAccelState = AccelState();   //Получим статус Акселерометра.
            }
            break;
         }

         rtc_gettime(&DateRTC);
         SecRTC = Date2Sec(&DateRTC);
         if (delay <= SecRTC)
         {
            break;   //Проснитесь и пойте, мистер Фриман… Проснитесь и пойте…
         }

         if (g_stRam.stDevice.eCurPwrStat == POWER_LOW_PWR1_MODE)
         {
            DP_GSM("D_LOW PWR1: ");
         }
         else
         {
            if (g_stRam.stDevice.eCurPwrStat == POWER_LOW_PWR2_MODE)
            {
               DP_GSM("D_LOW PWR2: ");
            }
            else
            {
               DP_GSM("D_ERROR:");
            }
         }
         DP_GSM("%d sec\r", delay - SecRTC);

         xLastWakeTimerDelay = xTaskGetTickCount();
         vTaskDelayUntil(&xLastWakeTimerDelay, (SLEEP_MS_1000 / portTICK_RATE_MS));
      }
      IWDG_ReloadCounter();
   }

   pGsmStatus->uiGsmStepDelay = SLEEP_MS_10;
   pGsmStatus->eGsmStep = GSM_START;
}

void StepGsmStart(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   g_stRam.stDevice.eLedStatus = LOAD_DEVICE;   //Загрузка устройства

   RET_INFO eStatusRet = (RET_INFO)(GSM_State(ON));
   if (eStatusRet != RET_OK)
   {
      pGsmStatus->uiGsmStepDelay = SLEEP_MS_1000;
      pGsmStatus->eGsmStep = GSM_OFF;
      return;
   }

   osDelay(SLEEP_MS_2000);
   eStatusRet = modem_on();
   if (eStatusRet != RET_OK)
   {
      pGsmStatus->uiGsmStepDelay = SLEEP_MS_1000;
      pGsmStatus->eGsmStep = GSM_SETTING_START;
      return;
   }

   pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;
   pGsmStatus->eGsmStep = GSM_CONFIG;
}

void StepGsmSettingStart(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   RET_INFO eStatusRet = modem_setting_start();
   if (eStatusRet == RET_OK)
   {
      pGsmStatus->uiGsmStepDelay = SLEEP_MS_5000;
      pGsmStatus->eGsmStep = GSM_CONFIG;
   }
   else
   {
      pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;
      pGsmStatus->eGsmStep = GSM_OFF;
   }
}

void StepGsmConfig(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   RET_INFO eStatusRet = modem_precmd();
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;
   if (eStatusRet == RET_OK)
   {
      if (g_stEepConfig.stSim.eJamDetect != NO)
      {
         pGsmStatus->eGsmStep = GSM_JAMMING_DETECTED;
      }
      else
      {
         pGsmStatus->eGsmStep = GSM_SIM;
      }
   }
   else
   {
      pGsmStatus->eGsmStep = GSM_OFF;
   }
}

void StepGsmJammingDetection(TGsmStatus* pGsmStatus)
{
   RET_INFO eStatusRet;
   GSM_INFO stGsm;
   memset(&stGsm, 0, sizeof(GSM_INFO));
   INIT_GSM_INFO(stGsm);

   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;
   DP_GSM("D_FIND GSM LBS, PLEASE WAIT\r\n");
   //Проверим LLS, что бы было не менее одной баз. станции GSM.
   get_opInfo(AT_SMOND, &stGsm, 10);   //"at^smond"
   DP_GSM("D_JMP LBS COUNT: %d\r\n", stGsm.count);
   if (stGsm.count > 1)
   {
      pGsmStatus->eGsmStep = GSM_SIM;
   }
   else
   {
      pGsmStatus->eGsmStep = SLEEP_DOWN;   //Засыпаем если не нашли станций GSM.
   }

   /* Задействона функция gsm модема определения глушения */
   if (g_stEepConfig.stSim.eJamDetect == JAM)
   {
      Sim1Pwr(ON);
      eStatusRet =
         modem_jamming_detected(g_stEepConfig.stSim.ucTimeFindJammingDetect);   //Функция определения глущения сигнала.
      if (eStatusRet == RET_JD_OK)
      {   //Если нас глушат засыпаем
         pGsmStatus->eGsmStep = SLEEP_DOWN;
      }
      else
      {
         pGsmStatus->eGsmStep = GSM_SIM;
      }

      if ((eStatusRet == ERR_GPRS_TIMEOUT) || (eStatusRet == ERR_ABORTED))
      {   //Если не получилось определить причину
         DP_GSM("D_JAM DET ERROR\r\n");
         pGsmStatus->eGsmStep = GSM_OFF;
      }
   }
}

void StepGsmSim(TGsmStatus* pGsmStatus)
{
   Sim1Pwr(ON);

   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;

   g_stRam.stDevice.eLedStatus = FIND_SIM;   //Поиск SIM-карты
   M_INFO eSimStatus = SimCardInit();
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;
   if (eSimStatus == M_SIM_READY)
   {
      pGsmStatus->eGsmStep = GSM_ADDITIONAL_CONFIG;
   }
   else
   {
      if (eSimStatus == M_OK)   // No SIM Card
      {
         pGsmStatus->uiGsmStepDelay = SLEEP_MS_2000;
         g_stRam.stSim.eRegStatus = SIM_NO_READY;
         pGsmStatus->eGsmStep = GSM_OFF;
      }

      if (eSimStatus == M_SIM_PIN)   // SIM card PIN code
      {
         g_stRam.stSim.eRegStatus = SIM_PIN;
         pGsmStatus->eGsmStep = GSM_SIM_PIN;
      }

      if (eSimStatus == M_SIM_PUK)   // SIM card PUK code
      {
         g_stRam.stSim.eRegStatus = SIM_PUK;
         if (g_stEepConfig.stDevice.eModeDevice == TRACK_ION)
         {
            pGsmStatus->eGsmStep = GSM_OFF;
         }
         else
         {
            pGsmStatus->eGsmStep = SLEEP_DOWN;
         }
      }

      if (eSimStatus == M_OTHER)
      {
         g_stRam.stSim.eRegStatus = SIM_ERROR;
         if (g_stEepConfig.stDevice.eModeDevice == TRACK_ION)
         {
            pGsmStatus->eGsmStep = GSM_OFF;
         }
         else
         {
            pGsmStatus->eGsmStep = GSM_OFF;
         }
      }
   }
}

void StepGsmSimPin(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   M_INFO eSimStatus = SimCardPin();
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;
   if (eSimStatus == M_OK)
   {
      pGsmStatus->eGsmStep = GSM_SIM;
      return;
   }
   else
   {
      if (eSimStatus == M_SIM_PUK)
      {
         g_stRam.stSim.eRegStatus = SIM_PIN_GUESSING;
      }
      else
      {
         g_stRam.stSim.eRegStatus = SIM_PIN_NO_SET;
      }
      pGsmStatus->eGsmStep = GSM_OFF;
   }
}

void StepGsmAdditionalConfig(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   RET_INFO eStatusRet = modem_additional_precmd();
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_1000;

   if ((!(GPIO_ReadInputDataBit(GPS_REF_PORT, GPS_REF_PIN))) == OFF)
   {   //Проверяем и включаем GPS
      xSemaphoreGive(mINIT_GPS_MODULE);
   }

   if (g_stEepConfig.stDevice.eModeDevice == TRACK_ION)
   {
      g_stRam.stSim.eRegStatus = FIND_NET;
      pGsmStatus->eGsmStep = GSM_CELLULAR_NETWORK;
   }

   if (g_stEepConfig.stDevice.eModeDevice == STANDART)
   {
      // Смотрим пролов GPS и повторный вход. Если да, то передаем данные, а если нет, то выключаем GSM модем и ждем
      // пролова GPS.
      if (GetGpsStatus() || (g_stRam.stConnect.bWaitGpsModeFm))
      {
         g_stRam.stSim.eRegStatus = FIND_NET;   //Передаем данные.
         pGsmStatus->eGsmStep = GSM_CELLULAR_NETWORK;
      }
      else
      {
         pGsmStatus->eGsmStep = GSM_SLEEP_WAIT_GPS;   //Ждем пролова GPS.
      }

      /* Обработчик событий невыхода на связь по расписанию */
      if (g_stRam.stDevice.ucCountGsmConnectFail >= MAX_GSM_CONNECT_FAIL)
      {
         g_stEepConfig.stDevice.ucCountReConnect++;
         DP_GSM("D_COUNT RECONNECT: %i\r\n", g_stEepConfig.stDevice.ucCountReConnect);
         if (g_stEepConfig.stDevice.ucCountReConnect >= MAX_GSM_CONNECT_FAIL)
         {
            g_stEepConfig.stDevice.ucCountReConnect = MAX_GSM_CONNECT_FAIL;
         }

         pGsmStatus->eGsmStep = SLEEP_DOWN;
         pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;
      }
      /***************************************************/
   }
   else
   {
      g_stRam.stDevice.ucCountGsmConnectFail = 0;
   }
}

void StepGsmCellularNetwork(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   g_stRam.stDevice.eLedStatus = REG_GSM;   //ожидание подключения к GSM сети
   RET_INFO eStatusRet = GetCellularNetwork();
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_3000;
   if (eStatusRet == RET_HOME_NET)   // Если домашняя сеть
   {
      g_stRam.stSim.eRegStatus = HOME_NET;
      pGsmStatus->eGsmStep = GSM_CHECK_SMS;
      return;
   }

   if (eStatusRet == RET_ROAMING_NET)   // Если роуминг сеть
   {
      g_stRam.stSim.eRegStatus = ROAMING_NET;
      // передача данных в роуминге
      if (g_stEepConfig.stGsm.bRoamingGprsEnable == ON)
      {   //передача данных разрещена.
         pGsmStatus->eGsmStep = GSM_CHECK_SMS;
      }
   }

   if (g_stEepConfig.stDevice.eModeDevice == TRACK_ION)
   {
      pGsmStatus->eGsmStep = GSM_OFF;
   }
   else
   {
      pGsmStatus->eGsmStep = SLEEP_DOWN;
   }
}

void StepGsmCheckSMS(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   CheckSmsCommand();
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_1000;
   pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_CONNECT;
}

void StepGsmProfileGprsConnect(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   RET_INFO eStatusRet = profile_gprs_connect(0);
   g_stRam.stDevice.eLedStatus = SERVER_CONNECT;   //ожидание подключения к серверу

   pGsmStatus->uiGsmStepDelay = SLEEP_MS_1000;
   if (eStatusRet == RET_GPRS_OK)
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_SOCKET;
   }
   else
   {
      DPS("D_ERR: SIM not find apn!");
      pGsmStatus->eGsmStep = GSM_OFF;
   }
}

void StepGsmProfileGprsSocket(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   RET_INFO eStatusRet = ProfileSocketInit(g_stEepConfig.stConnect.str_name_first_server, 0);
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;
   if (eStatusRet == RET_OK)
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_ACTIVATE;
   }
   else
   {
      pGsmStatus->eGsmStep = GSM_OFF;
   }
}

void StepGsmProfileGprsActivate(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   RET_INFO eStatusRet = profile_activate(PROF_FIRST_SERVER, g_stEepConfig.stConnect.str_name_first_server);
   g_stRam.stDevice.eLedStatus = NORMAL_RUN;   //Регистрация в сети GSM пройдена, устройство подключено к серверу
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;
   if (eStatusRet == RET_GPRS_OK)
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_SEND_DATA_INIT;
   }
   else
   {
      pGsmStatus->eGsmStep = GSM_OFF;
   }
}

void StepGsmProfileGprsDeactivate(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   RET_INFO eStatusRet = profile_deactivate(PROF_FIRST_SERVER);
   g_stRam.stDevice.eLedStatus = SERVER_CONNECT;   //ожидание подключения к серверу
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;
   if ((eStatusRet == RET_OK) && (g_stRam.stDevice.eCurPwrStat == POWER_RUN_MODE))
   {
      pGsmStatus->eGsmStep = GSM_DEACTIVATE_CHECK_SMS_ACTIVATE;
   }

   if ((eStatusRet == RET_OK) && (g_stRam.stDevice.eCurPwrStat != POWER_RUN_MODE))
   {
      pGsmStatus->eGsmStep = GSM_OFF_LOW_PWR;
   }

   if (eStatusRet != RET_OK)
   {
      pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;
      pGsmStatus->eGsmStep = GSM_OFF;
   }

   if (pGsmStatus->eCmdGsmStep != NO_STEP)
   {
      pGsmStatus->eGsmStep = pGsmStatus->eCmdGsmStep;
      pGsmStatus->eCmdGsmStep = NO_STEP;
   }
}

void StepDeactivateCheckSmsActivate(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;
   CheckSmsCommand();
   GSM_INFO out_check;
   check_csq(&out_check);
   check_creg(&out_check);
   if ((out_check.msg[1].var + 0x30) == RET_ROAMING_NET)
   {
      g_stRam.stSim.eRegStatus = ROAMING_NET;
      // Разрешить передачу данных в роуминге
      if (g_stEepConfig.stGsm.bRoamingGprsEnable == OFF)
      {
         pGsmStatus->eGsmStep = GSM_OFF;
         return;
      }
   }
   pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_ACTIVATE;
}

void StepGsmProfileGprsSendDataInit(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   int LenDataInitPacket = 0;
   uint8_t mask_ready = 0;
   g_stFrame.usDevicePaketNumber = 0;

   GSM_DPD("D_Packet INIT\r", strlen("D_Packet INIT\r"));

   g_stFrame.ucType = C_CONNECT;
   g_stFrame.ulSerialNumber = g_stRam.stGsm.ucIMEI;
   g_stFrame.usDevicePaketNumber++;

   LenDataInitPacket = frame_build_paket(&g_stFrame, InpDataBuffer, 0, g_asOutDataFrameBuffer);

   if (socket_send(0, g_asOutDataFrameBuffer, LenDataInitPacket, &mask_ready) == RET_GPRS_SEND_OK)
   {
      pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_ACKNOW_DATA_INIT;
   }
   else
   {
      pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_DEACTIVATE;
   }
}

void StepGsmProfileGprsAcknowDataInit(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   GSM_STEP eRet = GprsAckData();
   if (eRet == NO_STEP)
   {
      g_stRam.stConnect.eConnectNumSer = g_stEepConfig.stConnect.eUseNumSer;
      pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;
      pGsmStatus->eGsmStep = GSM_SWITCH_DATA;
   }
   else
   {
      pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_DEACTIVATE;
   }
}

void StepGsmSwitchData(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;

   if (g_stFrame.usFlagsRetDataServer & (fS_FAIL))
   {
      g_stFrame.usFlagsRetDataServer &= ~fS_FAIL;
      // pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_SEND_C_FAIL;
      pGsmStatus->uiGsmStepDelay = SLEEP_MS_10;
      GSM_DPD("D_SERVER ERR: S_FAIL\r", strlen("D_SERVER ERR: S_FAIL\r"));
   }

   if (g_stFrame.usFlagsRetDataServer & (fC_FAIL))
   {
      g_stFrame.usFlagsRetDataServer &= ~fC_FAIL;

      if (g_stFrame.eTypeInfoPack == NO_STEP)
      {
         Flash_DataSendOK();
      }
      pGsmStatus->eGsmStep = CHECK_SMS;
      pGsmStatus->uiGsmStepDelay = SLEEP_MS_10;
      GSM_DPD("D_SERVER ERR: C_FAIL\r", strlen("D_SERVER ERR: C_FAIL\r"));
   }

   if (g_stFrame.usFlagsRetDataServer & (fS_ACK))
   {
      g_stFrame.usFlagsRetDataServer &= ~fS_ACK;
      if (g_stFrame.usServerPaketNumber == g_stFrame.usDeviceDataPaketNumber)
      {
         if (g_stFrame.eTypeInfoPack == NO_STEP)
         {
            Flash_DataSendOK();
         }
      }

      pGsmStatus->eGsmStep = CHECK_SMS;
      pGsmStatus->uiGsmStepDelay = SLEEP_MS_10;
      GSM_DPD("D_SERVER ANS: S_ACK\r", strlen("D_SERVER ANS: S_ACK\r"));
   }

   if (g_stFrame.usFlagsRetDataServer & (fS_ACK_DLY | fS_FIN))
   {
      if (g_stFrame.usFlagsRetDataServer & (fS_ACK_DLY))
      {
         g_stFrame.usFlagsRetDataServer &= ~fS_ACK_DLY;
         pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_DEACTIVATE;   //Сделаем переконнект на рандомное время
         GSM_DPD("D_SERVER ANS: S_ACK_DLY\r", strlen("D_SERVER ANS: S_ACK_DLY\r"));
      }
      else
      {
         g_stFrame.usFlagsRetDataServer &= ~fS_FIN;

         pGsmStatus->uiGsmStepDelay = GSM_PROFILE_GPRS_DEACTIVATE;   //Сделаем переконнект на рандомное время
         GSM_DPD("D_SERVER ANS: S_FIN\r", strlen("D_SERVER ANS: S_FIN\r"));
      }

      pGsmStatus->uiGsmStepDelay = SLEEP_MS_10;
   }

   // Data //
   if (g_stFrame.usFlagsRetDataServer & (fS_ASK_DATA | S_DATA))
   {
      g_stFrame.usFlagsRetDataServer &= ~fS_ASK_DATA;
      g_stFrame.usFlagsRetDataServer &= ~S_DATA;
      g_stFrame.usServerDataPaketNumber = g_stFrame.usServerPaketNumber;

      if ((g_stFrame.usServerPaketNumber == g_stFrame.usDeviceDataPaketNumber) && (g_stFrame.eTypeInfoPack == NO_STEP))
      {
         Flash_DataSendOK();
      }

      // Обрабатываем данные от сервера.
      pGsmStatus->eNextAckGsmStep = ParsingData();

      if (g_stFrame.usServerPaketNumber == g_stFrame.usDeviceDataPaketNumber)
      {
         if (g_stFrame.eTypeInfoPack == NO_STEP)
         {
            Flash_DataSendOK();
         }
      }

      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_SEND_C_ACK;   //
      pGsmStatus->uiGsmStepDelay = SLEEP_MS_10;
      GSM_DPD("D_SERVER ANS: S_ASK_DATA\r", strlen("D_SERVER ANS: S_ASK_DATA\r"));
   }

   if (pGsmStatus->uiGsmStepDelay == NO_STEP)
   {
      if (g_stFrame.eTypeInfoPack == NO_STEP)
      {
         Flash_DataSendOK();
      }
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_DEACTIVATE;
      DP_GSM("D_SERVER ANS: %i\r", g_stFrame.usFlagsRetDataServer);
   }
}

//Отправляем серверу потверждение C_ACK.
void StepGsmSendAckC(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;
   RET_INFO eStatusRet = GprsSendAckC();
   if (eStatusRet == RET_OK)
   {
      pGsmStatus->eGsmStep = pGsmStatus->eNextAckGsmStep;
      pGsmStatus->eNextAckGsmStep = GSM_OFF;
   }
   else
   {
      pGsmStatus->eGsmStep = pGsmStatus->eNextAckGsmStep;
      pGsmStatus->eNextAckGsmStep = GSM_OFF;
   }
}

void StepCheckSMS(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;
   CheckSmsCommand();
   GSM_INFO out_check;
   check_csq(&out_check);
   check_creg(&out_check);
#ifdef TEMPERATURE_GSM
   if (gsm_getTemperature(&out_check) == -1)
   {
      out_check.s8Temperatur = -128;
   }
#endif
   if ((out_check.msg[1].var + 0x30) == RET_ROAMING_NET)
   {
      g_stRam.stSim.eRegStatus = ROAMING_NET;
      // Разрешить передачу данных в роуминге
      if (g_stEepConfig.stGsm.bRoamingGprsEnable == OFF)
      {
         pGsmStatus->eGsmStep = GSM_OFF;
         return;
      }
   }

   // Выбираем алгоритм дальнейшей работы.
   if (g_stEepConfig.stDevice.eModeDevice == TRACK_ION)
   {
      pGsmStatus->eGsmStep = WAIT_FLASH_DATA_READY;
   }
   else
   {
      pGsmStatus->eGsmStep = FIND_ME_DATA_READY;
   }
}

/* Шаг на котором определяем что будем передавать серверу в режиме iON */
void StepWaitFlashDataReady(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   static ACC_STATE eBackAccelStopEvent = (ACC_STATE)0xFF;
   static int iReConnectCount = 0;

   //Далее располагаем в порядке приоритета, то что будем передавать:

   //Пакет инфы о новой и текущей прошивки.
   if (g_stRam.stFirmware.g_bNewFirmware == TRUE)
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_SEND_FIRMWARE_STATUS;
      iReConnectCount = 0;
      return;
   }

   //Если сбилось время то синхронизируем его.
   if (!(SerSinchroTime()))
   {
      pGsmStatus->eGsmStep = SYNCHRONIZATION_SERVER_TIME_REQUEST;
      iReConnectCount = 0;
      return;
   }

   //Если изменился статус девайса.
   if (ChangeStatusDevice())   //Если менялся, то отправим его статус.
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_SEND_DATA_STATUS_DEVICE;
      iReConnectCount = 0;
      return;
   }

   if (g_stRam.stDevice.bSendInfoDevice == TRUE)
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_SEND_INFO_DEVICE;
      iReConnectCount = 0;
      return;
   }

   //Если произошло событие остановки/движения девайса.
   ACC_STATE eAccelStopEvent = AccelState();
   if (eAccelStopEvent != eBackAccelStopEvent)
   {
      eBackAccelStopEvent = eAccelStopEvent;
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_SEND_DATA_ACCEL_STATUS;
      iReConnectCount = 0;
      return;
   }

   //Если нет пакета реалтайма, нет данных во флешки то переходим в спящий режим.
   if ((((g_stRam.stFlash.bRealtimeReady == FALSE) && (g_stRam.stFlash.bFlashReady == FALSE))) ||
       (g_stRam.stDevice.eCurPwrStat == POWER_SLEEP_MODE) || (g_stRam.stDevice.eCurPwrStat == POWER_LOW_PWR2_MODE))
   {
      switch (g_stRam.stDevice.eCurPwrStat)
      {
      case POWER_RUN_MODE:
         break;
         // pGsmStatus->eGsmStep = CHECK_SMS;
         // iReConnectCount = 0;
         // return;

      case POWER_LOW_PWR1_MODE:
         pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_DEACTIVATE;
         iReConnectCount = 0;
         return;

      case POWER_LOW_PWR2_MODE:
         pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_DEACTIVATE;
         iReConnectCount = 0;
         return;

      case POWER_SLEEP_MODE:
         iReConnectCount = 0;
         return;
      }
   }

   //Если есть данные.
   if ((g_stRam.stFlash.bFlashReady == TRUE) || (g_stRam.stFlash.bRealtimeReady == TRUE))
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_SEND_DATA;
      iReConnectCount = 0;
      return;
   }

   pGsmStatus->eGsmStep = CHECK_SMS;
#define TIME_RECONNECT_SERVER 60
   iReConnectCount++;
   if (iReConnectCount > TIME_RECONNECT_SERVER)
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_DEACTIVATE;
      iReConnectCount = 0;
   }
   DP_GSM("D_TIME OPEN SOCKET: %i\r", iReConnectCount);
}

void StepGsmSleepWaitGps(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;

   // фиксим, что мы уже заходили, чтобы передавать данные, а не искать GPS
   g_stRam.stConnect.bWaitGpsModeFm = 1;
   uint32_t delay = g_stEepConfig.stDevice.uiGpsWait;

   portTickType xLastWakeTimerDelay;
   Sim1Pwr(OFF);
   GSM_State(OFF);   //Выкл модем
   DP_GSM("D_FIND GPS, PLEASE WAIT\r\n");
   while (delay)
   {
      DP_GSM("D_WAIT GPS: %i\r\n", delay);
      if (g_stGpsData.time)
      {
         DP_GSM("D_GPS TIME VALID\r\n");
      }
      if (GetGpsStatus())
      {
         break;   //Выходим если нашли спутники.
      }

      delay--;
      xLastWakeTimerDelay = xTaskGetTickCount();
      vTaskDelayUntil(&xLastWakeTimerDelay, (SLEEP_MS_1000 / portTICK_RATE_MS));
   }
   g_stRam.stGps.usTimeGpsFix = delay;

   pGsmStatus->uiGsmStepDelay = SLEEP_MS_10;
   pGsmStatus->eGsmStep = GSM_START;
}

/* Шаг на котором определяем что будем передавать серверу в режиме FindME */
void StepFindMeDataReady(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;
   static uint8_t ucStepCount = 0;

   //Пакет инфы о новой и текущей прошивки.
   if (g_stRam.stFirmware.g_bNewFirmware == TRUE)
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_SEND_FIRMWARE_STATUS;
      ucStepCount = 0;
      return;
   }

   ucStepCount++;
   switch (ucStepCount)
   {
   case 1:
      g_stRam.stDevice.eCurPwrStat = POWER_RUN_MODE;
      if (g_stRam.stDevice.eResetStatusDevice == WAKE_UP_ACCEL)
      {
         g_stRam.stAccel.eAccelState = ACC_STATE_MOVE;
      }
      if (!(SerSinchroTime()))
      {
         DP_GSM("D_FM SYNCHRON TIME\r\n");
         pGsmStatus->eGsmStep = SYNCHRONIZATION_SERVER_TIME_REQUEST;
         break;
      }
      else
      {
         ucStepCount++;
      }

   case 2:
      DP_GSM("D_FM STATUS\r\n");
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_SEND_DATA_STATUS_DEVICE;
      break;

   case 3:
      DP_GSM("D_FM REALTIME\r\n");
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_FM_SEND_DATA_REALTIME;
      break;

   case 4:
      DP_GSM("D_FM GPIO\r\n");
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_SEND_GPIO_DEVICE;
      break;

   case 5:
      if (g_stRam.stFlash.bFlashReady == FALSE)
      {
         if (g_stEepConfig.stDevice.eModeDevice == STANDART)
         {
            ArchiveErase();
         }
         ucStepCount++;
      }
      else
      {
         DP_GSM("D_FM TRACK\r\n");
         pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_FM_SEND_DATA_TRACK;
         ucStepCount = 4;
         break;
      }

   // Передаем энергорежим девайса перед уходом в сон
   case 7:
      DP_GSM("D_FM PWR&ACCEL\r\n");
      if (g_stEepConfig.stDevice.eModeDevice == ACCEL_FIND)
      {
         if (AccelState() == ACC_STATE_STOP)
         {   //Получим статус Акселерометра.
            g_stRam.stDevice.eCurPwrStat = POWER_LOW_PWR2_MODE;
            DP_GSM("D_FM MODE ACCEL STOP\r\n");
         }
         else
         {
            g_stRam.stDevice.eCurPwrStat = POWER_RUN_MODE;
            ucStepCount = 1;
            DP_GSM("D_FM MODE ACCEL MOVE\r\n");
         }
      }
      else
      {
         g_stRam.stDevice.eCurPwrStat = POWER_LOW_PWR2_MODE;
      }

      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_SEND_DATA_ACCEL_STATUS;
      break;

   case 8:
      DP_GSM("D_FM CLOSE CONNECT\r\n");
      profile_deactivate(PROF_FIRST_SERVER);
      ucStepCount++;

   case 9:
      if (g_stEepConfig.stDevice.eModeDevice == STANDART)
      {
         DP_GSM("D_FM MODE STANDART\r\n");
         g_stEepConfig.stDevice.ucCountReConnect = 0;
         pGsmStatus->eGsmStep = SLEEP_DOWN;
         ucStepCount = 0;
      }
      /*if(g_stEepConfig.stDevice.eModeDevice == ALARM_FIND) {
          DP_GSM("D_FM MODE ALARM FIND\r\n");
          pGsmStatus->eGsmStep = GSM_OFF_LOW_PWR;
          ucStepCount = 0;
      }*/
      if (g_stEepConfig.stDevice.eModeDevice == ACCEL_FIND)
      {
         DP_GSM("D_FM MODE ACCEL FIND\r\n");
         pGsmStatus->eGsmStep = SLEEP_DOWN;
         ucStepCount = 0;
      }
      break;
   }
}

void StepGsmProfileFindMeSendDataRealtime(TGsmStatus* pGsmStatus)
{
   uint8_t mask_ready = 0;
   int Len, LenDataGprs;
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_1000;
   memset(g_asOutDataFrameBuffer, 0, sizeof(g_asOutDataFrameBuffer));
   GSM_DPD("\r\nD_Packet Realtime FM: ", strlen("\r\nD_Packet Realtime FM: "));

   if (GetGpsStatus())
   {
      GSM_DPD("D_GPS\r", strlen("D_GPS\r"));
      Len = GetDataNavigationalGpsPacket(InpDataBuffer, NAVIGATIONAL_PACKET_REAL_TIME, 0);
   }
   else
   {
      GSM_DPD("D_LBS\r\n", strlen("D_LBS\r\n"));
      Len = 0;
      // memset(InpDataBuffer, 0, sizeof(InpDataBuffer));
      //Добавляем к пакету реалтайма LBS еще и навигационный пакет GPS с невалидными координатами
      Len += frame_build_navigational_not_valid_packet(&g_stGpsDataBack, InpDataBuffer, Len);
      Len += GetDataNavigationalGsmPacket(InpDataBuffer, Len);
   }
   g_stFrame.ucType = C_DATA;
   g_stFrame.ulSerialNumber = g_stRam.stGsm.ucIMEI;
   g_stFrame.usDevicePaketNumber++;
   LenDataGprs = frame_build_paket(&g_stFrame, InpDataBuffer, Len, g_asOutDataFrameBuffer);

   if (LenDataGprs == NULL)
   {
      pGsmStatus->eGsmStep = CHECK_SMS;
      return;
   }

   if (socket_send(0, g_asOutDataFrameBuffer, LenDataGprs, &mask_ready) == RET_GPRS_SEND_OK)
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_ACKNOW_DATA;
   }
   else
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_DEACTIVATE;
   }
}

void StepGsmProfileFindMeSendDataTrack(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   uint8_t mask_ready = 0;
   int LenDataGprs;
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_1000;
   memset(g_asOutDataFrameBuffer, 0, sizeof(g_asOutDataFrameBuffer));
   g_stRam.stFlash.bFlashReady = FALSE;

   DP_GSM("D_Packet Track FM\r\n");
   memset(g_asOutDataFrameBuffer, 0, sizeof(g_asOutDataFrameBuffer));
   u32 uiFlashLen = Flash_DataLen();
   DP_GSM("D_Flash Len Data: %i\r\n", uiFlashLen);
   if (uiFlashLen > SIZE_FLASH_BUF)
   {
      uiFlashLen = SIZE_FLASH_BUF;
   }
   LenDataGprs = GetDataToSend(g_asOutDataFrameBuffer, uiFlashLen);
   if (LenDataGprs == NULL)
   {
      DP_GSM("D_Flash Len Data: NULL\r\n");
      pGsmStatus->eGsmStep = CHECK_SMS;
      return;
   }

   if (socket_send(0, g_asOutDataFrameBuffer, LenDataGprs, &mask_ready) == RET_GPRS_SEND_OK)
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_ACKNOW_DATA;
   }
   else
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_DEACTIVATE;
   }
}

void StepGsmSendAnsOkData(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   RET_INFO eStatusRet = GprsSendAnsOkData();
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;
   if (eStatusRet == RET_OK)
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_DEACTIVATE;
   }
   else
   {
      pGsmStatus->eGsmStep = GSM_OFF;
   }
}

void StepGsmSendFailC(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   RET_INFO eStatusRet = GprsSendFailC();
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;
   if (eStatusRet == RET_OK)
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_DEACTIVATE;
   }
   else
   {
      pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;
      pGsmStatus->eGsmStep = GSM_OFF;
   }
}

void StepGsmProfileGprsAcknowData(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;
   GSM_STEP eRet = GprsAckData();
   if (eRet == NO_STEP)
   {
      g_stRam.stConnect.eConnectNumSer = g_stEepConfig.stConnect.eUseNumSer;
      pGsmStatus->eGsmStep = GSM_SWITCH_DATA;
   }
   else
   {
      pGsmStatus->eGsmStep = eRet;
   }
}

void StepGsmProfileGprsSendDataStatusFirmware(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   int LenDataInitPacket = 0;
   int iLen = 0;
   uint8_t mask_ready = 0;
   g_stFrame.usDevicePaketNumber = 0;

   pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;
   g_stFrame.ucType = C_DATA;
   g_stFrame.ulSerialNumber = g_stRam.stGsm.ucIMEI;
   g_stFrame.usDevicePaketNumber++;

   iLen = frame_build_received_firmware(InpDataBuffer);   // Формируем пакет инфы о прошивке.

   LenDataInitPacket = frame_build_paket(&g_stFrame, InpDataBuffer, iLen, g_asOutDataFrameBuffer);
   GSM_DPD("D_Packet FIRMWARE:\r", strlen("D_Packet FIRMWARE:\r"));

   DP_GSM("D_CUR: %d\r", g_stRam.stFirmware.uiNameCurrentFirmware);
   DP_GSM("D_NEW FLS: %d\r", g_stRam.stFirmware.uiNameNewFirmware);
   DP_GSM("D_NEW EEP: %d\r", g_stEepConfig.stFirmware.uiNameNewFirmware);

   if (socket_send(0, g_asOutDataFrameBuffer, LenDataInitPacket, &mask_ready) == RET_GPRS_SEND_OK)
   {
      g_stRam.stFirmware.g_bNewFirmware = FALSE;
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_ACKNOW_DATA;
   }
   else
   {
      pGsmStatus->eGsmStep = GSM_OFF;
   }
}

void StepSynchronizationServerTimeRequest(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   uint16_t Len;
   int LenDataGprs = 0;
   RTC_t DateRTC;
   uint32_t SecRTC;
   uint8_t mask_ready = 0;
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;
   rtc_gettime(&DateRTC);
   SecRTC = Date2Sec(&DateRTC);

   Len = ServerSynchroTime(SecRTC, InpDataBuffer, 0);

   g_stFrame.ucType = C_DATA;
   g_stFrame.ulSerialNumber = g_stRam.stGsm.ucIMEI;
   g_stFrame.usDevicePaketNumber++;
   LenDataGprs = frame_build_paket(&g_stFrame, InpDataBuffer, Len, g_asOutDataFrameBuffer);
   DP_GSM("\r\nD_Packet SINCHRO TIME\r\n");

   if (socket_send(0, g_asOutDataFrameBuffer, LenDataGprs, &mask_ready) == RET_GPRS_SEND_OK)
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_ACKNOW_DATA;
   }
   else
   {
      pGsmStatus->eGsmStep = GSM_OFF;
   }
}

void StepGsmProfileGprsSendDataStatusDevice(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   uint8_t mask_ready = 0;
   int LenDataGprs = 0;
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;
   DP_GSM("\r\nD_Packet STATUS\r\n");

   //Отправим навигационный пакет GPS с невалидными координатами.
   LenDataGprs += frame_build_navigational_not_valid_packet(&g_stGpsDataBack, InpDataBuffer, LenDataGprs);

   // Формируем пакет статуса девайса.
   LenDataGprs += frame_build_status_device_pakeg(InpDataBuffer, LenDataGprs);

   g_stFrame.ucType = C_DATA;
   g_stFrame.ulSerialNumber = g_stRam.stGsm.ucIMEI;
   g_stFrame.usDevicePaketNumber++;
   LenDataGprs = frame_build_paket(&g_stFrame, InpDataBuffer, LenDataGprs, g_asOutDataFrameBuffer);

   if (socket_send(0, g_asOutDataFrameBuffer, LenDataGprs, &mask_ready) == RET_GPRS_SEND_OK)
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_ACKNOW_DATA;
   }
   else
   {
      pGsmStatus->eGsmStep = GSM_OFF;
   }
}

void StepGsmProfileGprsSendDataAccelStatus(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   uint8_t mask_ready = 0;
   int LenDataGprs = 0;
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;
   DP_GSM("\r\nD_Packet ACCEL\r\n");

   if (GetGpsStatus())   // Если имеются спутники, то формируем навигационный пакет.
   {
      LenDataGprs += GetDataNavigationalGpsPacket(InpDataBuffer, NAVIGATIONAL_PACKET, LenDataGprs);
   }
   else
   {
      //Отправим навигационный пакет GPS с невалидными координатами.
      LenDataGprs += frame_build_navigational_not_valid_packet(&g_stGpsDataBack, InpDataBuffer, LenDataGprs);
   }

   /* Если статус остановки и есть не догруженные данные, то догрузим данные */
   if ((g_stEepConfig.stDevice.eModeDevice == TRACK_ION) && (AccelState() == ACC_STATE_STOP) && (GetLenDataBuf()))
   {
      u16 i;
      u16 usLenDataBuf = GetLenDataBuf();
      for (i = 0; i < sizeof(InpDataBuffer) - LenDataGprs; i++)
      {
         InpDataBuffer[LenDataGprs + i] = GetDataBuf(i);
         usLenDataBuf--;
         if (!(usLenDataBuf))
         {
            break;
         }
      }
      LenDataGprs += i;
   }

   g_stFrame.ucType = C_DATA;
   g_stFrame.ulSerialNumber = g_stRam.stGsm.ucIMEI;
   g_stFrame.usDevicePaketNumber++;
   LenDataGprs = frame_build_paket(&g_stFrame, InpDataBuffer, LenDataGprs, g_asOutDataFrameBuffer);

   if (socket_send(0, g_asOutDataFrameBuffer, LenDataGprs, &mask_ready) == RET_GPRS_SEND_OK)
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_ACKNOW_DATA;
   }
   else
   {
      pGsmStatus->eGsmStep = GSM_OFF;
   }
}

void StepGsmProfileGprsSendDataInfoDevice(TGsmStatus* pGsmStatus)
{
   uint8_t mask_ready = 0;
   int LenDataGprs = 0;
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;

   DP_GSM("D_Packet INFO\r\n");

   if (GetGpsStatus())   // Если имеются спутники, то формируем навигационный пакет.
   {
      LenDataGprs += GetDataNavigationalGpsPacket(InpDataBuffer, NAVIGATIONAL_PACKET, LenDataGprs);
   }
   else
   {
      //Отправим навигационный пакет GPS с невалидными координатами.
      LenDataGprs += frame_build_navigational_not_valid_packet(&g_stGpsDataBack, InpDataBuffer, LenDataGprs);
      //Отправим отправляем пакет LBS инициализации.
      LenDataGprs += GetDataNavigationalGsmPacket(InpDataBuffer, LenDataGprs);
   }
   //Отправим пакет переферии.
   LenDataGprs += GetDataPeripheryStatusPacket(&g_stInput, InpDataBuffer, LenDataGprs);

   g_stFrame.ucType = C_DATA;
   g_stFrame.ulSerialNumber = g_stRam.stGsm.ucIMEI;
   g_stFrame.usDevicePaketNumber++;
   LenDataGprs = frame_build_paket(&g_stFrame, InpDataBuffer, LenDataGprs, g_asOutDataFrameBuffer);

   if (socket_send(0, g_asOutDataFrameBuffer, LenDataGprs, &mask_ready) == RET_GPRS_SEND_OK)
   {
      g_stRam.stDevice.bSendInfoDevice = FALSE;
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_ACKNOW_DATA;
   }
   else
   {
      pGsmStatus->eGsmStep = GSM_OFF;
   }
}

void StepGsmProfileGprsSendDataGpioDevice(TGsmStatus* pGsmStatus)
{
   uint8_t mask_ready = 0;
   int LenDataGprs = 0;
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;

   DP_GSM("D_Packet GPIO\r\n");

   //Отправим пакет переферии.
   LenDataGprs += GetDataPeripheryStatusPacket(&g_stInput, InpDataBuffer, LenDataGprs);

   g_stFrame.ucType = C_DATA;
   g_stFrame.ulSerialNumber = g_stRam.stGsm.ucIMEI;
   g_stFrame.usDevicePaketNumber++;
   LenDataGprs = frame_build_paket(&g_stFrame, InpDataBuffer, LenDataGprs, g_asOutDataFrameBuffer);

   if (socket_send(0, g_asOutDataFrameBuffer, LenDataGprs, &mask_ready) == RET_GPRS_SEND_OK)
   {
      g_stRam.stDevice.bSendInfoDevice = FALSE;
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_ACKNOW_DATA;
   }
   else
   {
      pGsmStatus->eGsmStep = GSM_OFF;
   }
}

void StepGsmProfileGprsSendData(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   uint8_t mask_ready = 0;
   int Len, LenDataGprs;
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_1000;
   memset(g_asOutDataFrameBuffer, 0, sizeof(g_asOutDataFrameBuffer));
   g_stRam.stFlash.bFlashReady = FALSE;
   //Пакет по реалтайму
   if (g_stRam.stFlash.bRealtimeReady == TRUE)
   {
      DP_GSM("\r\nD_Packet REAL TIME: ");

      if (GetGpsStatus())
      {
         GSM_DPD("D_GPS\r", strlen("D_GPS\r"));
         Len = GetDataNavigationalGpsPacket(InpDataBuffer, NAVIGATIONAL_PACKET_REAL_TIME, 0);
      }
      else
      {
         GSM_DPD("D_LBS\r\n", strlen("D_LBS\r\n"));
         Len = 0;
         // memset(InpDataBuffer, 0, sizeof(InpDataBuffer));
         //Добавляем к пакету реалтайма LBS еще и навигационный пакет GPS с невалидными координатами
         Len += frame_build_navigational_not_valid_packet(&g_stGpsDataBack, InpDataBuffer, Len);
         Len += GetDataNavigationalGsmPacket(InpDataBuffer, Len);
      }
      g_stFrame.ucType = C_DATA;
      g_stFrame.ulSerialNumber = g_stRam.stGsm.ucIMEI;
      g_stFrame.usDevicePaketNumber++;
      LenDataGprs = frame_build_paket(&g_stFrame, InpDataBuffer, Len, g_asOutDataFrameBuffer);
      ReloadGpsRealTime();
      g_stRam.stFlash.bRealtimeReady = FALSE;

      if (LenDataGprs == NULL)
      {
         pGsmStatus->eGsmStep = CHECK_SMS;
         return;
      }

      if (socket_send(0, g_asOutDataFrameBuffer, LenDataGprs, &mask_ready) == RET_GPRS_SEND_OK)
      {
         pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_ACKNOW_DATA;
      }
      else
      {
         pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_DEACTIVATE;
      }
   }
   else
   {
      DP_GSM("\r\nD_Packet DATA\r\n");
      memset(g_asOutDataFrameBuffer, 0, sizeof(g_asOutDataFrameBuffer));
      u32 uiFlashLen = Flash_DataLen();
      if (uiFlashLen > SIZE_FLASH_BUF)
      {
         uiFlashLen = SIZE_FLASH_BUF;
      }
      LenDataGprs = GetDataToSend(g_asOutDataFrameBuffer, uiFlashLen);
      if (LenDataGprs == NULL)
      {
         pGsmStatus->eGsmStep = CHECK_SMS;
         return;
      }

      if (socket_send(0, g_asOutDataFrameBuffer, LenDataGprs, &mask_ready) == RET_GPRS_SEND_OK)
      {
         pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_ACKNOW_DATA;
      }
      else
      {
         pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_DEACTIVATE;
      }
   }
}

/* UPDATE FIRMWARE ************************************************************/
/* Даем настройки HTTP сервера */
void StepGsmProfileHttpSocket(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_10;
   profile_deactivate(PROF_HTTP_GET_SERVER);   // Close GPRS Connect

   RET_INFO eStatusRet = ProfileSocketInit("", PROF_HTTP_GET_SERVER);
   if (eStatusRet == RET_OK)
   {
      pGsmStatus->uiGsmStepDelay = SLEEP_MS_500;
      pGsmStatus->eGsmStep = GSM_PROFILE_HTTP_DOWNLOAD;
   }
   else
   {
      g_stRam.stFirmware.g_bNewFirmware =
         TRUE;   //Ставим флаг, что пришла команда на скачивание прошивки, но команда не правильная.
      g_stEepConfig.stFirmware.eFlagsStatusFirmware = ERR_CONNECT_FTP_OR_HTTP;
      SaveConfigCMD();
      pGsmStatus->eGsmStep = GSM_OFF;
   }
}

/* Стучимся на HTTP сервер */
/* Качаем прошивку с HTTP сервера */
void StepGsmProfileHttpDownload(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   FRAME_FIRMWARE_TYPE eFirmUpdataStatus;

#define DEF_HTTP_SERVER "web.irzonline.com/f.php?f=1463649663"   //&i=353437069574298&b=0//1458917674
   // memcpy(g_stEepConfig.stFirmware.strNameFtpSer, DEF_HTTP_SERVER, sizeof(DEF_HTTP_SERVER));

   g_stRam.stFirmware.g_bNewFirmware = TRUE;   //Ставим флаг, что пришла команда на скачивание прошивки.
   eFirmUpdataStatus = (FRAME_FIRMWARE_TYPE)profile_http_read(PROF_HTTP_GET_SERVER);   // Download Firmware
   profile_deactivate(PROF_HTTP_GET_SERVER);   // Close GPRS Connect
   if (eFirmUpdataStatus == FIRMWARE_OK)
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_CHECK_FIRMWARE;
   }
   else
   {
      g_stEepConfig.stFirmware.eFlagsStatusFirmware = eFirmUpdataStatus;   //Статус Err закаченой прошивки.
      SaveConfigCMD();
      pGsmStatus->eGsmStep = GSM_OFF;
   }
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_10;
}

/* Проверим CRC скаченной прошивки */
void StepGsmProfileCheckFirmware(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   g_stEepConfig.stFirmware.eFlagsStatusFirmware = (FRAME_FIRMWARE_TYPE)check_firmware();
   SaveConfigCMD();
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_10;
   pGsmStatus->eGsmStep = GSM_OFF;
}
/******************************************************************************/

int GSM_State(VALUE prm)
{
#define TIMEOUT_POWER_GSM 5000000
   uint32_t timeout_power_gsm = TIMEOUT_POWER_GSM;
   if (prm == ON)
   {
      memset(g_asCmdBuf, 0, sizeof(g_asCmdBuf));
      memset(g_asRxBuf, 0, sizeof(g_asRxBuf));
      memset(g_aucTxBufferUSART2, 0, GSM_TX_BUFFER_SIZE);
      memset(g_aucRxBufferUSART2, 0, GSM_RX_BUFFER_SIZE);

      InitUSART(UART_GSM, GSM_BAUDRATE);
      InitDMA(UART_GSM);

      ReStartDmaGsmUsart();   //Перезапускаем DMA, так навсякий случай.

      osDelay(100);

      GSM_Reference(prm);

#ifdef GSM_MODULE_SIM800
      osDelay(1000);
      PWR_KEY_PULL_UP;
      osDelay(1000);
      PWR_KEY_PULL_DOWN;
#else
      while ((!(GSM_STATUS_ON)) && (timeout_power_gsm))
      {
         timeout_power_gsm--;
         IWDG_ReloadCounter(); /* Reload IWDG counter */
      }
#endif

      if (timeout_power_gsm)
      {
         return RET_OK;
      }
   }

   if (prm == OFF)
   {
#ifndef GSM_MODULE_SIM800
      modem_off();
#endif
      DeInitUSART(UART_GSM);
      GSM_Reference(prm);

      while ((GSM_STATUS_ON) && (timeout_power_gsm))
      {
         timeout_power_gsm--;
         IWDG_ReloadCounter(); /* Reload IWDG counter */
      }

      if (timeout_power_gsm)
      {
         g_stRam.stGsm.aucGsmCsq[0] = 0;
         g_stRam.stGsm.aucGsmCsq[1] = 0;
         return RET_OK;
      }
   }

   if (prm == SLEEP)
   {
      WakeupGSM(0);
   }

   return ERR_POWER;
}

GSM_STEP GetGsmStep(void)
{
   return stGsmStatus.eGsmStep;
}

//Задание настроек GSM модему в энергорежиме1. Возвращаем время ожидания GSM модема.
uint32_t GsmLowPwr1(void)
{
   /*
  RTC_t DateRTC;
  uint32_t SecRTC;
  rtc_gettime(&DateRTC);
  SecRTC = Date2Sec(&DateRTC);
  */
   uint32_t delay_gsm_off = (uint32_t)g_stEepConfig.stDevice.usTimeSleepLowPwrMode1 * 60;

   DP_GSM("D_PWR LOW STATUS: PWR1\r");

   DP_GSM("D_GSM OFF DELAY: %isec.\r", delay_gsm_off);

   g_stRam.stDevice.eLedStatus = LOW_PWR1;   //Задаем лед статус в «спящем режиме» LOW_PWR1
   g_stRam.stDevice.eResetStatusDevice = WAKE_UP_LOW_PWR1;
   SetStatusDeviceReset(g_stRam.stDevice.eResetStatusDevice);
   return delay_gsm_off;
}

//Задание настроек GSM модему в энергорежиме2. Возвращаем время ожидания GSM модема.
uint32_t GsmLowPwr2(void)
{
   /*
   RTC_t DateRTC;
   uint32_t SecRTC;
   rtc_gettime(&DateRTC);
   SecRTC = Date2Sec(&DateRTC);
    */
   uint32_t delay_gsm_off = (uint32_t)g_stEepConfig.stDevice.usTimeSleepLowPwrMode2 * 60;

   DP_GSM("D_PWR LOW STATUS: PWR2\r");
   DP_GSM("D_GSM OFF DELAY: %isec.\r", delay_gsm_off);
   g_stRam.stDevice.eLedStatus = LOW_PWR2;   //Задаем лед статус в «спящем режиме» LOW_PWR2
   g_stRam.stDevice.eResetStatusDevice = WAKE_UP_LOW_PWR2;
   SetStatusDeviceReset(g_stRam.stDevice.eResetStatusDevice);
   return delay_gsm_off;
}

void SetGsmStep(GSM_STEP eCmdGsmStep)
{
   stGsmStatus.eCmdGsmStep = eCmdGsmStep;
}

void StepSleep(void)
{
   DPS("\r\n-D_DEVICE SLEEP-\r\n");
   xSemaphoreGive(mDEINIT_GPS_MODULE);
   AccelPowerDown(g_stEepConfig.stAccel.ucSensitivity);
   DelayResolution100us(3000);
   SleepModeDevice();
}

void StepRestartNow(void)
{
   DPS("\r\n-D_DEVICE REBOOT-\r\n");
   xSemaphoreGive(mDEINIT_GPS_MODULE);
   ResetDevice();
}