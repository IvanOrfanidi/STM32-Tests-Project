#include "http_general.h"
#include "includes.h"

char aucRxBufferESP[ESP_RX_BUFFER_SIZE];
char aucTxBufferESP[ESP_RX_BUFFER_SIZE];

// char ptEspRet[ESP_RX_BUFFER_SIZE];

#define MAX_IP_ADDRESS 20
#define MAX_MAC_ADDRESS 20
#define MAX_API_KEY 20
#define MAX_NAME_SSID_LEN 20
#define MAX_PASS_LEN 30

typedef enum
{
   OPEN = 0,
   WEP = 1,
   WPA_PSK = 2,
   WPA2_PSK = 3,
   WPA_WPA2_PSK = 4
} TYPE_SECURITY_NETWORK;

#define FIRST_IP_ADDRESS "narodmon.ru"
#define FIRST_TCP_PORT 8283

#define MAC_ADDRESS "18:FE:34:F2:70:D3"
/******************************************************/
#define SECOND_IP_ADDRESS "184.106.153.149"
#define SECOND_TCP_PORT 80

#define WRITE_API_KEY "HL40H242MD8DSH4F"
#define READ_API_KEY "TPN5GNWUUIB05S53"
/******************************************************/
#define SSID_NETWORK "HomeInternet"
#define PASSWORD_NETWORK "OmMaNiPadmeHum"

#define CHANNEL 10

typedef struct
{
   char strIpAddressServer[MAX_IP_ADDRESS];
   char strWriteApiKey[MAX_API_KEY];
   char strReadApiKey[MAX_API_KEY];
   char strMacAddress[MAX_MAC_ADDRESS];
   char srtSsid[MAX_NAME_SSID_LEN];
   char srtPasswd[MAX_PASS_LEN];
   uint8_t ucTypeSecurity;
   uint8_t ucChannel;
   uint16_t usTcpPort;
} TEspNetConfig;

int InitEspAP(char* ptEspRet, uint16_t Len, TEspNetConfig* pEspNetConfig);
int InitEspSTA(char* ptEspRet, uint16_t Len, TEspNetConfig* pEspNetConfig);
int ReadDataWiFi(char* ptEspRet, uint16_t Len);
int SendDataWiFi(int iProf, char* ptEspRet, uint16_t Len);
int SocketClose(int iProf);
int SendDataArchiveFirstServer(int iProf,
                               char* ptEspRet,
                               uint16_t Len,
                               TEspNetConfig* pEspNetConfig,
                               TServer_Data* pServerData);
int SendDataArchiveSecondServer(int iProf,
                                char* ptEspRet,
                                uint16_t Len,
                                TEspNetConfig* pEspNetConfig,
                                TServer_Data* pServerData);
void Delay(portTickType eDelayMS);

