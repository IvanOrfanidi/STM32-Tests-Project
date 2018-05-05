#ifndef _GSM_GENERAL_H_
#define _GSM_GENERAL_H_

typedef enum GSM_STEP
{
   NO_STEP = 0,
   GSM_OFF = 100,
   GSM_OFF_LOW_PWR = 200,
   GSM_START = 300,
   GSM_ON = 400,
   GSM_SET_BAUDRATE = 500,
   GSM_FIRST_CONFIG = 600,
   GSM_SIM = 800,
   GSM_SIM_PIN = 900,
   GSM_SECOND_CONFIG = 1000,
   GSM_CELLULAR_NETWORK = 1100,
   GSM_CHECK_SMS = 1200,
   GSM_PROFILE_GPRS_CONNECT = 1300,
   GSM_PROFILE_GPRS_SOCKET = 1500,
   GSM_PROFILE_GPRS_ACTIVATE = 1600,
   GSM_PROFILE_GPRS_DEACTIVATE = 1700,
   CHECK_SMS = 2300,

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
void StepGsmStart(TGsmStatus* pGsmStatus);
void StepGsmSetBaudrate(TGsmStatus* pGsmStatus);
void StepGsmFirstConfig(TGsmStatus* pGsmStatus);
void StepGsmSim(TGsmStatus* pGsmStatus);
void StepGsmSimPin(TGsmStatus* pGsmStatus);
void StepGsmSecondConfig(TGsmStatus* pGsmStatus);
void StepGsmCellularNetwork(TGsmStatus* pGsmStatus);

#endif