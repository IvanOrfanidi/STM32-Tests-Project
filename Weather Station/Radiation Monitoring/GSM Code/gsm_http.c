
#include "includes.h"
#include "gsm_http.h"

_Bool exit_ftp_or_http;

int iLenDownloadFirwmware;
uint32_t AddressFlashFirmware;
/*
89.223.42.88/lfirm.php?f=1458917674&i=353437069574298&b=0 - пример запроса
f=1458917674 - имя прошивки
i=2142343335435 - IMEI модуля
b=0 - с какого байта читать файл
*/
uint8_t profile_http_read(PROF_CONNECT_SERVER usProf)
{
   int ret;
   char strTemp[64];
   _Bool bErrHttp;

   static char strBackNameServer[SIZE_SERV_FTP];

   /* Проверим базовая ли это прошивка, если нет, то качаем её */
   /*
   if(g_stEepConfig.stFirmware.uiNameNewFirmware == g_stRam.stFirmware.uiNameBaseFirmware)  {
      DPS("\r-FIRMWARE BASE OK-\r");
      return FIRMWARE_OK;
   }
   */
   for (ret = 0; ret < SIZE_SERV_FTP; ret++)
   {
      if (strBackNameServer[ret] != g_stEepConfig.stFirmware.strNameFtpSer[ret])
      {
         DP_GSM("NEW FIRMWARE FILE\r");
         //Отчищаем 32 субсектора внешней flash для записи прошивки.
         EraseFirmwareFlash();
         iLenDownloadFirwmware = 0;
         AddressFlashFirmware = ADDR_EXT_FLASH_NEW_FIRMWARE;
         for (uint8_t i = 0; i < SIZE_SERV_FTP; i++)
         {
            strBackNameServer[i] = g_stEepConfig.stFirmware.strNameFtpSer[i];
         }
         break;
      }
   }

#ifdef GSM_MODULE_SIM800
   mc("at+sapbr=3,1,\"CONTYPE\",\"GPRS\"",
      5,
      MC_COUNT);   //задаем настройки подключения (3- Set bearer parameters; 1- Bearer is connected)
   osDelay(10);
   profile_gprs_connect(1);
   mc("at+sapbr=1,1", 5, 1);   //Открыть несущую (1- Open bearer; 1- Bearer is connected)
   osDelay(10);
   mc("at+httpinit", 5, 1);   //Инициализировать HTTP.
   osDelay(10);
   mc("at+httppara=\"CID\",1", 5, 1);   //Получить индификатор http(обязательный параметр).
   osDelay(10);
#endif

   int ucMaxCountDownFirmErr = 0;
   while (iLenDownloadFirwmware <
          (END_ADDRESS_CODE_INT_FLASH - START_ADDRESS_CODE_INT_FLASH))   //Основной цикл закачки прошивки
   {
      profile_deactivate(usProf);   // Close HTTP Connect
      osDelay(SLEEP_MS_1000);
#ifdef GSM_MODULE_BGS2
      sprintf(g_asCmdBuf, "at^siss=%d,address,\"http://", usProf);
      strcat(g_asCmdBuf, g_stEepConfig.stFirmware.strNameFtpSer);   // 89.223.42.88/lfirm.php?f=1458917674
      strcat(g_asCmdBuf, "&i=");
      strcat(g_asCmdBuf, g_stRam.stGsm.strStrIMEI);   //&i=353437069574298
      sprintf(strTemp, "&b=%i", iLenDownloadFirwmware);
      strcat(g_asCmdBuf, strTemp);   //&b=0
      strcat(g_asCmdBuf, "\"");
      mc(g_asCmdBuf, 5, 1);
#else
      sprintf(g_asCmdBuf,
              "at+httppara=\"URL\",\"http://%s",
              g_stEepConfig.stFirmware.strNameFtpSer);   // 89.223.42.88/lfirm.php?f=1458917674
      strcat(g_asCmdBuf, "&i=");

      strcat(g_asCmdBuf, g_stRam.stGsm.strStrIMEI);   //&i=353437069574298
      sprintf(strTemp, "&b=%i", iLenDownloadFirwmware);
      strcat(g_asCmdBuf, strTemp);   //&b=0
      strcat(g_asCmdBuf, "\"");
      mc(g_asCmdBuf, 5, 1);

#endif
      uint8_t ucMaxCountConnectErr = 0;
      /* Open HTTP Socket */
      while (1)
      {
#ifdef GSM_MODULE_BGS2
         ret = mc_siso(usProf, 1, 10);   // Open HTTP Connect
#else
         ret = mc("at+httpaction=0", 5, 1);   ////Запросить данные методом GET.
         osDelay(SLEEP_MS_1000);
#endif
         if (ret == RET_OK)
         {
            DP_GSM("__HTTP CONNECT\r");
            ucMaxCountConnectErr = 0;
            break;
         }
         else
         {
            osDelay(SLEEP_MS_10000);
            ucMaxCountConnectErr++;
         }
         if (ucMaxCountConnectErr > MAX_ERR_CONNECT_HTTP)
         {
            DPS("-ERR HTTP CONNECT-\r");
            return ERR_CONNECT_FTP_OR_HTTP;   //устройство не может подключиться к HTTP серверу
         }
      }

#ifdef GSM_MODULE_SIM800
      uint8_t ucMaxCountDownSesErr = 0;
      //Необходимый параметр чтения размера файла на HTTP.
      int iTotalSizeDataHTTP = 0;
      while (iTotalSizeDataHTTP <= 0)
      {
         iTotalSizeDataHTTP = socket_read(usProf, g_asTempBuf, sizeof(g_asTempBuf));
         osDelay(SLEEP_MS_1000);
         ucMaxCountDownSesErr++;
         if (ucMaxCountDownSesErr > MAX_ERR_DOWN_SESSION)
         {
            DPS("-ERR HTTP SESSION CONNECT-\r");
            // return ERR_CONNECT_FTP_OR_HTTP;
            iTotalSizeDataHTTP = (END_ADDRESS_CODE_INT_FLASH - START_ADDRESS_CODE_INT_FLASH + 1);
            break;
         }
      }
      if (iTotalSizeDataHTTP != (END_ADDRESS_CODE_INT_FLASH - START_ADDRESS_CODE_INT_FLASH + 1))
      {
         DPS("-ERR FIRM SIZE-\r");
         return ERR_FIRMWARE_SIZE;
      }
#endif

      int http_buf_data_len = 0;
      while (iLenDownloadFirwmware < (END_ADDRESS_CODE_INT_FLASH - START_ADDRESS_CODE_INT_FLASH))
      {
         /* Качаем и пишем одну страницу 256 */
         memset(g_asTempBuf, 0, sizeof(g_asTempBuf));
         bErrHttp = 0;
         int iLen = 0;
         uint8_t ucTimeoutDownload = 0;
         while (http_buf_data_len != SIZE_RECORD_EXT_FLASH / 2)
         {
#ifdef GSM_MODULE_BGS2
            iLen =
               ftp_or_http_read(usProf, &g_asTempBuf[http_buf_data_len], SIZE_RECORD_EXT_FLASH / 2 - http_buf_data_len);
#else
            sprintf(
               g_asCmdBuf, "at+httpread=%i,%i", iLenDownloadFirwmware, SIZE_RECORD_EXT_FLASH / 2 - http_buf_data_len);
            mc_send(g_asCmdBuf, NULL, NULL);
            iLen = socket_read(usProf, g_asTempBuf + http_buf_data_len, SIZE_RECORD_EXT_FLASH / 2 - http_buf_data_len);
#endif
            if (iLen > 0)
               http_buf_data_len += iLen;
            if (iLen < 0)
            {   // Error HTTP
               switch (iLen)
               {
               case -401:
                  return ERR_FIRMWARE_UNAUTHORIZED_401;
               case -426:
                  return ERR_FIRMWARE_UPGRADE_REQ_426;
               case -400:
                  return ERR_FIRMWARE_BAD_REQ_400;
               case -404:
                  return ERR_FIRMWARE_NOT_FOUND_404;
               }
            }
            if (exit_ftp_or_http == 1)
            {
               bErrHttp = 1;   //Закачка файла прервалась, файл закончился:(
               break;
            }
            if (iLen == NULL)
            {
               osDelay(5000);
               ucTimeoutDownload++;
               if (ucTimeoutDownload >= MAX_ERR_DOWNLOAD_PAGE)
               {
                  bErrHttp = 1;   //Закачка файла прервалась, слишком много ошибок:(
                  break;
               }
            }
         }
         http_buf_data_len = 0;

         if (bErrHttp == NULL && exit_ftp_or_http == NULL)
         {
            //Пишем страницу flash
            FLASH_Take_Semaphore();
            EXT_FLASH_Write((uint8_t*)&g_asTempBuf[0], AddressFlashFirmware, SIZE_RECORD_EXT_FLASH / 2);
            FLASH_Give_Semaphore();
            AddressFlashFirmware += SIZE_RECORD_EXT_FLASH / 2;
            iLenDownloadFirwmware += SIZE_RECORD_EXT_FLASH / 2;
         }
         else
         {
            ucMaxCountDownFirmErr++;
            DP_GSM("\r__HTTP ERR DOWNLOAD\r");
            DP_GSM("__HTTP RECONNECT\r");
            if (ucMaxCountDownFirmErr >= MAX_ERR_DOWN_FIRM_HTTP)
            {
               // Fatal Download Firmware ;(
               DPS("\r-FIRM ERR DOWN-\r");
               return ERR_FIRMWARE_SIZE;
            }
            break;   //Выходим в основной цикл по Err.
         }
         DP_GSM("N:%i\r", iLenDownloadFirwmware);
      }
   }
#ifdef GSM_MODULE_BGS2
   int iLen = ftp_or_http_read(usProf, g_asTempBuf, 256);
   if ((exit_ftp_or_http != 1) && (iLen > -1))
   {   //Файл реально закончился?
      //Что за хрень с файлом, он что бесконечный?
      DPS("\r-ERR FILE NOT END-\r");
      int iTimeoutDownload = 0;
      while (exit_ftp_or_http != 1)
      {
         osDelay(100);
         ftp_or_http_read(usProf, g_asTempBuf, 256);
         iTimeoutDownload++;
         if ((iTimeoutDownload > 3) || (iLen < 0))
         {
            DPS("\r-FIRM ERR SIZE-\r");
            // return ERR_FIRMWARE_SIZE;
            break;
         }
      }
   }
   else
   {
      DPS("\rFILE END\r");
   }
#endif
   profile_deactivate(usProf);   // Close HTTP Connect
   iLenDownloadFirwmware = 0;
   for (uint8_t i = 0; i < SIZE_SERV_FTP; i++)
   {
      strBackNameServer[i] = 0;
   }
   DPS("\r-FIRMWARE DOWNLOAD OK-\r");
   return FIRMWARE_OK;
}