void vWifiEspTask(void* pvParameters)
{
   int iNumConnect = 0;
   int iLen;
   TServer_Data stServerData;
   TServer_Data stServerDataForSend;
   memset(&stServerDataForSend, 0, sizeof(stServerDataForSend));

   int iRet;
   uint8_t ucSendDataServerFail =
      0;   //На какой сервер неполучилось отправить данные. Используется, что бы переслать данные
   uint8_t ucMaxErrSendDataFirstServ = 0;
   uint8_t ucMaxErrSendDataSecondServ = 0;

   TEspNetConfig stEspNetConfig;
   //Настраиваем параметры соединения. Эти настройки не меняются, а меняется только адрес сервера и его порт.
   strcpy(stEspNetConfig.strMacAddress, MAC_ADDRESS);
   strcpy(stEspNetConfig.strWriteApiKey, WRITE_API_KEY);
   strcpy(stEspNetConfig.strReadApiKey, READ_API_KEY);
   strcpy(stEspNetConfig.srtSsid, SSID_NETWORK);
   strcpy(stEspNetConfig.srtPasswd, PASSWORD_NETWORK);
   stEspNetConfig.ucTypeSecurity = OPEN;
   stEspNetConfig.ucChannel = CHANNEL;

   xQueueServerData = xQueueCreate(sizeof(uint8_t), sizeof(TServer_Data));
   vQueueAddToRegistry(xQueueServerData, "xQueueServerData");

   osDelay(SLEEP_MS_1000);
   InitUSART(UART_ESP, ESP_BAUDRATE);
   InitDMA(UART_ESP);

   while (1)
   {
      /* Обработка WEB клиента на два сервера */
      if ((xQueueReceive(xQueueServerData, &stServerData, (portTickType)1000)) && (xQueueServerData != 0))
      {
         _Bool fResetErr = 0;
         DRF_DISABLE;
         iNumConnect = 2;
         stServerDataForSend.fDose = stServerData.fDose;
         stServerDataForSend.iCPM = stServerData.iCPM;
         stServerDataForSend.fIntTemperatur = stServerData.fIntTemperatur;
         stServerDataForSend.stTimeRecords = stServerData.stTimeRecords;

         if (stServerData.fDoseDay)
         {
            stServerDataForSend.fDoseDay = stServerData.fDoseDay;
         }
#if 0  
      //Настройка первой конфигурации
      if(ucSendDataServerFail != 2)
      {
        strcpy(stEspNetConfig.strIpAddressServer, FIRST_IP_ADDRESS);
        stEspNetConfig.usTcpPort = FIRST_TCP_PORT;
        
        InitEspSTA(aucTxBufferESP, sizeof(aucTxBufferESP), &stEspNetConfig);
        
        iRet = SendDataArchiveFirstServer(iNumConnect, aucTxBufferESP, iLen, &stEspNetConfig, &stServerDataForSend);
        if( (iRet != RET_SEND_OK) && (iRet != RET_OK) ) {
          xQueueSendToFront(xQueueServerData, &stServerDataForSend, (portTickType) 1000);  //Если не получилось отправить сообщение, заново поставим его в очередь.
          ucSendDataServerFail = 1;
          ucMaxErrSendDataFirstServ++;
        }
        else {
           fResetErr = 1;       //Сбросили ошибку
           ucMaxErrSendDataFirstServ = 0;
        }
        osDelay(SLEEP_MS_5000);
        SocketClose(iNumConnect);
      }
      //******************************
      
      if(!(ucSendDataServerFail)) {
        fResetErr = 0;
      }
#endif

         //Настройка второй конфигурации
         if (ucSendDataServerFail != 1)
         {
            osDelay(SLEEP_MS_1000);
            strcpy(stEspNetConfig.strIpAddressServer, SECOND_IP_ADDRESS);
            stEspNetConfig.usTcpPort = SECOND_TCP_PORT;

            InitEspSTA(aucTxBufferESP, sizeof(aucTxBufferESP), &stEspNetConfig);

            iRet =
               SendDataArchiveSecondServer(iNumConnect, aucTxBufferESP, iLen, &stEspNetConfig, &stServerDataForSend);
            if ((iRet != RET_SEND_OK) && (iRet != RET_OK))
            {
               xQueueSendToFront(
                  xQueueServerData,
                  &stServerDataForSend,
                  (portTickType)1000);   //Если не получилось отправить сообщение, заново поставим его в очередь.
               ucSendDataServerFail = 2;
               ucMaxErrSendDataSecondServ++;
            }
            else
            {
               fResetErr = 1;   //Сбросили ошибку
               ucMaxErrSendDataSecondServ = 0;
            }
            osDelay(SLEEP_MS_5000);
            SocketClose(iNumConnect);
         }
         //****************************

         if (((ucSendDataServerFail) && (fResetErr)) || (ucMaxErrSendDataFirstServ >= MAX_ERR_SEND_FIRST_SERVER) ||
             (ucMaxErrSendDataSecondServ >= MAX_ERR_SEND_SECOND_SERVER))
         {   //Если имелись ошибки передачи.
            if (ucSendDataServerFail == 1)
            {
               xQueueSendToFront(
                  xQueueServerData,
                  &stServerDataForSend,
                  (portTickType)1000);   //Если не получилось отправить сообщение, заново поставим его в очередь.
               ucSendDataServerFail = 2;   // и отправим на второй сервер.
            }
            else
            {
               xQueueReceive(xQueueServerData, &stServerData, (portTickType)0);
               ucSendDataServerFail = 0;
               DPS("\r\nD_ESP CP PD OFF\r\n");
               ESP_CP_PD_OFF;
            }
            ucMaxErrSendDataFirstServ = 0;
            ucMaxErrSendDataSecondServ = 0;
         }
         DPS("\r\nD_ESP CP PD OFF\r\n");
         ESP_CP_PD_OFF;
         iNumConnect = 0;
      }
   }
}

