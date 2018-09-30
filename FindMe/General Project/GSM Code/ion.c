
#include "includes.h"
#include "ion.h"
#include "gsm_general.h"

static uint8_t g_ucFmStepCount = 0;
static _Bool g_bNewConfig = FALSE;
extern char g_aucBufDownHttpFirm[];
#define SERVER_TRUE_VALUE g_aucBufDownHttpFirm

void resetFmStepCount(void)
{
   g_ucFmStepCount = 0;
}

void stepGsmProfileGprsSendDataInit(TGsmStatus* pGsmStatus)
{
   DP_GSM("D_GSM STEP: stepGsmProfileGprsSendDataInit\r\n");

   pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;
   int LenDataInitPacket = 0;
   uint8_t mask_ready = 0;
   g_stFrame.usDevicePaketNumber = 0;

   DP_GSM("D_PACKET INIT\r\n");

   g_stFrame.ucType = C_CONNECT;
   g_stFrame.ulSerialNumber = GetIMEI();
   g_stFrame.usDevicePaketNumber++;
   memset(InpDataBuffer, 0, sizeof(InpDataBuffer));
   memset(g_aucOutDataFrameBuffer, 0, sizeof(g_aucOutDataFrameBuffer));

   LenDataInitPacket = FrameGeneralBuild(&g_stFrame, InpDataBuffer, 0, g_aucOutDataFrameBuffer);
   // pGsmStatus->eGsmStep = GSM_CONNECT_ERROR; return;

   if (socket_send(0, g_aucOutDataFrameBuffer, LenDataInitPacket, &mask_ready) == RET_GPRS_SEND_OK)
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_ACKNOW_DATA;
   }
   else
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_DEACTIVATE;
   }
}