uint8_t check_firmware(void)
{
   uint16_t crc;
   uint8_t count_err_flash = MAX_COUNT_ERR_FLASH;
   uint32_t StartAddress;

   crc = 0xFFFF;
   memset(g_asTempBuf, 0, sizeof(g_asTempBuf));
   while (crc)
   {
      StartAddress = ADDR_EXT_FLASH_NEW_FIRMWARE;
      uint16_t CountPageData = COUNT_FLASH_PAGE;
      crc = 0xFFFF;
      /* Reload IWDG counter */
      IWDG_ReloadCounter();

      if (!(count_err_flash))
      {
         DPS("-ERR FIRMWARE FLASH-\r");
         return ERR_FIRMWARE_FLASH;   // Выходим не обновив прошивку по ошибке записи во внешнию flash.
      }

      while (CountPageData)
      {
         IWDG_ReloadCounter();
         FLASH_Take_Semaphore();
         EXT_FLASH_Read((uint8_t*)g_asTempBuf, StartAddress, SIZE_RECORD_EXT_FLASH);
         FLASH_Give_Semaphore();
         crc = CRC16_FILL((uint8_t*)g_asTempBuf, SIZE_RECORD_EXT_FLASH, crc);

         StartAddress += SIZE_RECORD_EXT_FLASH;
         CountPageData--;
      }
      count_err_flash--;
   }

   DPS("\r-FIRMWARE OK-\r");
   return FIRMWARE_OK;
}