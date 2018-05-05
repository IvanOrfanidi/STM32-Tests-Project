#include "drf.h"
#include "includes.h"

#define MAX_COUNT 5

const uint8_t aucSpase[4] = { 11, 13, 15, 17 };

_Bool gbDrfEnable = 0;

void UsartSetConfig(_Bool bSetConf, uint32_t uiBaudRate);
int parser_drf(char* pstrResBuff, TDrf_Settings* pstDrfSettings);

void SetRadioEnable(_Bool bEnabDrf)
{
   if (bEnabDrf)
   {
      DRF_ENABLE;
      gbDrfEnable = 1;
   }
   else
   {
      DRF_DISABLE;
      gbDrfEnable = 0;
   }
}

void InitGpioDRF(void)
{
   GPIO_InitTypeDef GPIO_InitStructure;

   /* DRF ENABLE PIN */
   RCC_APB2PeriphClockCmd(DRF_EN_GPIO_CLK, ENABLE);
   GPIO_InitStructure.GPIO_Pin = DRF_EN_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
   GPIO_Init(DRF_EN_PORT, &GPIO_InitStructure);
   DRF_DISABLE;

   /* DRF SETTING PIN */
   RCC_APB2PeriphClockCmd(DRF_SET_GPIO_CLK, ENABLE);
   GPIO_InitStructure.GPIO_Pin = DRF_SET_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
   GPIO_Init(DRF_SET_PORT, &GPIO_InitStructure);
   DRF_SET_DISABLE;

#if _AUX_INTERRUPT_USE == 1
   /* DRF AUX PIN */
   RCC_APB2PeriphClockCmd(DRF_AUX_GPIO_CLK, ENABLE);
   GPIO_InitStructure.GPIO_Pin = DRF_AUX_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
   GPIO_Init(DRF_AUX_PORT, &GPIO_InitStructure);

   EXTI_InitTypeDef EXTI_InitStructure;
   NVIC_InitTypeDef NVIC_InitStructure;
   /* Enable AFIO clock */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

   /* Configure EXTI0 line */
   EXTI_InitStructure.EXTI_Line = EXT_AUX_Line;
   EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
   EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
   EXTI_InitStructure.EXTI_LineCmd = ENABLE;
   EXTI_Init(&EXTI_InitStructure);

   /* Enable and set EXTI0 Interrupt to the lowest priority */
   NVIC_InitStructure.NVIC_IRQChannel = EXT_AUX_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0xFF;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0xFF;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
#endif
}

int GetConfigDRF(TDrf_Settings* pstDrfSettings, uint32_t uiBaudRate)
{
   char strResBuff[RX_BUFFER_SIZE1];
   _Bool bDrfEnable = 0;
   memset(strResBuff, 0xFF, sizeof(strResBuff));
   uint8_t ucCountRead;
   char* pFindPARA;

   /*Проверяем включено ли радио и если да, то выключаем его, что бы включить далее.
     Это нужно для правильного чтения конфигурации */
   if (gbDrfEnable)
   {
      bDrfEnable = 1;
      SetRadioEnable(0);
      _delay_ms(500);
   }

   UsartSetConfig(1, uiBaudRate);
   SetRadioEnable(1);

   ucCountRead = 0;
   while (1)
   {
      DRF_SET_ENABLE;
      _delay_ms(1000);
      USART_Write(UART_DRF, "RD\r\n", strlen("RD\r\n"));
      _delay_ms(500);
      DRF_SET_DISABLE;
      _delay_ms(2000);
      USART_Read(UART_DRF, strResBuff, sizeof(strResBuff));
      uint8_t i;
      for (i = 0; i < sizeof(strResBuff); i++)
      {
         if (strResBuff[i] != 0)
         {
            break;
         }
      }
      pFindPARA = strstr(&strResBuff[i], "PARA ");
      if (pFindPARA > 0)
      {
         break;
      }

      if (ucCountRead > MAX_COUNT)
      {
         DRF_SET_DISABLE;
         UsartSetConfig(0, uiBaudRate);
         if (bDrfEnable == 0)
         {
            SetRadioEnable(0);
         }
         USART_Read(UART_DRF, strResBuff, sizeof(strResBuff));
         return ERR_DRF_TIMEOUT;
      }
      ucCountRead++;
      _delay_ms(2000);
      USART_Read(UART_DRF, strResBuff, sizeof(strResBuff));
      USART_Read(UART_DRF, strResBuff, sizeof(strResBuff));
   }
   DRF_SET_DISABLE;
   UsartSetConfig(0, uiBaudRate);

   if (bDrfEnable == 0)
   {
      SetRadioEnable(0);
   }

   if (parser_drf(pFindPARA, pstDrfSettings))
   {
      USART_Read(UART_DRF, strResBuff, sizeof(strResBuff));
      return ERR_DRF_RESP;
   }
   USART_Read(UART_DRF, strResBuff, sizeof(strResBuff));
   return OK_DRF_RESP;
}

void UsartSetConfig(_Bool bSetConf, uint32_t uiBaudRate)
{
   if (bSetConf)
   {
      DeInitUSART(UART_DRF);
      DeInitDMA(UART_DRF);
      InitUSART(UART_DRF, DRF_SETTING_BAUDRATE);
      InitDMA(UART_DRF);
   }
   else
   {
      if (uiBaudRate == 0)
         return;
      DeInitUSART(UART_DRF);
      DeInitDMA(UART_DRF);
      InitUSART(UART_DRF, uiBaudRate);
      InitDMA(UART_DRF);
   }
}

