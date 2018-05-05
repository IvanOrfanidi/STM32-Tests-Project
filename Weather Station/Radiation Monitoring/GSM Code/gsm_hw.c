
#include "includes.h"
#include "gsm_hw.h"

#define TIMEOUT_POWER_GSM 5000000

uint32_t TimeStamp;
uint32_t delay_rtos;
uint32_t uiCurSec = 0;

void GSM_Reference(uint8_t prm)
{
   GPIO_InitTypeDef GPIO_InitStructure;
   GPIO_InitStructure.GPIO_Pin = GSM_REF_PIN;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_Init(GSM_REF_PORT, &GPIO_InitStructure);

   if (prm)
   {
      DPS("-D_GSM PWR ON-\r\n");
      GPIO_HIGH(GSM_REF_PORT, GSM_REF_PIN);
   }
   else
   {
      DPS("-D_GSM PWR OFF-\r\n");
      GPIO_LOW(GSM_REF_PORT, GSM_REF_PIN);
   }
}

int GSM_State(_Bool prm)
{
   GPIO_InitTypeDef GPIO_InitStructure;

   uint32_t timeout_power_gsm = TIMEOUT_POWER_GSM;

   if (prm == ON)
   {
      memset(g_aucTxBufferUSART1, 0, DBG_TX_BUFFER_SIZE);
      memset(g_aucRxBufferUSART1, 0, DBG_RX_BUFFER_SIZE);
      memset(g_aucTxBufferUSART2, 0, GSM_TX_BUFFER_SIZE);
      memset(g_aucRxBufferUSART2, 0, GSM_RX_BUFFER_SIZE);

      GSM_Reference(prm);
      osDelay(100);
      /*
      GPIO_InitStructure.GPIO_Pin = GSM_DTR_PIN;
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
      GPIO_Init(GSM_DTR_PORT, &GPIO_InitStructure);
      GPIO_LOW(GSM_DTR_PORT, GSM_DTR_PIN);
      */
      GPIO_InitStructure.GPIO_Pin = GSM_PWR_KEY_PIN;
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
      GPIO_Init(GSM_PWR_KEY_PORT, &GPIO_InitStructure);

      InitUSART(UART_GSM, GSM_BAUDRATE);
      InitDMA(UART_GSM);
      DPS("-GSM ON-\r");

#ifdef GSM_MODULE_SIM800
      osDelay(1000);
      // PWR_KEY_PULL_UP;
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
         return 0;
      }
   }

   if (prm == OFF)
   {
      GPIO_InitStructure.GPIO_Pin = GSM_PWR_KEY_PIN;
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
      GPIO_Init(GSM_PWR_KEY_PORT, &GPIO_InitStructure);

      GPIO_InitStructure.GPIO_Pin = GSM_DTR_PIN;
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
      GPIO_Init(GSM_DTR_PORT, &GPIO_InitStructure);

      modem_off();
      DPS("-GSM OFF-\r");
      DeInitUSART(UART_GSM);
      GSM_Reference(prm);

      while ((GSM_STATUS_ON) && (timeout_power_gsm))
      {
         timeout_power_gsm--;
         IWDG_ReloadCounter(); /* Reload IWDG counter */
      }

      if (timeout_power_gsm)
      {
         return 0;
      }
   }

   return 1;
}

int isEndWaitPeriod(const uint32_t wait)
{
   if (delay_rtos != wait)
   {
      delay_rtos = wait;
   }

   if (TimeStamp >= delay_rtos)
   {
      delay_rtos = 0;
      TimeStamp = 0;
      return 1;
   }

   TimeStamp++;

   portTickType xLastWakeTimerDelay;
   xLastWakeTimerDelay = xTaskGetTickCount();
   vTaskDelayUntil(&xLastWakeTimerDelay, (SLEEP_MS_100 / portTICK_RATE_MS));
   return 0;
}