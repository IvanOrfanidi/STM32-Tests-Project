
#ifndef _GSM_GENERAL_H_
#define _GSM_GENERAL_H_

#include "includes.h"
#include "gsm_parser.h"
#include "eeprom.h"
#include "ram.h"

typedef enum
{
   SYNCHRO_TIME = 1,
   STATUS_DEVICE,
   DATA_FM,
   LOG_DATA,
   ARCHIVE_DATA,
   CLOSE_CONNECT,
   WAIT_STOP,
   END_SMS,
} TYPE_PAC_ION;

typedef enum GSM_STEP
{
   GSM_OFF = 0,
   GSM_OFF_LOW_PWR,
   GSM_START,
   GSM_SETTING_START,
   GSM_CONFIG,
   GSM_JAMMING_DETECTED,
   GSM_SIM,
   GSM_SIM_PIN,
   GSM_ADDITIONAL_CONFIG,
   GSM_MESSAGE_EVENT,
   GSM_CELLULAR_NETWORK,
   GSM_CHECK_SMS,
   GSM_PROFILE_GPRS_CONNECT,
   GSM_PROFILE_GPRS_SOCKET,
   GSM_PROFILE_GPRS_ACTIVATE,
   GSM_PROFILE_GPRS_DEACTIVATE,
   GSM_DEACTIVATE_CHECK_SMS_ACTIVATE,
   GSM_PROFILE_GPRS_SEND_DATA_INIT,
   GSM_SWITCH_DATA,
   GSM_PROFILE_GPRS_SEND_C_ACK,
   CHECK_SMS,
   WAIT_FLASH_DATA_READY,
   GSM_PROFILE_GPRS_ANS_CMD_SERVER_OK,
   GSM_PROFILE_GPRS_ANS_CMD_SERVER_ERR,
   GSM_PROFILE_GPRS_ACKNOW_DATA,
   FIND_ME_DATA_READY,
   GSM_SLEEP_WAIT_GPS,
   GSM_SLEEP_WAIT_ACCEL_STOP,
   GSM_PROFILE_GPRS_SEND_FIRMWARE_STATUS,
   SYNCHRONIZATION_SERVER_TIME_REQUEST,
   GSM_PROFILE_GPRS_SEND_DATA_STATUS_DEVICE,
   GSM_PROFILE_GPRS_SEND_LOG_DATA,
   GSM_PROFILE_GPRS_ACCEL_STATUS,
   GSM_PROFILE_GPRS_SEND_DATA_FM,
   GSM_PROFILE_GPRS_SEND_DATA_ION,
   GSM_PROFILE_GPRS_SEND_DATA_CONFIG,
   GSM_PROFILE_GPRS_SEND_ARCHIVE_DATA_FM,

   GSM_PROFILE_GPRS_SEND_DATA_911,
   GSM_PROFILE_GPRS_ACKNOW_DATA_911,
   GSM_PROFILE_WAIT_REG_USER_911,

   GSM_PROFILE_END_SMS,

   GSM_PROFILE_HTTP_SOCKET,
   GSM_PROFILE_HTTP_DOWNLOAD,
   GSM_PROFILE_CHECK_FIRMWARE,

   GSM_CONNECT_ERROR,
   SLEEP_DOWN,
   RESTART_NOW,

#if (USE_TEST_DEVICE)
   DEVICE_TEST,
#endif

} GSM_STEP;

typedef __packed struct
{
   GSM_STEP eGsmStep;
   uint32_t uiGsmStepDelay;
   GSM_STEP eNextAckGsmStep;
   GSM_STEP eCmdGsmStep;
} TGsmStatus;

void vGsmTask(void* pvParameters);

void stepGsmOff(TGsmStatus*);
void stepGsmLowOff(TGsmStatus*);
void stepGsmStart(TGsmStatus*);
void stepGsmSettingStart(TGsmStatus*);
void stepGsmInit(TGsmStatus*);
void stepGsmJammingDetection(TGsmStatus*);
void stepGsmSim(TGsmStatus*);
void stepGsmSimPin(TGsmStatus*);
void stepGsmConfig(TGsmStatus*);
void stepGsmMessageEvent(TGsmStatus*);
void stepGsmEndMessageEvent(TGsmStatus*);
void stepGsmCellularNetwork(TGsmStatus*);
void stepGsmCheckSMS(TGsmStatus*);
void stepGsmProfileGprsConnect(TGsmStatus*);
void stepGsmProfileGprsSocket(TGsmStatus*);
void stepGsmProfileGprsActivate(TGsmStatus*);
void stepGsmProfileGprsDeactivate(TGsmStatus*);
void stepGsmProfileGprsSendDataInit(TGsmStatus*);
void stepGsmProfileGprsAcknowDataInit(TGsmStatus*);
void stepDeactivateCheckSmsActivate(TGsmStatus*);
void stepGsmSwitchData(TGsmStatus*);
void stepGsmSendAckC(TGsmStatus*);
void stepCheckSMS(TGsmStatus*);
void stepWaitFlashDataReady(TGsmStatus*);
void stepFindMeDataReady(TGsmStatus*);
void stepGsmSleepWaitGps(TGsmStatus*);
void stepGsmSleepWaitAccelStop(TGsmStatus*);
void stepGsmProfileFindMeSendDataRealtime(TGsmStatus*);
void stepGsmProfileFindMeSendDataTrack(TGsmStatus*);
void stepGsmSendAnsOkData(TGsmStatus*);
void stepGsmSendFailC(TGsmStatus*);
void stepGsmProfileGprsSendDataStatusFirmware(TGsmStatus*);
void stepGsmProfileGprsAcknowData(TGsmStatus*);
void stepSynchronizationServerTimeRequest(TGsmStatus*);
void stepGsmProfileGprsSendDataInfoDevice(TGsmStatus*);
void stepGsmProfileGprsSendDataGpioDevice(TGsmStatus*);

void stepGsmProfileHttpSocket(TGsmStatus*);
void stepGsmProfileHttpDownload(TGsmStatus*);
void stepGsmProfileCheckFirmware(TGsmStatus*);

void stepGsmProfileGprsActivateSocketAGps(TGsmStatus*);
void stepGsmProfileGprsSendDataInitAGps(TGsmStatus*);
void stepGsmProfileGprsDownloadDataServerAGps(TGsmStatus*);
void stepGsmProfileGprsDeactivateAGps(TGsmStatus*);

void stepConnectError(TGsmStatus*);
void stepSleep(TGsmStatus*);
void stepRestartNow(TGsmStatus*);

void stepDeviceTest(TGsmStatus*);

void setGsmStep(GSM_STEP eCmdGsmstep);
int GSM_State(VALUE prm);
GSM_STEP GetGsmStep(void);
uint32_t GsmLowPwr1(void);
uint32_t GsmLowPwr2(void);
_Bool GetSleepGsm(void);

void SetFlagAlarmWkp(TYPE_MODE_DEV mode);
void resetConnectError(void);

#endif