int ReadDataWiFi(char* ptEspRet, uint16_t Len)
{
   ESP_INFO stEspInfo;
   int iLenAns;

   iLenAns = esp_read(ptEspRet, Len);
   if ((iLenAns <= 0) || (iLenAns > Len))
      return 0;

   esp_parser(&stEspInfo, "", ptEspRet, Len);

   if ((stEspInfo.eResType == RET_IPD) || (stEspInfo.eResType == RET_CONNECT))
   {
      if (stEspInfo.eResType == RET_CONNECT)
      {
         iLenAns = esp_read(ptEspRet, Len);
         if (iLenAns > 0)
         {
            esp_parser(&stEspInfo, "", ptEspRet, Len);
         }
      }
      memcpy(ptEspRet, ptEspRet, sizeof(ptEspRet));
      return (stEspInfo.iValueParamRes[0] + 1);   //Возвращает номер открытого коннекта.
   }

   if (stEspInfo.eResType == RET_CLOSE)
   {
      return (stEspInfo.iValueParamRes[0] + 1) * -1;   //Возвращает номер закрытого коннекта.
   }

   if (stEspInfo.eResType == RET_OK)
   {
      return 0;
   }

   return ERR_UNK_ANS;
}

int SocketClose(int iProf)
{
   ESP_INFO stEspInfo;
   memset(&stEspInfo, 0, sizeof(stEspInfo));
   char strTempCmd[64];
   int iNumSocket;
   if (iProf <= 0)
   {
      return ERR_SOCKET;
   }
   iNumSocket = iProf - 1;

   sprintf(strTempCmd, "%s%i", AT_CIPCLOSE, iNumSocket);
   cmd_send(strTempCmd, &stEspInfo, strTempCmd, strlen(strTempCmd));
   if (stEspInfo.eResType != RET_OK)
   {
      return ERR_NO_PROMPT;
   }
   return RET_OK;
}

int SendDataWiFi(int iProf, char* ptEspRet, uint16_t Len)
{
   ESP_INFO stEspInfo;
   memset(&stEspInfo, 0, sizeof(stEspInfo));
   char strTempCmd[128];
   int iLenAns;
   int iNumSocket;

   if (iProf <= 0)
   {
      return ERR_SOCKET;
   }

   iNumSocket = iProf - 1;

   sprintf(strTempCmd, "%s%i,%i", AT_CIPSEND, iNumSocket, Len);
   cmd_send(strTempCmd, &stEspInfo, strTempCmd, strlen(strTempCmd));

   for (uint8_t i = 0; i < 10; i++)
   {
      iLenAns = esp_read(strTempCmd, sizeof(strTempCmd));
      if (iLenAns > 0)
         break;
      osDelay(SLEEP_MS_10);
   }
   esp_parser(&stEspInfo, "", strTempCmd, sizeof(strTempCmd));

   if (stEspInfo.eResType != RET_PROMPT)
   {
      return ERR_SEND_DATA;
   }

   DPS("esp [->] ");
   /* Передаем данные */
   for (uint16_t i = 0; i < Len; i++)
   {
      esp_write(&ptEspRet[i], 1);
      DPD(&ptEspRet[i], 1);
   }

   for (uint8_t i = 0; i < 50; i++)
   {
      iLenAns = esp_read(strTempCmd, sizeof(strTempCmd));
      if (iLenAns > 0)
         break;
      osDelay(SLEEP_MS_10);
   }

   if (iLenAns <= 0)
   {
      //   return ERR_TIMEOUT;
   }

   esp_parser(&stEspInfo, "", strTempCmd, sizeof(strTempCmd));

   if (stEspInfo.eResType == RET_SEND_OK)
   {
      return RET_SEND_OK;
   }
   else
   {
      memset(strTempCmd, 0, sizeof(strTempCmd));
      osDelay(SLEEP_MS_50);
      for (uint8_t i = 0; i < 50; i++)
      {
         iLenAns = esp_read(strTempCmd, sizeof(strTempCmd));
         if (iLenAns > 0)
            break;
         osDelay(SLEEP_MS_10);
      }
      esp_parser(&stEspInfo, "", strTempCmd, sizeof(strTempCmd));
      if (stEspInfo.eResType == RET_SEND_OK)
      {
         return RET_SEND_OK;
      }
   }
   return ERR_SEND_DATA;
}