void stepGsmSwitchData(TGsmStatus* pGsmStatus)
{
   DP_GSM("D_GSM STEP: stepGsmSwitchData\r\n");

   if (g_stFrame.usFlagsRetDataServer & (fS_FAIL))
   {
      g_stFrame.usFlagsRetDataServer &= ~fS_FAIL;
      // pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_SEND_C_FAIL;
      pGsmStatus->uiGsmStepDelay = SLEEP_MS_10;
      DP_GSM("D_SERVER ERR: S_FAIL\r\n");
   }

   if (g_stFrame.usFlagsRetDataServer & (fC_FAIL))
   {
      g_stFrame.usFlagsRetDataServer &= ~fC_FAIL;

      Flash_DataSendOK();
      pGsmStatus->eGsmStep = CHECK_SMS;
      pGsmStatus->uiGsmStepDelay = SLEEP_MS_10;
      DP_GSM("D_SERVER ERR: C_FAIL\r\n");
   }

   if (g_stFrame.usFlagsRetDataServer & (fS_ACK))
   {
      g_stFrame.usFlagsRetDataServer &= ~fS_ACK;
      if (g_stFrame.usServerPaketNumber == g_stFrame.usDeviceDataPaketNumber)
      {
         Flash_DataSendOK();
      }

      pGsmStatus->uiGsmStepDelay = SLEEP_MS_10;
      if (GetModeDevice() != TRACK_ION && g_ucFmStepCount != ARCHIVE_DATA)
      {
         g_ucFmStepCount++;
      }

      /* проверим что новая конфигурация */
      if (g_bNewConfig)
      {
         g_bNewConfig = FALSE;

         /* определим что будем делать после получения новой конфигурации */
         if (GetModeDevice() == TRACK_ION)
         {   //перезагрузимся
            uint8_t type_config = 0;
            for (int i = 0; i < MAX_PARAM_CONFIG_VALUE; i++)
            {
               if (SERVER_TRUE_VALUE[i])
               {
                  type_config = SERVER_TRUE_VALUE[i];
                  break;
               }
            }

            /* перезагрузимся если конфиг установлен сервером */
            if (type_config == SERVER_CONFIG_DEVICE_PACKET)
            {
               pGsmStatus->eGsmStep = RESTART_NOW;
            }
            else
            {
               pGsmStatus->eGsmStep = CHECK_SMS;
            }
         }
         else
         {
            pGsmStatus->eGsmStep = CHECK_SMS;   //продолжим работу, так как режим маяка и все равно потом уснем.
         }
         memset(SERVER_TRUE_VALUE, 0, MAX_PARAM_CONFIG_VALUE);
      }
      else
      {
         if (getFlagReset())
         {
            pGsmStatus->eGsmStep = RESTART_NOW;
         }
         else
         {
            pGsmStatus->eGsmStep = CHECK_SMS;
         }
      }
      DP_GSM("D_SERVER ANS: S_ACK\r\n");
   }

   if (g_stFrame.usFlagsRetDataServer & (fS_ACK_DLY | fS_FIN))
   {
      if (g_stFrame.usFlagsRetDataServer & (fS_ACK_DLY))
      {
         g_stFrame.usFlagsRetDataServer &= ~fS_ACK_DLY;
         pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_DEACTIVATE;   //Сделаем переконнект на рандомное время
         DP_GSM("D_SERVER ANS: S_ACK_DLY\r\n");
      }
      else
      {
         g_stFrame.usFlagsRetDataServer &= ~fS_FIN;

         pGsmStatus->uiGsmStepDelay = GSM_PROFILE_GPRS_DEACTIVATE;   //Сделаем переконнект на рандомное время
         DP_GSM("D_SERVER ANS: S_FIN\r\n");
      }

      pGsmStatus->uiGsmStepDelay = SLEEP_MS_10;
   }

   // Data //
   if (g_stFrame.usFlagsRetDataServer & (fS_ASK_DATA | S_DATA))
   {
      g_stFrame.usFlagsRetDataServer &= ~fS_ASK_DATA;
      g_stFrame.usFlagsRetDataServer &= ~S_DATA;
      g_stFrame.usServerDataPaketNumber = g_stFrame.usServerPaketNumber;

      if (g_stFrame.usServerPaketNumber == g_stFrame.usDeviceDataPaketNumber)
      {
         Flash_DataSendOK();
      }

      // Обрабатываем данные от сервера.
      pGsmStatus->eNextAckGsmStep = parsingData((uint8_t*)&g_aucOutDataFrameBuffer[14]);

      if (g_stFrame.usServerPaketNumber == g_stFrame.usDeviceDataPaketNumber)
      {
         Flash_DataSendOK();
      }

      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_SEND_C_ACK;   //
      pGsmStatus->uiGsmStepDelay = SLEEP_MS_10;
      if (GetModeDevice() != TRACK_ION && g_ucFmStepCount == SYNCHRO_TIME)
      {
         g_ucFmStepCount++;
      }
      DP_GSM("D_SERVER ANS: S_ASK_DATA\r\n");
   }
}