int SetConfigDRF(TDrf_Settings* pstDrfSettings, uint32_t uiBaudRate)
{
   TDrf_Settings stDrfSettingsCrc;
   uint8_t ucCountRead;
   char strResBuff[21];
   memset(strResBuff, 0, sizeof(strResBuff));
   char stWriteCmd[20];
   memset(stWriteCmd, 0, sizeof(stWriteCmd));
   _Bool bDrfEnable = 0;

   sprintf((char*)stWriteCmd, "WR %i", pstDrfSettings->uiFreq);
   int n = strlen(stWriteCmd);
   stWriteCmd[n++] = 0x20;
   stWriteCmd[n++] = pstDrfSettings->eDRfsk;
   stWriteCmd[n++] = 0x20;
   stWriteCmd[n++] = pstDrfSettings->ecPout;
   stWriteCmd[n++] = 0x20;
   stWriteCmd[n++] = pstDrfSettings->eDRin;
   stWriteCmd[n++] = 0x20;
   stWriteCmd[n++] = pstDrfSettings->eParity;
   stWriteCmd[n++] = '\r';
   stWriteCmd[n++] = '\n';

   if (gbDrfEnable)
   {
      bDrfEnable = 1;
      SetRadioEnable(0);
      _delay_ms(500);
   }

   UsartSetConfig(1, uiBaudRate);
   SetRadioEnable(1);

   ucCountRead = 0;

   while (1)
   {
      DRF_SET_ENABLE;
      USART_Write(UART_DRF, stWriteCmd, strlen(stWriteCmd));
      _delay_ms(200);
      DRF_SET_DISABLE;
      _delay_ms(1000);
      USART_Read(UART_DRF, strResBuff, sizeof(strResBuff));
      if (strlen(strResBuff))
      {
         break;
      }
      if (ucCountRead > MAX_COUNT)
      {
         DRF_SET_DISABLE;
         UsartSetConfig(0, uiBaudRate);
         if (bDrfEnable == 0)
         {
            SetRadioEnable(0);
         }
         USART_Read(UART_DRF, strResBuff, sizeof(strResBuff));
         return ERR_DRF_TIMEOUT;
      }
      ucCountRead++;
   }
   DRF_SET_DISABLE;
   UsartSetConfig(0, uiBaudRate);

   if (bDrfEnable == 0)
   {
      SetRadioEnable(0);
   }

   if (parser_drf(strResBuff, &stDrfSettingsCrc))
   {
      USART_Read(UART_DRF, strResBuff, sizeof(strResBuff));
      return ERR_DRF_RESP;
   }

   if ((stDrfSettingsCrc.ecPout != pstDrfSettings->ecPout) || (stDrfSettingsCrc.eDRfsk != pstDrfSettings->eDRfsk) ||
       (stDrfSettingsCrc.eDRin != pstDrfSettings->eDRin) || (stDrfSettingsCrc.eParity != pstDrfSettings->eParity) ||
       (stDrfSettingsCrc.uiFreq != pstDrfSettings->uiFreq))
   {
      USART_Read(UART_DRF, strResBuff, sizeof(strResBuff));
      return ERR_DRF_RESP;
   }

   USART_Read(UART_DRF, strResBuff, sizeof(strResBuff));
   return OK_DRF_RESP;
}

int parser_drf(char* pstrResBuff, TDrf_Settings* pstDrfSettings)
{
   char* pFind;
   char strTemp[7];
   memset(strTemp, 0, sizeof(strTemp));

   // printf(pstrResBuff);
   pFind = strstr(pstrResBuff, "PARA ");
   if (!(pFind > 0))
   {
      return -1;
   }
   for (uint8_t i = 0; i < sizeof(aucSpase); i++)
   {
      if (pstrResBuff[aucSpase[i]] != 0x20)
      {
         return -1;
      }
   }
   if ((pstrResBuff[19] != '\r') || (pstrResBuff[20] != '\n'))
   {
      return -1;
   }

   for (uint8_t i = 0; i < 6; i++)
   {
      strTemp[i] = pstrResBuff[i + strlen("PARA ")];
   }
   int uiFreq = atoi(strTemp);
   if ((uiFreq > MAX_FREQUENCY) || (uiFreq < MIN_FREQUENCY))
   {
      return -1;
   }
   pstDrfSettings->uiFreq = uiFreq;

   if ((pstrResBuff[12] > DRfsk_19200bps) || (pstrResBuff[12] < DRfsk_2400bps))
   {
      return -1;
   }
   pstDrfSettings->eDRfsk = (DRF_DR_FSK)pstrResBuff[12];

   if ((pstrResBuff[14] > Pout_20dBm) || (pstrResBuff[14] < Pout_11dBm))
   {
      return -1;
   }
   pstDrfSettings->ecPout = (DRF_RF_POWER)pstrResBuff[14];

   if ((pstrResBuff[16] > DRin_57600bps) || (pstrResBuff[16] < DRin_1200bps))
   {
      return -1;
   }
   pstDrfSettings->eDRin = (DRF_DR_IN)pstrResBuff[16];

   if ((pstrResBuff[18] > ODD_PARITY) || (pstrResBuff[18] < NO_PARITY))
   {
      return -1;
   }
   pstDrfSettings->eParity = (DRF_PARITY)pstrResBuff[18];
   return 0;
}

#if _AUX_INTERRUPT_USE == 1
void EXTI0_IRQHandler(void)
{
   if (EXTI_GetITStatus(EXT_AUX_Line) != RESET)
   {
      EXTI_ClearITPendingBit(EXT_AUX_Line);
   }
}
#endif