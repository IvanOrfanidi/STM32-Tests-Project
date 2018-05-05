
#ifndef _GSM_GENERAL_H_
#define _GSM_GENERAL_H_

#include "includes.h"
#include "gsm_parser.h"
#include "eeprom.h"
#include "sram.h"

typedef enum GSM_STEP
{
   NO_STEP = 0,
   GSM_OFF = 100,
   GSM_OFF_LOW_PWR = 200,
   GSM_START = 300,
   GSM_ON = 400,
   GSM_SETTING_START = 500,
   GSM_CONFIG = 600,
   GSM_JAMMING_DETECTED = 700,
   GSM_SIM = 800,
   GSM_SIM_PIN = 900,
   GSM_ADDITIONAL_CONFIG = 1000,
   GSM_CELLULAR_NETWORK = 1100,
   GSM_CHECK_SMS = 1200,
   GSM_PROFILE_GPRS_CONNECT = 1300,
   GSM_SWITCH_PROTOCOL = 1400,
   GSM_PROFILE_GPRS_SOCKET = 1500,
   GSM_PROFILE_GPRS_ACTIVATE = 1600,
   GSM_PROFILE_GPRS_DEACTIVATE = 1700,
   GSM_DEACTIVATE_CHECK_SMS_ACTIVATE = 1800,
   GSM_PROFILE_GPRS_SEND_DATA_INIT = 1900,
   GSM_PROFILE_GPRS_ACKNOW_DATA_INIT = 2000,
   GSM_SWITCH_DATA = 2100,
   GSM_PROFILE_GPRS_SEND_C_ACK = 2200,
   CHECK_SMS = 2300,
   WAIT_FLASH_DATA_READY = 2400,
   GSM_PROFILE_GPRS_ANS_CMD_SERVER_OK = 2500,
   GSM_PROFILE_GPRS_ANS_CMD_SERVER_ERR = 2600,
   GSM_PROFILE_GPRS_ACKNOW_DATA = 3000,

   FIND_ME_DATA_READY = 5000,
   GSM_PROFILE_GPRS_FM_SEND_DATA_REALTIME = 5100,
   GSM_PROFILE_GPRS_FM_SEND_DATA_TRACK = 5200,
   GSM_SLEEP_WAIT_GPS = 5300,

   GSM_PROFILE_GPRS_SEND_FIRMWARE_STATUS = 10000,
   SYNCHRONIZATION_SERVER_TIME_REQUEST = 10100,
   GSM_PROFILE_GPRS_SEND_DATA_STATUS_DEVICE = 10200,
   GSM_PROFILE_GPRS_SEND_DATA_ACCEL_STATUS = 10300,
   GSM_PROFILE_GPRS_SEND_INFO_DEVICE = 10400,
   GSM_PROFILE_GPRS_SEND_DATA = 10500,
   GSM_PROFILE_GPRS_SEND_GPIO_DEVICE = 10600,

   GSM_PROFILE_HTTP_SOCKET = 30000,
   GSM_PROFILE_HTTP_DOWNLOAD = 30100,
   GSM_PROFILE_CHECK_FIRMWARE = 30200,

   SLEEP_DOWN = 49000,
   RESTART_NOW = 50000,

} GSM_STEP;

typedef __packed struct
{
   GSM_STEP eGsmStep;
   uint32_t uiGsmStepDelay;
   GSM_STEP eNextAckGsmStep;
   GSM_STEP eBackGsmStep;
   GSM_STEP eCmdGsmStep;
} TGsmStatus;

void vGsmTask(void* pvParameters);

void StepGsmOff(TGsmStatus* pGsmStatus);
void StepGsmLowOff(TGsmStatus* pGsmStatus);
void StepGsmStart(TGsmStatus* pGsmStatus);
void StepGsmSettingStart(TGsmStatus* pGsmStatus);
void StepGsmConfig(TGsmStatus* pGsmStatus);
void StepGsmJammingDetection(TGsmStatus* pGsmStatus);
void StepGsmSim(TGsmStatus* pGsmStatus);
void StepGsmSimPin(TGsmStatus* pGsmStatus);
void StepGsmAdditionalConfig(TGsmStatus* pGsmStatus);
void StepGsmCellularNetwork(TGsmStatus* pGsmStatus);
void StepGsmCheckSMS(TGsmStatus* pGsmStatus);
void StepGsmProfileGprsConnect(TGsmStatus* pGsmStatus);
void StepGsmProfileGprsSocket(TGsmStatus* pGsmStatus);
void StepGsmProfileGprsActivate(TGsmStatus* pGsmStatus);
void StepGsmProfileGprsDeactivate(TGsmStatus* pGsmStatus);
void StepGsmProfileGprsSendDataInit(TGsmStatus* pGsmStatus);
void StepGsmProfileGprsAcknowDataInit(TGsmStatus* pGsmStatus);
void StepDeactivateCheckSmsActivate(TGsmStatus* pGsmStatus);
void StepGsmSwitchData(TGsmStatus* pGsmStatus);
void StepGsmSendAckC(TGsmStatus* pGsmStatus);
void StepCheckSMS(TGsmStatus* pGsmStatus);
void StepWaitFlashDataReady(TGsmStatus* pGsmStatus);
void StepFindMeDataReady(TGsmStatus* pGsmStatus);
void StepGsmSleepWaitGps(TGsmStatus* pGsmStatus);
void StepGsmProfileFindMeSendDataRealtime(TGsmStatus* pGsmStatus);
void StepGsmProfileFindMeSendDataTrack(TGsmStatus* pGsmStatus);
void StepGsmSendAnsOkData(TGsmStatus* pGsmStatus);
void StepGsmSendFailC(TGsmStatus* pGsmStatus);
void StepGsmProfileGprsSendDataStatusFirmware(TGsmStatus* pGsmStatus);
void StepGsmProfileGprsAcknowData(TGsmStatus* pGsmStatus);
void StepSynchronizationServerTimeRequest(TGsmStatus* pGsmStatus);
void StepGsmProfileGprsSendDataStatusDevice(TGsmStatus* pGsmStatus);
void StepGsmProfileGprsSendDataAccelStatus(TGsmStatus* pGsmStatus);
void StepGsmProfileGprsSendDataInfoDevice(TGsmStatus* pGsmStatus);
void StepGsmProfileGprsSendDataGpioDevice(TGsmStatus* pGsmStatus);
void StepGsmProfileGprsSendData(TGsmStatus* pGsmStatus);

void StepGsmProfileHttpSocket(TGsmStatus* pGsmStatus);
void StepGsmProfileHttpDownload(TGsmStatus* pGsmStatus);
void StepGsmProfileCheckFirmware(TGsmStatus* pGsmStatus);

void StepSleep(void);
void StepRestartNow(void);

void SetGsmStep(GSM_STEP eCmdGsmStep);
int GSM_State(VALUE prm);
GSM_STEP GetGsmStep(void);
uint32_t GsmLowPwr1(void);
uint32_t GsmLowPwr2(void);

#endif