//Отправляем серверу потверждение C_ACK.
void stepGsmSendAckC(TGsmStatus* pGsmStatus)
{
   DP_GSM("D_GSM STEP: stepGsmSendAckC\r\n");

   pGsmStatus->uiGsmStepDelay = SLEEP_MS_10;
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

/* Шаг на котором определяем что будем передавать серверу в режиме маяка */
void stepFindMeDataReady(TGsmStatus* pGsmStatus)
{
   DP_GSM("D_GSM STEP: stepFindMeDataReady\r\n");
   static ACC_STATE fAccelStation = ACC_STATE_STOP;
   if (GetStatusReset() == WAKE_UP_ACCEL)
      fAccelStation = ACC_STATE_MOVE;
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;
   // static _Bool bFlagEndMessageStop = 0;
   if (g_ucFmStepCount < SYNCHRO_TIME || g_ucFmStepCount > END_SMS)
   {
      g_ucFmStepCount = STATUS_DEVICE;
   }

   //Пакет инфы о новой и текущей прошивки.
   if (g_stRam.stFirmware.bNewFirmware == TRUE && g_stRam.stFirmware.bFirmSendServ == TRUE)
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_SEND_FIRMWARE_STATUS;
      g_ucFmStepCount = SYNCHRO_TIME;
      return;
   }

   switch (g_ucFmStepCount)
   {
   case SYNCHRO_TIME:
      if (!(SerSinchroTime()))
      {   //Если есть синхронизация времени
         DP_GSM("D_FM SYNCHRON TIME\r\n");
         pGsmStatus->eGsmStep = SYNCHRONIZATION_SERVER_TIME_REQUEST;
         break;
      }
      else
      {
         g_ucFmStepCount++;
      }

   case STATUS_DEVICE:
      DP_GSM("D_FM SEND DATA STATUS DEVICE\r\n");
      if (g_uiPacStatDevTime < 1427464200)
      {
         g_uiPacStatDevTime = time();
      }
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_SEND_DATA_STATUS_DEVICE;
      break;

   case DATA_FM:
      DP_GSM("D_FM DATA\r\n");
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_SEND_DATA_FM;
      break;

   case LOG_DATA:
#ifndef USE_LOG_DATA
#   define USE_LOG_DATA 0
#endif
#if (USE_LOG_DATA)
      DP_GSM("D_FM LOG\r\n");
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_SEND_LOG_DATA;
      break;
#else
      g_ucFmStepCount++;
#endif

   case ARCHIVE_DATA:
      if (GetCountDataFlash())
      {
         DP_GSM("D_FM ARCHIVE DATA\r\n");
         pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_SEND_ARCHIVE_DATA_FM;
         break;
      }
      else
      {
         g_ucFmStepCount++;
      }

   case CLOSE_CONNECT:
      DP_GSM("D_FM CLOSE CONNECT\r\n");
      profile_deactivate(PROF_FIRST_SERVER);
      g_ucFmStepCount++;

   case WAIT_STOP:   //Если режим поиска по акселерометру и есть движение, то будем ждать остановки ТС
      if ((GetEnableAccelToFind()) && (fAccelStation == ACC_STATE_MOVE || AccelState() == ACC_STATE_MOVE) &&
          (GetModeDevice() == TIMER_FIND))
      {
         DP_GSM("D_FM WAIT ACCEL STOP\r\n");
         fAccelStation = ACC_STATE_STOP;
         pGsmStatus->eGsmStep = GSM_SLEEP_WAIT_ACCEL_STOP;
         SetStatusReset(WAKE_UP_STOP);
         // bFlagEndMessageStop = 1;
         break;
      }
      else
      {
         g_ucFmStepCount++;   //иначе след. шаг
      }

   case END_SMS:
      DP_GSM("D_FM END SMS\r\n");
      SetCountReConnect(0);
      pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;
      pGsmStatus->eGsmStep = GSM_PROFILE_END_SMS;
   }
}

void set_fm_step_count(uint8_t ucStep)
{
   g_ucFmStepCount = ucStep;
}

void stepGsmProfileGprsSendStatusDevice(TGsmStatus* pGsmStatus)
{
   DP_GSM("D_GSM STEP: stepGsmProfileGprsSendStatusDevice\r\n");

   uint8_t mask_ready = 0;
   int LenDataGprs = 0;
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;

   // Формируем пакет статуса девайса.
   LenDataGprs += frame_build_status_device_package(InpDataBuffer, LenDataGprs);
   if (!(LenDataGprs))
   {
      pGsmStatus->eGsmStep = CHECK_SMS;
      return;
   }
   DP_GSM("\r\nD_Packet STATUS\r\n");

   g_stFrame.ucType = C_DATA;
   g_stFrame.ulSerialNumber = GetIMEI();
   g_stFrame.usDevicePaketNumber++;

   LenDataGprs = FrameGeneralBuild(&g_stFrame, InpDataBuffer, LenDataGprs, g_aucOutDataFrameBuffer);
   if (socket_send(0, g_aucOutDataFrameBuffer, LenDataGprs, &mask_ready) == RET_GPRS_SEND_OK)
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_ACKNOW_DATA;
   }
   else
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_DEACTIVATE;
   }
}

