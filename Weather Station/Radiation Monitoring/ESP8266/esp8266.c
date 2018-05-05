
#include "includes.h"
#include "esp8266.h"

#define TIMEOUT_ANS 60000

#define COUNT_ANS 10
#define SIZE_MSG_RES 20

const char a_strESP_Responses[COUNT_ANS][SIZE_MSG_RES] = {
   "SEND OK",   // 0
   "ERROR",   // 1
   "CONNECT",   // 2
   "+RST",   // 3
   "+IPD",   // 4
   "CLOSE",   // 5
   ">",   // 6
   "OK",   // 7
   "busy p",   // 8
   "WIFI GOT IP"   // 9
};

ANS_INFO cmd_send(const char* pCmd, ESP_INFO* pstEsp, char* pRet, int iSize)
{
   int iLenAns;
   uint16_t iMaxLen = (uint16_t)iSize;
   DPS("esp [->] ");
   DPD(pCmd, strlen(pCmd));
   esp_write(pCmd, strlen(pCmd));
   esp_write(CR_LF, strlen(CR_LF));

   iLenAns = esp_read(pRet, iMaxLen);

   if (iLenAns < 0)
      return ERR_SIZE_BUF;

   return esp_parser(pstEsp, pCmd, pRet, iSize);
}

ANS_INFO esp_parser(ESP_INFO* pstEsp, const char* pEcho, char* pAns, int iSize)
{
   char strMsgAns[SIZE_MSG_RES];
   ANS_INFO eFindAns;
   char* pStartAns = pAns;
   int iLen_pEcho = strlen(pEcho);
   int iLen_pAns = strlen(pAns);

   pstEsp->eResType = WAIT_DATA;
   pstEsp->ucCountParamRes = 0;
   pstEsp->bEcho = FALSE;
   pstEsp->iValueParamRes[0] = 0;
   pstEsp->iValueParamRes[1] = 0;

   if (iLen_pEcho)
   {
      char* pFindECHO = strstr(pAns, pEcho);
      if (pFindECHO > 0)
      {
         pstEsp->bEcho = TRUE;
         int i = 0;
         for (; i < iLen_pAns; i++)
         {
            if (pAns[0] == CR || pAns[0] == LF)
            {
               break;
            }
            pAns++;
         }
         for (; i < iLen_pAns; i++)
         {
            if (pAns[0] != CR && pAns[0] != LF)
            {
               break;
            }
            pAns++;
         }
      }
      else
      {
         pstEsp->bEcho = FALSE;
      }
   }
   else
   {
      pstEsp->bEcho = FALSE;
   }

   _Bool bRespTrue = 0;
   for (uint8_t ucIndex = 0; ucIndex < COUNT_ANS; ucIndex++)
   {
      for (uint8_t i = 0; i < SIZE_MSG_RES; i++)
      {
         strMsgAns[i] = a_strESP_Responses[ucIndex][i];
      }
      // Find Responses
      char* pFindAns = strstr(pAns, strMsgAns);
      if (pFindAns > 0)
      {
         bRespTrue = 1;
         while (pFindAns[0] != LF)
         {
            pFindAns++;
         }
         pFindAns++;
         eFindAns = (ANS_INFO)ucIndex;
         break;
      }
   }
   if (bRespTrue)
   {
      pstEsp->eResType = eFindAns;
   }
   else
   {
      pstEsp->eResType = ERR_UNK_ANS;
   }
   int i = 0;
   switch (eFindAns)
   {
   case RET_OK:
   case RET_SEND_OK:
   case RET_PROMPT:
      return eFindAns;
      break;
   case RET_CONNECT:
   case RET_CLOSE:
      for (i = 0; i < iLen_pAns; i++)
      {
         if (pAns[i] == ',')
         {
            pAns[i] = 0;
            break;
         }
      }
      pstEsp->ucCountParamRes = 1;
      pstEsp->iValueParamRes[pstEsp->ucCountParamRes - 1] = atoi(pAns);
      pAns[i] = ',';
      break;
   case RET_IPD:
      for (i = 0; i < 30; i++)
      {
         if (pAns[0] == ',')
         {
            pstEsp->ucCountParamRes++;
            pAns++;
            pstEsp->iValueParamRes[pstEsp->ucCountParamRes - 1] = atoi(pAns);
            if (pstEsp->ucCountParamRes == 2)
            {
               break;
            }
         }
         pAns++;
      }
      for (int i = 0; i < 30; i++)
      {
         if (pAns[0] == ':')
         {
            pAns++;
            break;
         }
         pAns++;
      }
      int iLen = pStartAns - pAns;
      memmove(&pAns[iLen], &pAns[0], pstEsp->iValueParamRes[1]);
      break;
   }

   return eFindAns;
}

void esp_write(const char* pBuf, uint16_t size)
{
   USART_Write(UART_ESP, pBuf, size);
}

int esp_read(char* pBuf, uint16_t iMaxLen)
{
   uint32_t uiTimeoutAns = TIMEOUT_ANS;
   uint16_t usLen;
   uint16_t usLenBack;

   while (uiTimeoutAns)
   {
      usLen = USART_Rx_Len(UART_ESP);

      if (usLenBack == usLen)
      {
         uiTimeoutAns--;
      }
      else
      {
         uiTimeoutAns = TIMEOUT_ANS;
         usLenBack = usLen;
      }
   }

   if (usLen > iMaxLen)
   {
      return -1;
   }
   if (usLen > 0)
   {
      USART_Read(UART_ESP, pBuf, usLen);
      DPS("esp [<-] ");
      DPD(pBuf, usLen);
   }
   return (int)usLen;
}