
#include "includes.h"
#include "gsm_code.h"

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
      case GSM_START:
         StepGsmStart(&stGsmStatus);
         break;
      case GSM_SET_BAUDRATE:
         StepGsmSetBaudrate(&stGsmStatus);
         break;
      case GSM_FIRST_CONFIG:
         StepGsmFirstConfig(&stGsmStatus);
         break;
      case GSM_SIM:
         StepGsmSim(&stGsmStatus);
         break;
      case GSM_SIM_PIN:
         StepGsmSimPin(&stGsmStatus);
         break;
      case GSM_SECOND_CONFIG:
         StepGsmSecondConfig(&stGsmStatus);
         break;
      case GSM_CELLULAR_NETWORK:
         StepGsmCellularNetwork(&stGsmStatus);
         break;
      }
   }
}

//Выключение модема.
void StepGsmOff(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   GSM_State(OFF);

   pGsmStatus->uiGsmStepDelay = SLEEP_MS_3000;
   pGsmStatus->eGsmStep = GSM_START;
}

//Включение модема.
void StepGsmStart(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;

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
      pGsmStatus->eGsmStep = GSM_SET_BAUDRATE;
      return;
   }

   pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;
   pGsmStatus->eGsmStep = GSM_FIRST_CONFIG;
}

//Настройки скорости UART GSM модема.
void StepGsmSetBaudrate(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   RET_INFO eStatusRet = modem_setting_start();
   if (eStatusRet == RET_OK)
   {
      pGsmStatus->uiGsmStepDelay = SLEEP_MS_5000;
      pGsmStatus->eGsmStep = GSM_FIRST_CONFIG;
   }
   else
   {
      pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;
      pGsmStatus->eGsmStep = GSM_OFF;
   }
}

//Стартовые настройки, могут выполнятся без SIM карты.
void StepGsmFirstConfig(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   RET_INFO eStatusRet = modem_precmd();
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;

   GSM_INFO stGsm;
   memset(&stGsm, 0, sizeof(GSM_INFO));
   INIT_GSM_INFO(stGsm);
   /*
   DP_GSM("D_FIND GSM LBS, PLEASE WAIT\r\n");
   //Проверим LLS, что бы было не менее одной баз. станции GSM.
   get_opInfo(AT_SMOND, &stGsm, 10);     //"at^smond"
   DP_GSM("D_LBS COUNT: %d\r\n", stGsm.count);
   */
   if (eStatusRet == RET_OK)
   {
      pGsmStatus->eGsmStep = GSM_SIM;
   }
   else
   {
      pGsmStatus->eGsmStep = GSM_OFF;
   }
}

void StepGsmSim(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   mc("at+cfun=0", 4, MC_COUNT);
   osDelay(1000);
   mc("at+cfun=1", 4, MC_COUNT);
   osDelay(3000);

   M_INFO eSimStatus = SimCardInit();
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_100;
   if (eSimStatus == M_SIM_READY)
   {
      pGsmStatus->eGsmStep = GSM_SECOND_CONFIG;
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
         pGsmStatus->eGsmStep = GSM_OFF;
      }

      if (eSimStatus == M_OTHER)
      {
         g_stRam.stSim.eRegStatus = SIM_ERROR;
         pGsmStatus->eGsmStep = GSM_OFF;
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

void StepGsmSecondConfig(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_1000;

   RET_INFO eStatusRet = modem_second_precmd();
   if (eStatusRet != RET_OK)
   {
      pGsmStatus->eGsmStep = GSM_OFF;
   }
   else
   {
      g_stRam.stSim.eRegStatus = FIND_NET;
      pGsmStatus->eGsmStep = GSM_CELLULAR_NETWORK;
   }
}

void StepGsmCellularNetwork(TGsmStatus* pGsmStatus)
{
   pGsmStatus->eBackGsmStep = pGsmStatus->eGsmStep;
   RET_INFO eStatusRet = GetCellularNetwork();
   pGsmStatus->uiGsmStepDelay = SLEEP_MS_3000;
   if ((eStatusRet == RET_HOME_NET) || (eStatusRet == RET_ROAMING_NET))
   {
      g_stRam.stSim.eRegStatus = HOME_NET;
      // pGsmStatus->eGsmStep = GSM_CHECK_SMS;
      return;
   }
   pGsmStatus->eGsmStep = GSM_OFF;
}