/* Отправка лога работы девайса */
void stepGsmProfileGprsSendLogDevice(TGsmStatus* pGsmStatus)
{
   DP_GSM("D_GSM STEP: stepGsmProfileGprsSendLogDevice\r\n");

   uint8_t mask_ready = 0;
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;
   int LenDataGprs = frame_build_log_device_paket(InpDataBuffer, 0);

   g_stFrame.ucType = C_DATA;
   g_stFrame.ulSerialNumber = GetIMEI();
   g_stFrame.usDevicePaketNumber++;
   LenDataGprs = FrameGeneralBuild(&g_stFrame, InpDataBuffer, LenDataGprs, g_aucOutDataFrameBuffer);

   if (socket_send(0, g_aucOutDataFrameBuffer, LenDataGprs, &mask_ready) == RET_GPRS_SEND_OK)
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_ACKNOW_DATA;
   }
   else
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_DEACTIVATE;
   }
}

void stepGsmProfileGprsAccelStatus(TGsmStatus* pGsmStatus)
{
   DP_GSM("D_GSM STEP: stepGsmProfileGprsAccelStatus\r\n");

   uint8_t mask_ready = 0;
   int LenDataGprs = 0;
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;
   if (GetGpsStatus())
   {
      LenDataGprs = GetDataNavigationalGpsPacket(InpDataBuffer, NAVIGATIONAL_PACKET_REAL_TIME, 0);
   }
   else
   {
      LenDataGprs = frame_build_navigational_not_valid_packet(InpDataBuffer, 0);
   }

   g_stFrame.ucType = C_DATA;
   g_stFrame.ulSerialNumber = GetIMEI();
   g_stFrame.usDevicePaketNumber++;
   LenDataGprs = FrameGeneralBuild(&g_stFrame, InpDataBuffer, LenDataGprs, g_aucOutDataFrameBuffer);

   if (LenDataGprs == 0)
   {
      pGsmStatus->eGsmStep = CHECK_SMS;
      return;
   }

   if (socket_send(0, g_aucOutDataFrameBuffer, LenDataGprs, &mask_ready) == RET_GPRS_SEND_OK)
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_ACKNOW_DATA;
   }
   else
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_DEACTIVATE;
   }
}

void stepGsmSendAnsOkData(TGsmStatus* pGsmStatus)
{
   DP_GSM("D_GSM STEP: stepGsmSendAnsOkData\r\n");

   RET_INFO eStatusRet = GprsSendAnsOkData();
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_10;
   if (eStatusRet == RET_OK)
   {
      pGsmStatus->eGsmStep = CHECK_SMS;   // GSM_PROFILE_GPRS_DEACTIVATE;
   }
   else
   {
      pGsmStatus->eGsmStep = GSM_OFF;
   }
}

void stepGsmSendFailC(TGsmStatus* pGsmStatus)
{
   DP_GSM("D_GSM STEP: stepGsmSendFailC\r\n");

   RET_INFO eStatusRet = GprsSendFailC();
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_10;
   if (eStatusRet == RET_OK)
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_DEACTIVATE;
   }
   else
   {
      pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;
      pGsmStatus->eGsmStep = GSM_OFF;
   }
}

void stepGsmProfileGprsAcknowData(TGsmStatus* pGsmStatus)
{
   DP_GSM("D_GSM STEP: stepGsmProfileGprsAcknowData\r\n");

   pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;
   pGsmStatus->eGsmStep = GprsAckData();
}

void stepGsmProfileGprsSendDataStatusFirmware(TGsmStatus* pGsmStatus)
{
   DP_GSM("D_GSM STEP: stepGsmProfileGprsSendDataStatusFirmware\r\n");

   int LenDataInitPacket = 0;
   int iLen = 0;
   uint8_t mask_ready = 0;
   g_stFrame.usDevicePaketNumber = 0;

   pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;
   g_stFrame.ucType = C_DATA;
   g_stFrame.ulSerialNumber = GetIMEI();
   g_stFrame.usDevicePaketNumber++;

   iLen = frame_build_received_firmware(InpDataBuffer);   // Формируем пакет инфы о прошивке.

   LenDataInitPacket = FrameGeneralBuild(&g_stFrame, InpDataBuffer, iLen, g_aucOutDataFrameBuffer);
   DP_GSM("\r\nD_Packet FIRMWARE:\r\n");

   DP_GSM("D_CUR: %d\r\n", flash_read_word(__CONST_FIRM_VER));
   DP_GSM("D_NEW FLS: %d\r\n", g_stRam.stFirmware.uiNameNewFirmware);
   DP_GSM("D_NEW EEP: %d\r\n", GetIntNewFirmware());

   if (socket_send(0, g_aucOutDataFrameBuffer, LenDataInitPacket, &mask_ready) == RET_GPRS_SEND_OK)
   {
      g_stRam.stFirmware.bNewFirmware = FALSE;
      g_stRam.stFirmware.bFirmSendServ = FALSE;
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_ACKNOW_DATA;
   }
   else
   {
      pGsmStatus->eGsmStep = GSM_OFF;
   }
}