#define ESP_CMD_DELAY 800

int InitEspSTA(char* ptEspRet, uint16_t Len, TEspNetConfig* pEspNetConfig)
{
   uint8_t ucTimeout = 0;
   int iLenAns;
   ESP_INFO stEspInfo;
   memset(&stEspInfo, 0, sizeof(stEspInfo));
   char strCmd[64];

   DPS("\r\nD_ESP CP PD ON\r\n");
   ESP_CP_PD_ON;
   Delay(SLEEP_MS_10000);

   cmd_send(AT_RST, &stEspInfo, ptEspRet, Len);
   Delay(SLEEP_MS_3000);

   cmd_send(AT, &stEspInfo, ptEspRet, Len);
   if (stEspInfo.bEcho == FALSE)
   {
      cmd_send(ATE1, &stEspInfo, ptEspRet, Len);
      osDelay(ESP_CMD_DELAY);
   }

   // Set mode Wifi STA.
   cmd_send(ESP_MODE_STA, &stEspInfo, ptEspRet, Len);
   Delay(SLEEP_MS_5000);

   // Set config WiFi.
   sprintf(strCmd, "%s\"%s\",\"%s\"", AT_CWJAP, pEspNetConfig->srtSsid, pEspNetConfig->srtPasswd);
   memset(ptEspRet, 0, Len);
   cmd_send(strCmd, &stEspInfo, ptEspRet, Len);
   Delay(SLEEP_MS_10000);

   ucTimeout = 0;
   while (ucTimeout < 10)
   {
      iLenAns = esp_read(ptEspRet, Len);
      if (iLenAns > 0)
      {
         esp_parser(&stEspInfo, "", ptEspRet, Len);
      }
      if ((stEspInfo.eResType == RET_OK) || (stEspInfo.eResType == RET_busy) || (stEspInfo.eResType == RET_CONNECT))
      {
         break;
      }
      ucTimeout++;
      Delay(SLEEP_MS_1000);
   }
   if (stEspInfo.eResType == RET_busy)
   {
      return -1;
   }

   // Set mode Wifi STA.
   cmd_send(AT_CIFSR, &stEspInfo, ptEspRet, Len);
   if (stEspInfo.eResType != RET_OK)
   {
      return -1;
   }
   Delay(ESP_CMD_DELAY);

   cmd_send(ESP_MULTIPE_CONNECTION, &stEspInfo, ptEspRet, Len);
   if (stEspInfo.eResType != RET_OK)
   {
      return -1;
   }
   Delay(ESP_CMD_DELAY);

   // Start web server
   sprintf(strCmd, "%s,%i", ESP_START_SERVER, pEspNetConfig->usTcpPort);
   cmd_send(strCmd, &stEspInfo, ptEspRet, Len);
   if (stEspInfo.eResType != RET_OK)
   {
      return -1;
   }
   return 0;
}