void stepSynchronizationServerTimeRequest(TGsmStatus* pGsmStatus)
{
   DP_GSM("D_GSM STEP: stepSynchronizationServerTimeRequest\r\n");

   uint16_t Len;
   int LenDataGprs = 0;
   uint8_t mask_ready = 0;
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;
   uint32_t SecRTC = time();

   Len = ServerSynchroTime(SecRTC, InpDataBuffer, 0);

   g_stFrame.ucType = C_DATA;
   g_stFrame.ulSerialNumber = GetIMEI();
   g_stFrame.usDevicePaketNumber++;
   LenDataGprs = FrameGeneralBuild(&g_stFrame, InpDataBuffer, Len, g_aucOutDataFrameBuffer);
   DP_GSM("\r\nD_Packet SINCHRO TIME\r\n");

   if (socket_send(0, g_aucOutDataFrameBuffer, LenDataGprs, &mask_ready) == RET_GPRS_SEND_OK)
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_ACKNOW_DATA;
   }
   else
   {
      pGsmStatus->eGsmStep = GSM_OFF;
   }
}

void stepGsmProfileGprsSendDataION(TGsmStatus* pGsmStatus)
{
   DP_GSM("D_GSM STEP: stepGsmProfileGprsSendDataION\r\n");

   uint8_t mask_ready = 0;
   int Len, LenDataGprs;
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;
   memset(g_aucOutDataFrameBuffer, 0, sizeof(g_aucOutDataFrameBuffer));
   g_stRam.stFlash.bFlashReady = FALSE;
   //Пакет по реалтайму
   if (g_stRam.stFlash.bRealtimeReady == TRUE)
   {
      DP_GSM("\r\nD_Packet REAL TIME: ");

      if (GetGpsStatus())
      {
         DP_GSM("D_GPS\r\n");
         Len = GetDataNavigationalGpsPacket(InpDataBuffer, NAVIGATIONAL_PACKET_REAL_TIME, 0);
      }
      else
      {
         DP_GSM("D_LBS\r\n");
         Len = 0;
         // memset(InpDataBuffer, 0, sizeof(InpDataBuffer));
         //Добавляем к пакету реалтайма LBS еще и навигационный пакет GPS с невалидными координатами
         Len += frame_build_navigational_not_valid_packet(InpDataBuffer, Len);
         Len += GetDataNavigationalGsmPacket(InpDataBuffer, Len);
      }

      g_stFrame.ucType = C_DATA;
      g_stFrame.ulSerialNumber = GetIMEI();
      g_stFrame.usDevicePaketNumber++;
      LenDataGprs = FrameGeneralBuild(&g_stFrame, InpDataBuffer, Len, g_aucOutDataFrameBuffer);
      ReloadGpsRealTime();
      g_stRam.stFlash.bRealtimeReady = FALSE;

      if (LenDataGprs == 0)
      {
         pGsmStatus->eGsmStep = CHECK_SMS;
         return;
      }

      if (socket_send(0, g_aucOutDataFrameBuffer, LenDataGprs, &mask_ready) == RET_GPRS_SEND_OK)
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
      if (g_stRam.stFlash.bTempBufReady == TRUE)
      {
         /* Выгрузка данных из временного буфера (реализация догрузки данных) */
         DP_GSM("\r\nD_Packet BUF DATA\r\n");
         memset(g_aucOutDataFrameBuffer, 0, sizeof(g_aucOutDataFrameBuffer));
         LenDataGprs = GetBufDataToSend(g_aucOutDataFrameBuffer);
         g_stRam.stFlash.bTempBufReady = FALSE;
      }
      else
      {
         /* Выгрузка данных из flash */
         DP_GSM("\r\nD_Packet FLASH DATA\r\n");
         memset(g_aucOutDataFrameBuffer, 0, sizeof(g_aucOutDataFrameBuffer));
         u32 uiFlashLen = Flash_DataLen();
         if (uiFlashLen > SIZE_FLASH_BUF)
         {
            uiFlashLen = SIZE_FLASH_BUF;
         }
         LenDataGprs = GetFlashDataToSend(g_aucOutDataFrameBuffer, uiFlashLen);
      }

      if (LenDataGprs == 0)
      {
         pGsmStatus->eGsmStep = CHECK_SMS;
         return;
      }

      if (socket_send(0, g_aucOutDataFrameBuffer, LenDataGprs, &mask_ready) == RET_GPRS_SEND_OK)
      {
         pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_ACKNOW_DATA;
      }
      else
      {
         pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_DEACTIVATE;
      }
   }
}

void stepGsmProfileGprsSendDataConfig(TGsmStatus* pGsmStatus)
{
   DP_GSM("D_GSM STEP: stepGsmProfileGprsSendDataConfig\r\n");

   uint8_t mask_ready = 0;
   int LenDataGprs, Len;

   uint8_t type_config = 0;
   for (int i = 0; i < MAX_PARAM_CONFIG_VALUE; i++)
   {
      if (SERVER_TRUE_VALUE[i])
      {
         type_config = SERVER_TRUE_VALUE[i];
         break;
      }
   }

   Len = frame_build_config_packet(type_config, InpDataBuffer, 0);

   g_stFrame.ucType = C_DATA;
   g_stFrame.ulSerialNumber = GetIMEI();
   g_stFrame.usDevicePaketNumber++;
   LenDataGprs = FrameGeneralBuild(&g_stFrame, InpDataBuffer, Len, g_aucOutDataFrameBuffer);

   if (socket_send(0, g_aucOutDataFrameBuffer, LenDataGprs, &mask_ready) == RET_GPRS_SEND_OK)
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_ACKNOW_DATA;
      g_bNewConfig = TRUE;
   }
   else
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_DEACTIVATE;
   }
}

void stepGsmProfileGprsSendArchiveData(TGsmStatus* pGsmStatus)
{
   DP_GSM("D_GSM STEP: stepGsmProfileGprsSendArchiveDataConfig\r\n");
   DP_GSM("D_DataPoint: %i\r\n", GetCountDataFlash());

   uint8_t mask_ready = 0;
   int LenDataGprs;
   int Len = ReadDataFm(GetCountDataFlash(), InpDataBuffer);

   g_stFrame.ucType = C_DATA;
   g_stFrame.ulSerialNumber = GetIMEI();
   g_stFrame.usDevicePaketNumber++;
   LenDataGprs = FrameGeneralBuild(&g_stFrame, InpDataBuffer, Len, g_aucOutDataFrameBuffer);

   if (socket_send(0, g_aucOutDataFrameBuffer, LenDataGprs, &mask_ready) == RET_GPRS_SEND_OK)
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_ACKNOW_DATA;
   }
   else
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_DEACTIVATE;
   }
}