int SendDataArchiveFirstServer(int iProf,
                               char* ptEspRet,
                               uint16_t Len,
                               TEspNetConfig* pEspNetConfig,
                               TServer_Data* pServerData)
{
   ESP_INFO stEspInfo;
   memset(&stEspInfo, 0, sizeof(stEspInfo));
   char strTempCmd[128];
   int iNumSocket;
   int iLenAns;

   if (iProf <= 0)
   {
      return ERR_SOCKET;
   }

   iNumSocket = iProf - 1;

   sprintf(strTempCmd,
           "%s%i,\"TCP\",\"%s\",%i",
           AT_CIPSTART,
           iNumSocket,
           pEspNetConfig->strIpAddressServer,
           pEspNetConfig->usTcpPort);
   cmd_send(strTempCmd, &stEspInfo, strTempCmd, strlen(strTempCmd));
   Delay(TIME_OPEN_SOCKET);

   sprintf(ptEspRet, "#%s#Radiation Monitoring", pEspNetConfig->strMacAddress);
   strcat(ptEspRet, "\n");

   if (pServerData->fIntTemperatur > -40)
   {
      sprintf(strTempCmd, "#T2#%.01f#DS18B20\n", pServerData->fIntTemperatur);
      strcat(ptEspRet, strTempCmd);
   }

   if (pServerData->fDose != 0)
   {
      sprintf(strTempCmd, "#R1#%.00f#SBM-20 gamma\n", pServerData->fDose);
      strcat(ptEspRet, strTempCmd);
   }

   strcat(ptEspRet, "##");

   for (uint8_t i = 0; i < 2; i++)
   {
      Len = strlen(ptEspRet);
      memset(strTempCmd, 0, sizeof(strTempCmd));
      sprintf(strTempCmd, "%s%i,%i", AT_CIPSEND, iNumSocket, Len);
      cmd_send(strTempCmd, &stEspInfo, strTempCmd, strlen(strTempCmd));
      memset(strTempCmd, 0, sizeof(strTempCmd));
      iLenAns = esp_read(strTempCmd, sizeof(strTempCmd));
      if ((iLenAns > 0) && (!(strstr(strTempCmd, "busy") > 0)))
      {
         break;
      }
      Delay(SLEEP_MS_10);
   }

   if (!(strstr(strTempCmd, ">") > 0))
   {
      return ERR_SEND_DATA;
      // return ERR_SEND_DATA;
   }

   DPS("esp [->] ");
   Delay(SLEEP_MS_10);

   /* Передаем данные */
   for (uint16_t i = 0; i < Len; i++)
   {
      esp_write(&ptEspRet[i], 1);
      DPD(&ptEspRet[i], 1);
   }

   for (uint8_t i = 0; i < 50; i++)
   {
      iLenAns = esp_read(strTempCmd, sizeof(strTempCmd));
      if (iLenAns > 0)
         break;
      Delay(SLEEP_MS_10);
   }

   if (iLenAns <= 0)
      return RET_OK;   // ERR_TIMEOUT;

   esp_parser(&stEspInfo, "", strTempCmd, sizeof(strTempCmd));

   if (stEspInfo.eResType == RET_SEND_OK)
   {
      return RET_SEND_OK;
   }
   else
   {
      memset(strTempCmd, 0, sizeof(strTempCmd));
      Delay(SLEEP_MS_50);
      for (uint8_t i = 0; i < 50; i++)
      {
         iLenAns = esp_read(strTempCmd, sizeof(strTempCmd));
         if (iLenAns > 0)
            break;
         osDelay(SLEEP_MS_10);
      }
      esp_parser(&stEspInfo, "", strTempCmd, sizeof(strTempCmd));
      if (stEspInfo.eResType == RET_SEND_OK)
      {
         return RET_SEND_OK;
      }
   }
   return ERR_SEND_DATA;
}