/* Шаг на котором определяем что будем передавать серверу в режиме iON */
void stepWaitFlashDataReady(TGsmStatus* pGsmStatus)
{
   DP_GSM("D_GSM STEP: StepWaitFlashDataReady\r\n");

   static int iReConnectCount = 0;

   //Далее располагаем в порядке приоритета, то что будем передавать:

   //Пакет инфы о новой и текущей прошивки.
   if (g_stRam.stFirmware.bNewFirmware == TRUE && g_stRam.stFirmware.bFirmSendServ == TRUE)
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
   if (ChangeStatusDevice())
   {   //Если менялся, то отправим его статус.
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_SEND_DATA_STATUS_DEVICE;
      iReConnectCount = 0;
      return;
   }

   //Если произошло событие остановки/движения девайса.
   ACC_STATE eAccelStopEvent = AccelState();
   static ACC_STATE eBackAccelStopEvent = (ACC_STATE)0xFF;
   if (eAccelStopEvent != eBackAccelStopEvent)
   {
      eBackAccelStopEvent = eAccelStopEvent;
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_ACCEL_STATUS;
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

      default:
         iReConnectCount = 0;
         return;
      }
   }

   //Если есть данные.
   if ((g_stRam.stFlash.bFlashReady == TRUE) || (g_stRam.stFlash.bRealtimeReady == TRUE) ||
       (g_stRam.stFlash.bTempBufReady == TRUE))
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_SEND_DATA_ION;
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
   DP_GSM("D_TIME OPEN SOCKET: %i\r\n", iReConnectCount);
}

void stepGsmProfileGprsSendDataFm(TGsmStatus* pGsmStatus)
{
   DP_GSM("D_GSM STEP: stepGsmProfileGprsSendDataFm\r\n");

   uint8_t mask_ready = 0;
   int LenDataGprs = 0;
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_1;
   memset(g_aucOutDataFrameBuffer, 0, sizeof(g_aucOutDataFrameBuffer));
   g_stRam.stFlash.bFlashReady = FALSE;

   GPS_INFO stGpsData;

   //Пакет по реалтайму
   DP_GSM("\r\nD_Packet Data FM:\r\n");
   if (GetPositionGps(&stGpsData))
   {
      if (stGpsData.time < 1427464200)
      {
         stGpsData.time = time();
      }
      DP_GSM("D_GPS REALTIME\r\n");
      LenDataGprs += frame_build_navigational_packet_realtime(&stGpsData, &InpDataBuffer[LenDataGprs]);
      // LenDataGprs += frame_build_navigational_packet_track(&stGpsData, &InpDataBuffer[LenDataGprs]);
   }
   else
   {
      LenDataGprs = 0;
      // memset(InpDataBuffer, 0, sizeof(InpDataBuffer));
      //Добавляем к пакету реалтайма LBS еще и навигационный пакет GPS с невалидными координатами
      DP_GSM("D_GPS NOT VALID\r\n");
      LenDataGprs += frame_build_navigational_not_valid_packet(InpDataBuffer, LenDataGprs);
      DP_GSM("D_LBS\r\n");
      GSM_INFO stGsm;
      TS_MOND base_station;
      stGsm.inf = &base_station;

      if (getInfoLbsData(&stGsm))
      {
         LenDataGprs += frame_build_lbs_packet(&stGsm, InpDataBuffer, LenDataGprs);
      }
      else
      {
         DP_GSM("D_FIND GSM LBS, PLEASE WAIT\r\n");
         LenDataGprs += GetDataNavigationalGsmPacket(InpDataBuffer, LenDataGprs);
      }
   }

   DP_GSM("D_GPIO: %d Time, %d Vbat\r\n", g_stInput.SecADC, g_stInput.Meas_VIN);
   extern int GetDataPeripheryPacket(TPortInputCFG * pAds, char* pOut, int OffsetData);
   LenDataGprs += GetDataPeripheryPacket(&g_stInput, InpDataBuffer, LenDataGprs);

   g_stFrame.ucType = C_DATA;
   g_stFrame.ulSerialNumber = GetIMEI();
   g_stFrame.usDevicePaketNumber++;
   LenDataGprs = FrameGeneralBuild(&g_stFrame, InpDataBuffer, LenDataGprs, g_aucOutDataFrameBuffer);

   if (LenDataGprs == 0)
   {
      pGsmStatus->eGsmStep = CHECK_SMS;
      return;
   }

   if (socket_send(0, g_aucOutDataFrameBuffer, LenDataGprs, &mask_ready) == RET_GPRS_SEND_OK)
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_ACKNOW_DATA;
   }
   else
   {
      pGsmStatus->eGsmStep = GSM_PROFILE_GPRS_DEACTIVATE;
   }
}