int SendDataArchiveSecondServer(int iProf,
                                char* ptEspRet,
                                uint16_t Len,
                                TEspNetConfig* pEspNetConfig,
                                TServer_Data* pServerData)
{
   ESP_INFO stEspInfo;
   memset(&stEspInfo, 0, sizeof(stEspInfo));
   char strTempCmd[128];
   int iNumSocket;
   int iLenAns;

   if (iProf <= 0)
   {
      return ERR_SOCKET;
   }

   iNumSocket = iProf - 1;

   sprintf(strTempCmd,
           "%s%i,\"TCP\",\"%s\",%i",
           AT_CIPSTART,
           iNumSocket,
           pEspNetConfig->strIpAddressServer,
           pEspNetConfig->usTcpPort);
   cmd_send(strTempCmd, &stEspInfo, strTempCmd, strlen(strTempCmd));
   Delay(TIME_OPEN_SOCKET);

   sprintf(ptEspRet, "GET /update?key=%s", pEspNetConfig->strWriteApiKey);

   if (pServerData->fDose != 0)
   {
      sprintf(strTempCmd, "&field1=%.00f", pServerData->fDose);
      strcat(ptEspRet, strTempCmd);
   }

   if (pServerData->fIntTemperatur > -40)
   {
      sprintf(strTempCmd, "&field2=%.01f", pServerData->fIntTemperatur);
      strcat(ptEspRet, strTempCmd);
   }

   if (pServerData->iCPM != 0)
   {
      sprintf(strTempCmd, "&field3=%i", pServerData->iCPM);
      strcat(ptEspRet, strTempCmd);
   }

   strcat(ptEspRet, "\r\n");

   for (uint8_t i = 0; i < 2; i++)
   {
      Len = strlen(ptEspRet);
      memset(strTempCmd, 0, sizeof(strTempCmd));
      sprintf(strTempCmd, "%s%i,%i", AT_CIPSEND, iNumSocket, Len);
      cmd_send(strTempCmd, &stEspInfo, strTempCmd, strlen(strTempCmd));
      memset(strTempCmd, 0, sizeof(strTempCmd));
      iLenAns = esp_read(strTempCmd, sizeof(strTempCmd));
      if ((iLenAns > 0) && (!(strstr(strTempCmd, "busy") > 0)))
      {
         break;
      }
      Delay(SLEEP_MS_10);
   }

   if (!(strstr(strTempCmd, ">") > 0))
   {
      // return ERR_SEND_DATA;
   }

   DPS("esp [->] ");
   Delay(SLEEP_MS_10);
   /* Передаем данные */
   for (uint16_t i = 0; i < Len; i++)
   {
      esp_write(&ptEspRet[i], 1);
      DPD(&ptEspRet[i], 1);
   }

   for (uint8_t i = 0; i < 50; i++)
   {
      iLenAns = esp_read(strTempCmd, sizeof(strTempCmd));
      if (iLenAns > 0)
         break;
      Delay(SLEEP_MS_10);
   }

   if (iLenAns <= 0)
      return RET_OK;   // ERR_TIMEOUT;

   esp_parser(&stEspInfo, "", strTempCmd, sizeof(strTempCmd));

   if (stEspInfo.eResType == RET_SEND_OK)
   {
      return RET_SEND_OK;
   }
   else
   {
      memset(strTempCmd, 0, sizeof(strTempCmd));
      osDelay(SLEEP_MS_50);
      for (uint8_t i = 0; i < 50; i++)
      {
         iLenAns = esp_read(strTempCmd, sizeof(strTempCmd));
         if (iLenAns > 0)
            break;
         osDelay(SLEEP_MS_10);
      }
      esp_parser(&stEspInfo, "", strTempCmd, sizeof(strTempCmd));
      if (stEspInfo.eResType == RET_SEND_OK)
      {
         return RET_SEND_OK;
      }
   }
   return ERR_SEND_DATA;
}

void Delay(portTickType eDelayMS)
{
   portTickType xLastWakeTimerDelay;
   xLastWakeTimerDelay = xTaskGetTickCount();
   vTaskDelayUntil(&xLastWakeTimerDelay, (eDelayMS / portTICK_RATE_MS));
}
