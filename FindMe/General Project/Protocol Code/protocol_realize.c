

#include "includes.h"
#include "protocol_realize.h"

int ParseByteValue(uint8_t* pIn, int index, uint16_t Len);
int ParseDoubleByteValue(uint8_t* pIn, int index, uint16_t Len);
int ParseWordValue(uint8_t* pIn, int index, uint16_t Len);
int ParseStringValue(uint8_t* pIn, int index, uint16_t Len);

tBACK_DATA_PERIPH stBackDataPeriph;
extern const char US_DEV_VER[2];
extern const char US_HW_VER[2];
extern RESET_STATUS_DEVICE GetFlagsResetDevice(_Bool reset_status_device);

/* Формируем основную обертку пакетов */
int FrameGeneralBuild(const DATA_FRAME* stTypePack, const char* pInp, uint16_t LenInp, char* pOut)
{
   uint16_t n = 0;
   uint16_t head_size = 0;
   uint16_t crc16 = 0;
   uint64_t ulSerialNumber = stTypePack->ulSerialNumber;
   uint8_t* p = (uint8_t*)&ulSerialNumber;

   pOut[n++] = PROTOCOL_ID;
   pOut[n++] = PROTOCOL_VERSION;

   n++;   // Здесь должна быть длина пакета,
   n++;   // заполним её в конце.

   // Подставляем серийный номер IMEI //
   pOut[n + head_size++] = *p;
   pOut[n + head_size++] = *(p + 1);
   pOut[n + head_size++] = *(p + 2);
   pOut[n + head_size++] = *(p + 3);
   pOut[n + head_size++] = *(p + 4);
   pOut[n + head_size++] = *(p + 5);
   pOut[n + head_size++] = *(p + 6);
   //

   // Подставляем тип информационного пакета //
   pOut[n + head_size++] = stTypePack->ucType;
   //

   // Подставляем порядковый номер пакета  //
   pOut[n + head_size++] = (stTypePack->usDevicePaketNumber) & 0xFF;
   pOut[n + head_size++] = (stTypePack->usDevicePaketNumber >> 8) & 0xFF;

   n += 10;
   //
   loop(LenInp)
   {
      pOut[n++] = pInp[i];
   }

   // Подставляем длину пакета до CRC //
   pOut[2] = (n - 4) & 0xFF;
   pOut[3] = ((n - 4) >> 8) & 0xFF;

   crc16 = get_cs16((uint8_t*)pOut, n);

   pOut[n++] = crc16 & 0xFF;
   pOut[n++] = (crc16 >> 8) & 0xFF;

   return n;
}

/* Разбираем ответ от сервера */
int ack_data_parser(const char* pInp, uint16_t LenInp)
{
   uint16_t crc = get_cs16((uint8_t*)pInp, LenInp);   // Check CRC16

   if ((crc) || (LenInp < 11))
   {
      if (crc)
      {
         GSM_DPD("!CRC Pac Fail!\r\n", strlen("!CRC Pac Fail!\r\n"));
      }
      //Не совпала СRC и пакет битый, нужно отправить серверу C_FAIL.
      g_stFrame.usFlagsRetDataServer |= fC_FAIL;
      return fC_FAIL;
   }

   //Запоменаем порядковый номер пакета.
   g_stFrame.usServerPaketNumber = g_aucOutDataFrameBuffer[12];
   int temp = g_aucOutDataFrameBuffer[13];
   g_stFrame.usServerPaketNumber |= temp << 8;

   switch (pInp[11])
   {
   case S_ACK:
      g_stFrame.usFlagsRetDataServer |= fS_ACK;
      break;   //данные приняты
   case S_REQ:
      g_stFrame.usFlagsRetDataServer |= fS_REQ;
      break;   //сервер запрашивает данные
   case S_FAIL:
      g_stFrame.usFlagsRetDataServer |= fS_FAIL;
      break;   //данные не приняты
   case S_ACK_REQ:
      g_stFrame.usFlagsRetDataServer |= fS_ACK_REQ;
      break;   //данные приняты и есть данные для устройства
   case S_DATA:
      g_stFrame.usFlagsRetDataServer |= fS_DATA;
      break;   //данные от сервера
   case S_ACK_DLY:
      g_stFrame.usFlagsRetDataServer |= fS_ACK_DLY;
      break;   //сервера данные приняты и сообщение о перегрузке сервера
   case S_ASK_DATA:
      g_stFrame.usFlagsRetDataServer |= fS_ASK_DATA;
      break;   //сервера данные приняты и список команды от сервера
   case S_FIN:
      g_stFrame.usFlagsRetDataServer |= fS_FIN;
      break;   //сервер завершает сеанс связи
   }

   return (pInp[11]);
}

/* Формируем пакет ответа серверу C_ASK */
int FrameGeneralBuildAckC(uint8_t TypePack, const char* pInp, uint16_t LenInp, char* pOut)
{
   uint16_t n = 0;
   uint16_t head_size = 0;
   uint16_t crc16 = 0;

   pOut[n++] = PROTOCOL_ID;
   pOut[n++] = PROTOCOL_VERSION;

   n++;   // Здесь должна быть длина пакета,
   n++;   // заполним её в конце.

   g_stFrame.ulSerialNumber = GetIMEI();
   // Подставляем серийный номер IMEI //
   uint8_t* p = (uint8_t*)&g_stFrame.ulSerialNumber;

   pOut[n + head_size++] = *p;
   pOut[n + head_size++] = *(p + 1);
   pOut[n + head_size++] = *(p + 2);
   pOut[n + head_size++] = *(p + 3);
   pOut[n + head_size++] = *(p + 4);
   pOut[n + head_size++] = *(p + 5);
   pOut[n + head_size++] = *(p + 6);
   //

   // Подставляем тип информационного пакета //
   pOut[n + head_size++] = TypePack;
   //

   // Подставляем порядковый номер пакета  //
   pOut[n + head_size++] = (g_iFramePaketNumber)&0xFF;
   pOut[n + head_size++] = (g_iFramePaketNumber >> 8) & 0xFF;

   n += 10;
   //
   for (uint16_t i = 0; i < LenInp; i++)
   {
      pOut[n++] = pInp[i];
   }

   // Подставляем длину пакета до CRC //
   pOut[2] = (n - 4) & 0xFF;
   pOut[3] = ((n - 4) >> 8) & 0xFF;

   crc16 = get_cs16((uint8_t*)pOut, n);

   pOut[n++] = crc16 & 0xFF;
   pOut[n++] = (crc16 >> 8) & 0xFF;

   return n;
}

/* Пакет невалидный навигации */
uint32_t frame_build_navigational_not_valid_packet(char* pOut, int OffsetData)
{
   int n = 0;
   uint8_t bitFree = 8;

   pOut += OffsetData;
   pOut[n++] = NAVIGATIONAL_PACKET;

   //Получаем время из RTC
   uint32_t SecRTC = time();

   /* TIME */
   n += bit_packing(pOut + n, SecRTC, &bitFree, 32);

   /* Заполним не валидными координатами(нулями) */
   /* LAT, LONG */
   n += bit_packing(pOut + n, 0, &bitFree, 29);
   n += bit_packing(pOut + n, 0, &bitFree, 29);
   /* COOURSE */
   n += bit_packing(pOut + n, 0, &bitFree, 6);
   /* SPEED */
   pOut[n++] = 0;
   /* SAT */
   n += bit_packing(pOut + n, 0, &bitFree, 5);
   /* HDOP */
   n += bit_packing(pOut + n, 0, &bitFree, 3);

   uint8_t FlagPotocol = 0x40;   // 6 бит – валидность координат 1- не валидные.

   //Ставим флаг движения Акселерометра(только акселерометр, без анализа скорости по GPS).
   if ((AccelState() == ACC_STATE_MOVE) || (g_stRam.stAccel.eAccelState == ACC_STATE_MOVE))
   {
      g_stRam.stAccel.eAccelState = ACC_STATE_STOP;
      FlagPotocol |= 0x08;
   }

   switch (g_stRam.stDevice.eCurPwrStat)
   {
   case POWER_RUN_MODE:
      break;
   case POWER_LOW_PWR1_MODE:
      FlagPotocol |= 0x10;
      break;
   case POWER_LOW_PWR2_MODE:
      FlagPotocol |= 0x20;
      break;
   default:
      FlagPotocol |= 0x30;
   }

   pOut[n++] = FlagPotocol;

   return n;
}

/* Пакет навигации */
int frame_build_navigational_packet_realtime(const GPS_INFO* const pGps, char* pOut)
{
   int n = 0;
   u8 ucHdop;
   uint8_t bitFree = 8;

   pOut[n++] = NAVIGATIONAL_PACKET_REAL_TIME;

   /* TIME */
   n += bit_packing(pOut + n, pGps->time, &bitFree, 32);
   /* LAT, LONG */
   n += bit_packing(pOut + n, (uint32_t)pGps->latitude, &bitFree, 29);
   n += bit_packing(pOut + n, (uint32_t)pGps->longitude, &bitFree, 29);
   /* COOURSE */
   uint16_t course = (uint16_t)(pGps->course / 10);
   n += bit_packing(pOut + n, course, &bitFree, 6);
   /* SPEED */
   // pGps->speed = (float)g_ucTest;
   pOut[n++] = (uint8_t)(pGps->speed);

   /* SAT */
   if (!(pGps->sat))
   {
      n += bit_packing(pOut + n, 5, &bitFree, 5);
   }
   else
   {
      n += bit_packing(pOut + n, pGps->sat, &bitFree, 5);
   }

   /* HDOP */
   if (pGps->hdop <= 1)
      ucHdop = 0;
   if ((pGps->hdop > 1) && (pGps->hdop <= 3))
      ucHdop = 1;
   if ((pGps->hdop > 3) && (pGps->hdop <= 6))
      ucHdop = 2;
   if ((pGps->hdop > 6) && (pGps->hdop <= 8))
      ucHdop = 3;
   if ((pGps->hdop > 8) && (pGps->hdop <= 20))
      ucHdop = 4;
   if (pGps->hdop > 20)
      ucHdop = 5;

   n += bit_packing(pOut + n, ucHdop, &bitFree, 3);

   uint8_t FlagPotocol = 0;

   //Ставим флаг движения Акселерометра(только акселерометр, без анализа скорости по GPS).
   if ((AccelState() == ACC_STATE_MOVE) || (g_stRam.stAccel.eAccelState == ACC_STATE_MOVE))
   {
      g_stRam.stAccel.eAccelState = ACC_STATE_STOP;
      FlagPotocol |= 0x08;
   }

   switch (g_stRam.stDevice.eCurPwrStat)
   {
   case POWER_RUN_MODE:
      break;
   case POWER_LOW_PWR1_MODE:
      FlagPotocol |= 0x10;
      break;
   case POWER_LOW_PWR2_MODE:
      FlagPotocol |= 0x20;
      break;
   default:
      FlagPotocol |= 0x30;
   }

   pOut[n++] = FlagPotocol;

   return n;
}

int frame_build_navigational_packet_track(const GPS_INFO* const pGps, char* pOut)
{
   int n = 0;
   u8 ucHdop;
   uint8_t bitFree = 8;

   pOut[n++] = NAVIGATIONAL_PACKET;

   /* TIME */
   n += bit_packing(pOut + n, pGps->time, &bitFree, 32);
   /* LAT, LONG */
   n += bit_packing(pOut + n, (uint32_t)pGps->latitude, &bitFree, 29);
   n += bit_packing(pOut + n, (uint32_t)pGps->longitude, &bitFree, 29);
   /* COOURSE */
   uint16_t course = (uint16_t)(pGps->course / 10);
   n += bit_packing(pOut + n, course, &bitFree, 6);
   /* SPEED */
   pOut[n++] = (uint8_t)(pGps->speed);

   /* SAT */
   if (!(pGps->sat))
   {
      n += bit_packing(pOut + n, 5, &bitFree, 5);
   }
   else
   {
      n += bit_packing(pOut + n, pGps->sat, &bitFree, 5);
   }

   /* HDOP */
   if (pGps->hdop <= 1)
      ucHdop = 0;
   if ((pGps->hdop > 1) && (pGps->hdop <= 3))
      ucHdop = 1;
   if ((pGps->hdop > 3) && (pGps->hdop <= 6))
      ucHdop = 2;
   if ((pGps->hdop > 6) && (pGps->hdop <= 8))
      ucHdop = 3;
   if ((pGps->hdop > 8) && (pGps->hdop <= 20))
      ucHdop = 4;
   if (pGps->hdop > 20)
      ucHdop = 5;

   n += bit_packing(pOut + n, ucHdop, &bitFree, 3);

   uint8_t FlagPotocol = 0;

   //Ставим флаг движения Акселерометра(только акселерометр, без анализа скорости по GPS).
   if ((AccelState() == ACC_STATE_MOVE) || (g_stRam.stAccel.eAccelState == ACC_STATE_MOVE))
   {
      g_stRam.stAccel.eAccelState = ACC_STATE_STOP;
      FlagPotocol |= 0x08;
   }

   switch (g_stRam.stDevice.eCurPwrStat)
   {
   case POWER_RUN_MODE:
      break;
   case POWER_LOW_PWR1_MODE:
      FlagPotocol |= 0x10;
      break;
   case POWER_LOW_PWR2_MODE:
      FlagPotocol |= 0x20;
      break;
   default:
      FlagPotocol |= 0x30;
   }
   pOut[n++] = FlagPotocol;
   return n;
}

/* Пакет по GPIO */
int GetDataPeripheryPacket(const TPortInputCFG* const pAds, char* pOut, int OffsetData)
{
   int n = OffsetData;
   uint8_t bitFree = 8;

   // Тип пакета
   pOut[n++] = PERIPHERY_PACKET;
   //длину пока не заполняем, в конце заполним
   n++;

   n += bit_packing(pOut + n, pAds->SecADC, &bitFree, 32);

#ifdef FM4
   uint16_t Meas_VIN = (uint16_t)pAds->Meas_VIN;
   if (Meas_VIN)
   {
      if (GetModeDevice() == TRACK_ION)
      {
         if (stBackDataPeriph.Meas_VIN > (Meas_VIN + DELTA_V) || stBackDataPeriph.Meas_VIN < (Meas_VIN - DELTA_V))
         {
            stBackDataPeriph.Meas_VIN = Meas_VIN;
            pOut[n++] = VPWR_1;
            n += bit_packing(pOut + n, Meas_VIN, &bitFree, 16);
         }
      }
      else
      {
         pOut[n++] = VPWR_1;
         n += bit_packing(pOut + n, Meas_VIN, &bitFree, 16);
         stBackDataPeriph.Meas_VIN = 0xFFFF;
      }
   }
#endif

#ifdef FM3
   pOut[n++] = VBAT_1;
   n += bit_packing(pOut + n, (uint16_t)pAds->Meas_VIN, &bitFree, 16);
#endif

   uint8_t ucGsmCsq = GetCSQ();
   if (ucGsmCsq)
   {
      if (GetModeDevice() == TRACK_ION)
      {
         if (stBackDataPeriph.ucGsmCsq != ucGsmCsq)
         {
            stBackDataPeriph.ucGsmCsq = ucGsmCsq;
            pOut[n++] = ID_CSG_GSM;
            pOut[n++] = GetCSQ();
         }
      }
      else
      {
         stBackDataPeriph.ucGsmCsq = 0xFF;
         pOut[n++] = ID_CSG_GSM;
         pOut[n++] = GetCSQ();
      }
   }

#if (TEMPERATURE_ACCEL)
   int8_t cTemperatur = GetTemperatur();
   if (cTemperatur)
   {
      if (GetModeDevice() == TRACK_ION)
      {
         if (stBackDataPeriph.cTemperatur != cTemperatur)
         {
            stBackDataPeriph.cTemperatur = cTemperatur;
            pOut[n++] = ID_TEMPERATUR_ACCSEL;
            pOut[n++] = cTemperatur;
         }
      }
      else
      {
         pOut[n++] = ID_TEMPERATUR_ACCSEL;
         pOut[n++] = cTemperatur;
         stBackDataPeriph.cTemperatur = -127;
      }
   }
#endif

   pOut[OffsetData + 1] = n - OffsetData - 2;
   return n - OffsetData;
}

/* Пакет получения адреса и имени новой прошивки */
GSM_STEP configFirmware(uint8_t* pIn)
{
   uint8_t n = 1;
   uint32_t SizeFirmware = 0;
   int8_t LenTempDataServer = 0;   //Длина произвольных данных

   SizeFirmware = pIn[n++] << 24;
   SizeFirmware |= pIn[n++] << 16;
   SizeFirmware |= pIn[n++] << 8;
   SizeFirmware |= pIn[n++];

   g_stRam.stFirmware.bFirmSendServ = TRUE;   // Ставим флаг что прошивка пришла от сервера.

   if (SizeFirmware == 0)
   {   //Размер прошивки
      return GSM_PROFILE_HTTP_SOCKET;
   }

   LenTempDataServer = pIn[n++];   //Длина логина
   // Проверим длину
   if (LenTempDataServer > SIZE_FTP_USER)
   {
      return GSM_PROFILE_HTTP_SOCKET;
   }
   // Заполняем логин
   volatile char strUserSer[SIZE_FTP_USER] = { 0 };
   for (uint8_t i = 0; i < LenTempDataServer; i++)
   {
      strUserSer[i] = pIn[n++];
   }

   LenTempDataServer = pIn[n++];   //Длина пароля
   if (LenTempDataServer > SIZE_PASW)
   {
      return GSM_PROFILE_HTTP_SOCKET;
   }
   // Заполняем пароль
   volatile char strPswSer[SIZE_PASW] = { 0 };
   for (uint8_t i = 0; i < LenTempDataServer; i++)
   {
      strPswSer[i] = pIn[n++];
   }
   LenTempDataServer = pIn[n++];   // Длина ссылки
   // Заполняем ссылку

   char* pFindTypeServ = NULL;
   pFindTypeServ = strstr((char*)&pIn[n], "http://");
   if (pFindTypeServ)
   {
      LenTempDataServer -= strlen("http://");
      n += strlen("http://");
   }

   if (LenTempDataServer > SIZE_SERV_FTP)
   {
      return GSM_PROFILE_HTTP_SOCKET;
   }
   char strNameSer[SIZE_SERV_FTP] = { 0 };
   for (uint8_t i = 0; i < LenTempDataServer; i++)
   {
      strNameSer[i] = pIn[n++];
   }

   //находим символ '=' для HTTP
   uint8_t sym_n = 0;
   for (uint8_t i = 0; i < sizeof(strNameSer); i++)
   {
      if (strNameSer[i] == '=')
         sym_n = i;
   }
   SetAddrFirmSer(strNameSer);

   if (!(sym_n))
   {   //не нашли символ '/' или '='
      return GSM_PROFILE_HTTP_SOCKET;
   }

   //Сохранить Конфиг
   SetIntNewFirmware(atol(&strNameSer[++sym_n]));
   SaveConfigCMD();

   return GSM_PROFILE_HTTP_SOCKET;
}

/* Пакет результата скаченой прошивки */
int frame_build_received_firmware(char* pOut)
{
   uint8_t n = 0;
   pOut[n++] = RECEIVED_FIRMWARE_PACKET;   // Тип пакета.
   uint32_t uiNameNewFirm = GetIntNewFirmware();   // Имя файла прошивки.
   uint8_t bitFree = 8;
   n += bit_packing(pOut + n, uiNameNewFirm, &bitFree, 32);

   if (GetFlagsStatusFirmware())
   {   // 1-были ошибки при скачивании, 0-нет ошибок.
      DPS("-ERR: FLAG FIRM-\r\n");
   }
   else
   {
      DPS("-OK: FLAG FIRM-\r\n");
   }

   pOut[n++] = GetFlagsStatusFirmware();   //Ставим флаги статуса прошивки.
   return n;
}

/*  Команда смены прошивки */
GSM_STEP changeFirmware(uint8_t* pIn)
{
   SetStatusReset(UPDATE_FIRM_DEVICE);
   SetStatusDeviceReset(GetStatusReset());
   DPS("-UPDATE FIRMWARE-\r\n");
   return RESTART_NOW;
}

/* Пакет аппаратное событие */
int frame_build_hardware_event_packet(char* pOut, int OffsetData)
{
   int n = OffsetData;
   uint8_t bitFree = 8;
   uint8_t FlagPotocol = 0;

   pOut[n++] = HARDWARE_EVENT_PACKET;   // Тип пакета
   //длину пока не заполняем, в конце заполним
   n++;

   //Получаем время из RTC
   uint32_t SecRTC = time();

   /* TIME */
   n += bit_packing(pOut + n, SecRTC, &bitFree, 32);

   /* LAT, LONG */
   GPS_INFO stGpsDataTemp;
   if (!(GetPositionGps(&stGpsDataTemp)))
   {
      memset(&stGpsDataTemp, 0, sizeof(GPS_INFO));
   }
   n += bit_packing(pOut + n, (uint32_t)stGpsDataTemp.latitude, &bitFree, 29);
   n += bit_packing(pOut + n, (uint32_t)stGpsDataTemp.longitude, &bitFree, 29);

   FlagPotocol = 2;

   //Если валидные координаты
   if (!(GetGpsStatus()))
   {
      FlagPotocol |= 1;   // 0-координаты валидны, 1-координаты не валидны
   }

   //Ставим флаг движения Акселерометра(только акселерометр, без анализа скорости по GPS).
   if ((AccelState() == ACC_STATE_MOVE) || (g_stRam.stAccel.eAccelState == ACC_STATE_MOVE))
   {
      g_stRam.stAccel.eAccelState = ACC_STATE_STOP;
      FlagPotocol |= 2;   // 0-нет движения, 1-есть движение
   }
   n += bit_packing(pOut + n, FlagPotocol, &bitFree, 6);

   /* Нажата тревожная кнопка */
   pOut[n++] = ALARM_BUTTON;
   pOut[n++] = (GetFlagsResetDevice(0) == BUTTON_RESET);

   /* Вычисление длины сообщения */
   pOut[OffsetData + 1] = n - OffsetData - 2;
   return n - OffsetData;
}

#define ID_1 1
#define ID_4 4
#define ID_5 5
#define ID_7 7
#define ID_8 8
#define ID_9 9

/* Пакет статуса девайса */
int frame_build_status_device_package(char* pOut, int OffsetData)
{
   int n = OffsetData;
   uint8_t bitFree = 8;
   uint8_t temp_byte = 0;
   char strNameFirmware[25];

   pOut[n++] = STATUS_DEVICE_PACKET;   // Тип пакета #1

   //Получаем время из RTC
   // RTC_t DateRTC;
   // getSystemDate(&DateRTC);
   // SecRTC = Date2Sec(&DateRTC);
   uint32_t SecRTC = g_uiPacStatDevTime;   //поставим время события
   n += bit_packing(pOut + n, SecRTC, &bitFree, 32);
   // ---===--- //

   //Подставим версию базовой прошивки.
   n += bit_packing(pOut + n, 0, &bitFree, 32);

   //Подставим версию текущей прошивки.
   n += bit_packing(pOut + n, flash_read_word(__CONST_FIRM_VER), &bitFree, 32);

   // Подставим флаг смены прошивки
   if (GetStatusReset() == UPDATE_FIRM_DEVICE)
   {
      temp_byte = 1;   // (прошивка обновлялась)
   }
   else
   {
      temp_byte = 0;
   }

   pOut[n++] = temp_byte;   //Флаг смены прошивки

   /* Status Reset */
   temp_byte = GetFlagsResetDevice(1);   // Флаг причины перезагрузки.
   pOut[n++] = temp_byte;
   DP_GSM("Satus Reset %d\r\n", temp_byte);

   /* Передаем количество элементов в таблице
       1) Режим работы устройства,
       2) SCID первой SIM карты,
       3) Версия текущей прошивки
       4) Режим работы девайса(трекер, стандартный, поиск)
   */
   pOut[n++] = 5;

#if (TWO_SIMCARD)
   pOut[n++] = 6;
   /* Передаем количество элементов в таблице
     1) Режим работы устройства,
     2) SCID первой SIM карты,
     3) SCID второй SIM карты,
     4) Версия текущей прошивки
     5) Режим работы девайса(трекер, стандартный, поиск)
     6)
   */
#endif   // TWO_SIMCARD
   uint8_t ucPowerStatus = 3;
   switch (g_stRam.stDevice.eCurPwrStat)
   {
   case POWER_RUN_MODE:
      ucPowerStatus = 0;
      break;
   case POWER_LOW_PWR1_MODE:
      ucPowerStatus = 1;
      break;
   case POWER_LOW_PWR2_MODE:
      ucPowerStatus = 2;
      break;
   default:
      ucPowerStatus = 3;
   }

   pOut[n++] = ID_1;   // ID Режим работы устройства.  //1
   pOut[n++] = ucPowerStatus;   //Основной режим работы устройства(0 – основной режим,1 – режим эн. сбер. #1
                                // 2 – режим эн. сбер. #2, 3 – режим глубокий сон).

   char str_scid[SIZE_SCID] = { '\0' };
   if (GetScidCurentFirstSim(str_scid))
   {
      pOut[n++] = ID_4;   // SCID первой SIM карты
      size_t len = strlen(str_scid);
      pOut[n++] = (char)len;
      loop(len)
      {
         pOut[n++] = str_scid[i];
      }
   }

#if (TWO_SIMCARD)
   if (GetScidCurentSecondSim(str_scid))
   {
      pOut[n++] = ID_5;   // SCID второй SIM карты
      size_t len = strlen(str_scid);
      pOut[n++] = (char)len;
      loop(len)
      {
         pOut[n++] = str_scid[i];
      }
   }
#endif   // TWO_SIMCARD

   RTC_t DateFirmware;
   Sec2Date(&DateFirmware, flash_read_word(__CONST_FIRM_VER));
   pOut[n++] = ID_7;   //Версия текущей прошивки
   sprintf(strNameFirmware, "U01.%s", DEV_VER);
   sprintf((strNameFirmware + strlen(strNameFirmware)), "%s.", HW_VER);
   sprintf((strNameFirmware + strlen(strNameFirmware)), "%.2i", DateFirmware.year);
   sprintf((strNameFirmware + strlen(strNameFirmware)), "%.2i", DateFirmware.month);
   sprintf((strNameFirmware + strlen(strNameFirmware)), "%.2i", DateFirmware.mday);
   sprintf((strNameFirmware + strlen(strNameFirmware)), "%.2i", DateFirmware.hour);
   sprintf((strNameFirmware + strlen(strNameFirmware)), "%.2i", DateFirmware.min);
   sprintf((strNameFirmware + strlen(strNameFirmware)), "%.2i", DateFirmware.sec);
   pOut[n++] = strlen(strNameFirmware);
   size_t len = strlen((char*)strNameFirmware);
   loop(len)
   {
      pOut[n++] = strNameFirmware[i];
   }

   /* Режим работы девайса */
   pOut[n++] = ID_8;
   pOut[n++] = GetModeDevice();   //(трекер, стандартный, поиск)

   /* номер симкарты, с которой осуществляется текущий сеанс. 1 – SIM1, 2 – SIM2 */
   pOut[n++] = ID_9;
   if (SelectNumSim(GET_NUM_SIM) == CURRENT_FIRST_SIMCARD)
   {
      pOut[n++] = 1;
   }
   else
   {
      pOut[n++] = 2;
   }

   return n - OffsetData;
}

/* Пакет получения данных по GSM станциям (LBS) */
int GetDataNavigationalGsmPacket(char* pBuf, int iOffsetData)
{
   int offset = 0;
   uint8_t sta_find_timeout = 0;
   GSM_INFO iGsm;

   INIT_GSM_INFO(iGsm);

   char* ptr = pBuf + 3;

   //поиск по домашнему оператору
   iGsm.count = 0;
   memset(&iGsm, 0, sizeof(iGsm));

   while (!(iGsm.count))
   {
      if (sta_find_timeout >= MC_COUNT)
      {
         return 0;   // Не нашли ни одной GSM станций ;(
      }
      getActiveLbsInfo(&iGsm, GetLbsFindTimeout());   //"at^smond"
      sta_find_timeout++;
      osDelay(1000);
   }

   // memcpy(&oGsm, &iGsm, sizeof(GSM_INFO));
   DP_GSM("D_LBS COUNT: %d\r\n", iGsm.count);

   /* заполняем пакет согласно протоколу iON Fm */
   offset = lbsInfo2buffer(&iGsm, time(), ptr, iOffsetData, (GetModeDevice() != TRACK_ION));
   if (offset > 0)
   {
      if (GetModeDevice() != TRACK_ION)
      {
         pBuf[iOffsetData] = NAVIGATIONAL_ADD_GSM_PACKET;
      }
      else
      {
         pBuf[iOffsetData] = NAVIGATIONAL_GSM_PACKET;
      }
      pBuf[iOffsetData + 1] = (offset)&0xFF;
      pBuf[iOffsetData + 2] = (offset >> 8) & 0xFF;
      offset += 3;
   }

   return offset;
}

/* Пакет формирования LBS без поиска сети */
int frame_build_lbs_packet(GSM_INFO* oGsm, char* pBuf, int iOffsetData)
{
   int offset = 0;
   char* ptr = pBuf + 3;

   offset = lbsInfo2buffer(oGsm, time(), ptr, iOffsetData, (GetModeDevice() != TRACK_ION));

   if (offset > 0)
   {
      if (GetModeDevice() != TRACK_ION)
      {
         pBuf[iOffsetData] = NAVIGATIONAL_ADD_GSM_PACKET;
      }
      else
      {
         pBuf[iOffsetData] = NAVIGATIONAL_GSM_PACKET;
      }
      pBuf[iOffsetData + 1] = (offset)&0xFF;
      pBuf[iOffsetData + 2] = (offset >> 8) & 0xFF;
      offset += 3;
   }

   return offset;
}

int frame_build_ans_cmd_paket(char* pOut, int OffsetData, char* pIn)
{
   int n = OffsetData;

   pOut[n++] = COMMAND_DEVICE_TO_SERVER_PACKET;

   int Len = strlen(pIn);
   pOut[n++] = Len;
   memcpy(&pOut[n++], &pIn[0], Len);
   memset(&pIn[0], 0, Len);   //Не забывем отчищать буфер ответа.
   return Len + 2;
}

/* Пакет синхронизации времени RTC */
int ServerSynchroTime(uint32_t SecRTC, char* pOut, int OffsetData)
{
   int n = OffsetData;
   uint8_t bitFree = 8;

   pOut[n++] = SYNCHRO_TIME_DEVICE_PACKET;   // Тип пакета 209
   n += bit_packing(pOut + n, SecRTC, &bitFree, 32);   // заполняем время.
   pOut[n++] = '\0';   // Reserve. NULL
   return n;
}

/* Функции с логами для отправки на iON Fm */
uint16_t (*ptrLogIonFmScheduler[])() = {
   GetDeviceWakeup,   // общее количество циклов (пробуждений) устройства
   GetGsmGprsErr,   // количество не открытых GPRS сеансов
   GetGsmFind,   // количество не удачных попыток зарегистрироваться в сети оператора
   GetServerErr,   // количество не полученных ответов от сервера за заданное время ожидания

   GetTimeAllPwrGps,   // время работы модуля  GPS (sec)
   GetTimeAllPwrGsm,   // время работы модуля  GSM (sec)

   GetGpsAllFind,   // поиск GPS координат всего запросов
   GetGpsErrFind,   // из них поиск не удачный

   GetGsmAllFind,   // мониторинг сети GSM всего запросов
   GetGsmErrFind,   // из них запрос не удачный

   GetServerAllConnect,   // соединений с сервером
   GetServerErrConnect,   // из них не удачных

   GetGsmAllErrWork,   // количество отключений GSM модуля во время работы из-за просадки по питанию
   GetGsmAllCoolWork,   // из-за низкой температуры
   GetCountReConnect,   // порядковый номер установленного таймаута на момент соединения с сервером из ряда таймаутов
                        // заданного сервером

   GetCountAllRebootDevice,   // общее количество перезагрузок устройства
   GetCountLowPwrRebootDevice,   // из них выход из LowPower
};

/* Пакет диагностики девайса */
/* ID 1-199 Числовая переменная. Размер 4 байта.
Формат записи - float число с плавающей точкой */
int frame_build_log_device_paket(char* pOut, int OffsetData)
{
   int n = OffsetData;
   uint8_t bitFree = 8;

   pOut[n++] = LOG_DEVICE_PACKET;   //Тип пакета.
   n++;   //длину пакета заполним в конце.

   //Получаем время из RTC
   uint32_t SecRTC = time();
   n += bit_packing(pOut + n, SecRTC, &bitFree, 32);   //Дата время.

   for (uint16_t id = 0; id < (sizeof(ptrLogIonFmScheduler) / sizeof(void*)); id++)
   {
#define OFFSET_ID_LOG 50   //Смещение ID события для протокола iON FM
      pOut[n++] = id + OFFSET_ID_LOG;   // ID 1-го события
      pOut[n++] = 0;   //Смещение в мс,
      pOut[n++] = 0;   //хз для чего.

      /* запаковываем все во float */
      float fData;
      char* pFL = (char*)&fData;
      fData = (float)(*ptrLogIonFmScheduler[id])();
      for (uint8_t i = 0; i < sizeof(float); i++)
      {
         pOut[n++] = pFL[i];
      }
   }

   /* Вычисление длины сообщения */
   pOut[OffsetData + 1] = n - OffsetData - 2;
   return n - OffsetData;
}

GSM_STEP configTime(uint8_t* pIn)
{
   uint32_t temp = 0;
   uint8_t bitFree = 8;
   pIn++;
   bit_unpacking(pIn, &temp, &bitFree, 32);
   TimeSynchronizationRTC(temp);
   RTC_t stDate;
   Sec2Date(&stDate, temp);
   DP_GSM("D_SRV DATE: %02d/", stDate.mday);
   DP_GSM("%02d/", stDate.month);
   DP_GSM("%02d ", stDate.year);
   DP_GSM("%02d:", stDate.hour);
   DP_GSM("%02d:", stDate.min);
   DP_GSM("%02d\r\n", stDate.sec);
   return CHECK_SMS;
}

GSM_STEP configCmdDevice(uint8_t* pIn)
{
   uint8_t n = 1;
   uint8_t len = pIn[n++];   //Длинна пакета

   if (!(len))
   {
      return GSM_PROFILE_GPRS_ANS_CMD_SERVER_ERR;   //Нет данных
   }

   //Найдем конец строки и подставим ноль.
   for (uint8_t i = 2; i < (len + 2); i++)
   {
      if (pIn[i] == '\n')
      {
         i++;
         pIn[i] = 0;
         break;
      }
   }

   if (parse_cmd(&pIn[n++], (uint8_t*)g_asInpDataFrameBuffer, INTERF_SRV, 0, 0) != FAIL)
   {
      //Если команда верна, ответим ОК серверу.
      return GSM_PROFILE_GPRS_ANS_CMD_SERVER_OK;
   }

   //Если команда неверна, ответим ERROR серверу.
   return GSM_PROFILE_GPRS_ANS_CMD_SERVER_ERR;
}

/* Driver Simple */
int put_ds_report(u8 id, u8 val, u8 speed, u8* out)
{
   int n = 0;
   uint32_t time_cur = time();

   out[n++] = DT_DRIVE_STYLE;
   out[n++] = 7;
   out[n++] = time_cur & 0xFF;
   out[n++] = (time_cur >> 8) & 0xFF;
   out[n++] = (time_cur >> 16) & 0xFF;
   out[n++] = (time_cur >> 24) & 0xFF;

   out[n++] = id;
   out[n++] = val;
   out[n++] = speed;

   return n;
}

int var2buffer(u8 id, void* pdat, u32 size, u8* out)
{
   u8* data = (u8*)pdat;
   u32 time_sec, time_ms = 0;
   int n = 0;

   time_sec = time();

   out[n++] = LOG_DEVICE_PACKET;
   n++;
   out[n++] = time_sec & 0xFF;
   out[n++] = (time_sec >> 8) & 0xFF;
   out[n++] = (time_sec >> 16) & 0xFF;
   out[n++] = (time_sec >> 24) & 0xFF;
   out[n++] = id;
   out[n++] = time_ms & 0xFF;
   out[n++] = (time_ms >> 8) & 0xFF;

   if (id < 200)
   {
      out[1] = 11;
      out[n++] = data[0];
      out[n++] = data[1];
      out[n++] = data[2];
      out[n++] = data[3];
   }
   else if (id != 255)
   {
      int i;
      if (size > 247)
         size = 247;
      out[1] = 8 + size;
      out[n++] = size;
      for (i = 0; i < size; i++)
      {
         out[n++] = data[i];
      }
   }

   return n;
}

int VarLog(u8 id, float data, u8* out)
{
   return var2buffer(id, &data, 4, out);
}

extern char g_aucBufDownHttpFirm[];
#define SERVER_TRUE_VALUE g_aucBufDownHttpFirm

int (*ptrParseValue[])(uint8_t* pIn, int index, uint16_t Len) = {
   ParseByteValue,   //Парсер однобайтовых параметров
   ParseDoubleByteValue,   //Парсер двухбайтовых параметров
   ParseWordValue,   //Парсер четырехбайтовых параметров
   ParseStringValue,   //Парсер строковых параметров
};

/* Пакет от сервера на конфигурацию параметров */
GSM_STEP configDevice(uint8_t* pIn)
{
   uint32_t temp;
   int n = 1;
   uint8_t bitFree = 8;
   uint16_t Len = 0;

   /* Длина данных пакета */
   n += bit_unpacking(&pIn[n], &temp, &bitFree, 16);
   // n += sizeof(Len);
   Len = (uint16_t)temp;
   DP_GSM("D_Pac Config len %02d\r\n", Len);
   memset(SERVER_TRUE_VALUE, 0, MAX_PARAM_CONFIG_VALUE);

   uint8_t num_func_parse = 0;
   while (n <= (Len + 1))
   {
      if (pIn[n] < 64)
         num_func_parse = 0;   //Парсер однобайтовых параметров
      if (pIn[n] < 128 && pIn[n] >= 64)
         num_func_parse = 1;   //Парсер двухбайтовых параметров
      if (pIn[n] < 192 && pIn[n] >= 128)
         num_func_parse = 2;   //Парсер четырехбайтовых параметров
      if (pIn[n] >= 192)
         num_func_parse = 3;   //Парсер строковых параметров
      n = (*ptrParseValue[num_func_parse])(pIn, n, Len);
   }
   SaveConfigCMD();
   return CHECK_SMS;
}

/* Ищем однобайтовые параметры от 0 до 64 */
const uint8_t MIN_VALUE_BYTE[] = {
   0,   //Маска отправки уведомлений СМС на телефон
   FIRST_SERVER,   //Тип и номер рабочего сервера
   0,   //Использование пользовательских настроек при подключении к GPRS
   0,   //Использование пин кода для сим карты
   FALSE,   //Настройки типа определения глушения
   0,   //Время поиска gsm сети
   0,   //Время ожидания GPRS соединения
   0,   //Время ожидания информации о GSM станциях
   1,   //Количество попыток установления GPRS соединения
   0,   //Разрешение передачи данных в роуминге
   60,   //Время ожидания ответа от сервера
   USER_POWER_AUTO,   //Ручная установка режима энергопотребления
   0,   //Разрешение режима энегросбер 1
   0,   //Разрешение режима энегросбер 2
   TRACK_ION,   //Алгоритм работы девайса
   ION_FM,   //Протокол по которому будет выходить девайс на сервер
   0xB0,   //Минимальная рабочая температура в градусах Цельсия -80
   1,   //Чувствительность Акселерометра
   10,   //Фильтр записи по курсу
   FALSE,   //Использование акселерометра в режиме поиска.
};
const uint8_t MAX_VALUE_BYTE[] = {
   255,   //Маска отправки уведомлений СМС на телефон
   SECOND_SERVER,   //Тип и номер рабочего сервера
   1,   //Использование пользовательских настроек при подключении к GPRS
   1,   //Использование пин кода для сим карты
   TRUE,   //Настройки типа определения глушения
   255,   //Время поиска gsm сети
   255,   //Время ожидания GPRS соединения
   255,   //Время ожидания информации о GSM станциях
   10,   //Количество попыток установления GPRS соединения
   1,   //Разрешение передачи данных в роуминге
   255,   //Время ожидания ответа от сервера
   USER_POWER_LOW_PWR2_MODE,   //Ручная установка режима энергопотребления
   1,   //Разрешение режима энегросбер 1
   1,   //Разрешение режима энегросбер 2
   TIMER_FIND,   //Алгоритм работы девайса
   FINDME_911,   //Протокол по которому будет выходить девайс на сервер
   0xF6,   //Минимальная рабочая температура в градусах Цельсия -10
   8,   //Чувствительность Акселерометра
   180,   //Фильтр записи по курсу
   TRUE,   //Использование акселерометра в режиме поиска.
};

int ParseByteValue(uint8_t* pIn, int index, uint16_t Len)
{
   int n = index;
   uint8_t Id = 255;   // ID параметра

   while (1)
   {
      _Bool val_true = FALSE;
      uint8_t value = 0;
      if (n >= (Len + sizeof(Len)))
         break;   //Проверка на переполнение длины
      Id = pIn[n];
      if (Id > 63)
         break;   //Однобайтовые переменные закончились.
      n++;
      value = pIn[n];
      n += sizeof(value);
      SERVER_TRUE_VALUE[Id] = SERVER_CONFIG_DEVICE_PACKET;
      switch (Id)
      {
      case 0:   //Маска отправки уведомлений СМС на телефон
         if (value >= MIN_VALUE_BYTE[Id] && value <= MAX_VALUE_BYTE[Id])
         {
            SetMaskMessageUser((TYPE_MESSAGE)value);
            val_true = TRUE;
         }
         else
         {
            val_true = FALSE;
         }
         break;

      case 1:   //Тип и номер рабочего сервера
         if (value >= MIN_VALUE_BYTE[Id] && value <= MAX_VALUE_BYTE[Id])
         {
            SetUseTypeServ((TYPE_SERVER)value);
            val_true = TRUE;
         }
         else
         {
            val_true = FALSE;
         }
         break;

      case 2:   //Использование пользовательских настроек при подключении к GPRS
         if (value >= MIN_VALUE_BYTE[Id] && value <= MAX_VALUE_BYTE[Id])
         {
            SetManualModeSimFirst((_Bool)value);
            val_true = TRUE;
         }
         else
         {
            val_true = FALSE;
         }
         break;

      case 3:   //Использование пин кода для сим карты
         if (value >= MIN_VALUE_BYTE[Id] && value <= MAX_VALUE_BYTE[Id])
         {
            SetPinLock((_Bool)value);
            val_true = TRUE;
         }
         else
         {
            val_true = FALSE;
         }
         break;

      case 4:   //Настройки типа определения глушения
         if (value >= MIN_VALUE_BYTE[Id] && value <= MAX_VALUE_BYTE[Id])
         {
            SetJamDetect((_Bool)value);
            val_true = TRUE;
         }
         else
         {
            val_true = FALSE;
         }
         break;

      case 5:   //Время поиска gsm сети в секундах
         if (value >= MIN_VALUE_BYTE[Id] && value <= MAX_VALUE_BYTE[Id])
         {
            SetGsmFindTimeout(value);
            val_true = TRUE;
         }
         else
         {
            val_true = FALSE;
         }
         break;

      case 6:   //Время поиска gprs в секундах
         if (value >= MIN_VALUE_BYTE[Id] && value <= MAX_VALUE_BYTE[Id])
         {
            SetGsmGprsTimeout(value);
            val_true = TRUE;
         }
         else
         {
            val_true = FALSE;
         }
         break;

      case 7:   //Время ожидания информации о GSM станциях
         if (value >= MIN_VALUE_BYTE[Id] && value <= MAX_VALUE_BYTE[Id])
         {
            SetLbsFindTimeout(value);
            val_true = TRUE;
         }
         else
         {
            val_true = FALSE;
         }
         break;

      case 8:   //Количество попыток установления GPRS соединения
         if (value >= MIN_VALUE_BYTE[Id] && value <= MAX_VALUE_BYTE[Id])
         {
            SetGprsOpenCount(value);
            val_true = TRUE;
         }
         else
         {
            val_true = FALSE;
         }
         break;

      case 9:   //Разрешение передачи данных в роуминге
         if (value >= MIN_VALUE_BYTE[Id] && value <= MAX_VALUE_BYTE[Id])
         {
            SetRoamingGprs((_Bool)value);
            val_true = TRUE;
         }
         else
         {
            val_true = FALSE;
         }
         break;

      case 10:   //Время ожидания ответа от сервера
         if (value >= MIN_VALUE_BYTE[Id] && value <= MAX_VALUE_BYTE[Id])
         {
            SetAnswerTimeout(value);
            val_true = TRUE;
         }
         else
         {
            val_true = FALSE;
         }
         break;

      case 11:   //Ручная установка режима энергопотребления
         if (value >= MIN_VALUE_BYTE[Id] && value <= MAX_VALUE_BYTE[Id])
         {
            SetUserPwrDevice((USER_PWR_STATUS)value);
            val_true = TRUE;
         }
         else
         {
            val_true = FALSE;
         }
         break;

      case 12:   //Разрешение режима энегросбер 1
         if (value >= MIN_VALUE_BYTE[Id] && value <= MAX_VALUE_BYTE[Id])
         {
            SetEnableUseLowPwr1Mode((_Bool)value);
            val_true = TRUE;
         }
         else
         {
            val_true = FALSE;
         }
         break;

      case 13:   //Разрешение режима энегросбер 2
         if (value >= MIN_VALUE_BYTE[Id] && value <= MAX_VALUE_BYTE[Id])
         {
            SetEnableUseLowPwr2Mode((_Bool)value);
            val_true = TRUE;
         }
         else
         {
            val_true = FALSE;
         }
         break;

      case 14:   //Алгоритм работы девайса(0 – стандартный, 1 - выход по таймеру, 2 - выход по акселерометру, 3 – режим
                 //трекера)
         if (value >= MIN_VALUE_BYTE[Id] && value <= MAX_VALUE_BYTE[Id])
         {
            SetModeDevice((TYPE_MODE_DEV)value);
            val_true = TRUE;
         }
         else
         {
            val_true = FALSE;
         }
         break;

      case 15:   //Протокол по которому будет выходить девайс на сервер (0 – протокол тип dev, 1 – протокол тип FM911)
         if (value >= MIN_VALUE_BYTE[Id] && value <= MAX_VALUE_BYTE[Id])
         {
            SetModeProtocol((TYPE_MODE_PROTOCOL)value);
            val_true = TRUE;
         }
         else
         {
            val_true = FALSE;
         }
         break;

      case 16:   //Минимальная рабочая температура в градусах Цельсия
         if (value >= MIN_VALUE_BYTE[Id] && value <= MAX_VALUE_BYTE[Id])
         {
            SetMinTemperaturWorkDevice((int8_t)value);
            val_true = TRUE;
         }
         else
         {
            val_true = FALSE;
         }
         break;

      case 17:   //Чувствительность Акселерометра 1...8
         if (value >= MIN_VALUE_BYTE[Id] && value <= MAX_VALUE_BYTE[Id])
         {
            SetAccelSensitivity(value);
            val_true = TRUE;
         }
         else
         {
            val_true = FALSE;
         }
         break;

      case 18:   //Фильтр записи по курсу (от 10, до 180 град)
         if ((value >= MIN_VALUE_BYTE[Id] && value <= MAX_VALUE_BYTE[Id]) || (!(value)))
         {
            SetGpsRecordCourse(value);
            val_true = TRUE;
         }
         else
         {
            val_true = FALSE;
         }
         break;

      case 19:   //Использование акселерометра в режиме поиска
         if (value >= MIN_VALUE_BYTE[Id] && value <= MAX_VALUE_BYTE[Id])
         {
            SetEnableAccelToFind(value);
            val_true = TRUE;
         }
         else
         {
            val_true = FALSE;
         }
         break;

      default:
         value = pIn[n];
         n += sizeof(value);
      }

      if (val_true)
      {
         DP_GSM("ID:%d, Value:%d, Save: YES\r\n", Id, value);
      }
      else
      {
         DP_GSM("ID:%d, Value:%d, Save: NO\r\n", Id, value);
      }
   }

   return n;
}
/******************************************************************************/

/* Ищем двухбайтовые параметры от 64 до 128 */
const uint16_t MIN_VALUE_DOUBLE_BYTE[] = {
   MIN_VAL_LOW_POWER1,   //Время сна в режиме энергопотребления LOW PWR 1
   MIN_VAL_LOW_POWER2,   //Время сна в режиме энергопотребления LOW PWR 2
   1,   //Время, в секундах, сброса флага движения
   MIN_VAL_RT,   //Время выхода по GPS с пакетом Real Time
   5,   //Фильтр по дистанции
   5,   //Фильтр по минимальной скорости
   0,   //Период сохранения данных, напряжении
   MIN_VAL_FTIME,   //Фильтр записи по времени

};
const uint16_t MAX_VALUE_DOUBLE_BYTE[] = {
   MAX_VAL_LOW_POWER1,   // 0)Время сна в режиме энергопотребления LOW PWR 1
   MAX_VAL_LOW_POWER2,   // 1)Время сна в режиме энергопотребления LOW PWR 2
   3600,   // 2)Время, в секундах, сброса флага движения
   MAX_VAL_RT,   // 3)Время выхода по GPS с пакетом Real Time
   65535,   // 4)Фильтр по дистанции
   30,   // 5)Фильтр по минимальной скорости
   65535,   // 6)Период сохранения данных, напряжении
   MAX_VAL_FTIME,   // 7)Фильтр записи по времени
};

int ParseDoubleByteValue(uint8_t* pIn, int index, uint16_t Len)
{
   int n = index;
   uint8_t Id = 255;   // ID параметра
   uint32_t temp;
   uint8_t bitFree = 8;

   void (*ptrU16_Value[])(uint16_t) = {
      SetTimeLowPwrMode1,   // 0)Время сна в режиме энергопотребления LOW PWR 1
      SetTimeLowPwrMode2,   // 1)Время сна в режиме энергопотребления LOW PWR 2
      SetAccelTimeCurrState,   // 2)Время, в секундах, сброса флага движения
      SetGpsRealtime,   // 3)Время выхода по GPS с пакетом Real Time
      SetGpsRecordDistanse,   // 4)Фильтр по дистанции
      SetGpsRecordMinSpeed,   // 5)Фильтр по минимальной скорости
      SetGpioRecordTime,   // 6)Период сохранения данных, напряжении в секундах
      SetGpsRecordtime,   // 7)Фильтр записи по времени
   };

   while (1)
   {
      uint16_t value = 0;
      if (n >= (Len + sizeof(Len)))
         break;   //Проверка на переполнение длины
      Id = pIn[n];
      if (Id > 127 || Id < 64)
         break;   //Двухбайтовые переменные закончились.
      Id -= 64;
      if (Id > MAX_UI16)
         break;   //Двухбайтовые переменные закончились.
      n++;
      n += bit_unpacking(&pIn[n], &temp, &bitFree, 16);
      // n += sizeof(value);
      value = (uint16_t)temp;

      if (temp >= MIN_VALUE_DOUBLE_BYTE[Id] && temp <= MAX_VALUE_DOUBLE_BYTE[Id])
      {
         (*ptrU16_Value[Id])(value);
         Id += 64;
         SERVER_TRUE_VALUE[Id] = SERVER_CONFIG_DEVICE_PACKET;
         DP_GSM("ID:%d, Value:%d, Save: YES\r\n", Id, value);
      }
      else if (Id == 5 && value == 0)
      {
         (*ptrU16_Value[Id])(value);
         Id += 64;
         SERVER_TRUE_VALUE[Id] = SERVER_CONFIG_DEVICE_PACKET;
         DP_GSM("ID:%d, Value:%d, Save: YES\r\n", Id, value);
      }
      else
      {
         Id += 64;
         SERVER_TRUE_VALUE[Id] = SERVER_CONFIG_DEVICE_PACKET;
         DP_GSM("ID:%d, Value:%d, Save: NO\r\n", Id, value);
      }
   }

   return n;
}
/******************************************************************************/

/* Ищем четырехбайтовые параметры от 128 до 192 */
const uint32_t MIN_VALUE_WORD[] = {
   MIN_VAL_LOW_POWER1,   // 0)Время перехода в режим энергопотребления LOW PWR 1
   MIN_VAL_LOW_POWER2,   // 1)Время перехода в режим энергопотребления LOW PWR 2
   MIN_VAL_SLEEP_TIME_STANDART,   // 2)Время сна в Стандартном режиме
   MIN_VAL_SLEEP_TIME_FIND,   // 3)Время выхода в режиме Поиск
   MIN_VAL_FIND_GPS_SAT,   // 4)Время поиска GPS спутников в режиме Поиск
   600,   // 5)Время откладывающие соединения с сервером в случае отсутствия связи
   600,   // 6)Время откладывающие соединения с сервером в случае отсутствия связи
   600,   // 7)Время откладывающие соединения с сервером в случае отсутствия связи
   600,   // 8)Время откладывающие соединения с сервером в случае отсутствия связи
   600,   // 9)Время откладывающие соединения с сервером в случае отсутствия связи
   1024,   // 10)Размер данных во флешке для перерехода в режим энергосбережения
};
const uint32_t MAX_VALUE_WORD[] = {
   MAX_VAL_LOW_POWER1,   // 0)Время перехода в режим энергопотребления LOW PWR 1
   MAX_VAL_LOW_POWER2,   // 1)Время перехода в режим энергопотребления LOW PWR 2
   MAX_VAL_SLEEP_TIME_STANDART,   // 2)Время сна в Стандартном режиме
   MAX_VAL_SLEEP_TIME_FIND,   // 3)Время выхода в режиме Поиск
   MAX_VAL_FIND_GPS_SAT,   // 4)Время поиска GPS спутников в режиме Поиск
   86400,   // 5)Время откладывающие соединения с сервером в случае отсутствия связи
   86400,   // 6)Время откладывающие соединения с сервером в случае отсутствия связи
   86400,   // 7)Время откладывающие соединения с сервером в случае отсутствия связи
   86400,   // 8)Время откладывающие соединения с сервером в случае отсутствия связи
   86400,   // 9)Время откладывающие соединения с сервером в случае отсутствия связи
   32768,   // 10)Размер данных во флешке для перерехода в режим энергосбережения
};

int ParseWordValue(uint8_t* pIn, int index, uint16_t Len)
{
   int n = index;
   uint8_t Id = 255;   // ID параметра
   uint8_t bitFree = 8;

   void (*ptrU32_Value[])(uint32_t) = {
      SetTimeoutLowPwrMode1,   // 0)Время перехода в режим энергопотребления LOW PWR 1
      SetTimeoutLowPwrMode2,   // 1)Время перехода в режим энергопотребления LOW PWR 2
      SetSleepTimeStandart,   // 2)Время сна в Стандартном режиме
      SetSleepTimeFind,   // 3)Время выхода в режиме Поиск
      SetGpsWait,   // 4)Время поиска GPS спутников в режиме Поиск
      SetTimeReconnect1,   // 5)Время откладывающие соединения с сервером в случае отсутствия связи
      SetTimeReconnect2,   // 6)Время откладывающие соединения с сервером в случае отсутствия связи
      SetTimeReconnect3,   // 7)Время откладывающие соединения с сервером в случае отсутствия связи
      SetTimeReconnect4,   // 8)Время откладывающие соединения с сервером в случае отсутствия связи
      SetTimeReconnect5,   // 9)Время откладывающие соединения с сервером в случае отсутствия связи
      SetLenDataFlashReady,   // 10)Размер данных во флешке для перерехода в режим энергосбережения
   };

   while (1)
   {
      uint32_t value = 0;
      if (n >= (Len + sizeof(Len)))
         break;   //Проверка на переполнение длины
      Id = pIn[n];
      if (Id > 191 || Id < 128)
         break;   //Двухбайтовые переменные закончились.
      Id -= 128;
      if (Id > MAX_UI32)
         break;   //Двухбайтовые переменные закончились.
      n++;
      n += bit_unpacking(&pIn[n], &value, &bitFree, 32);
      // n += sizeof(value);

      if (value >= MIN_VALUE_WORD[Id] && value <= MAX_VALUE_WORD[Id])
      {
         (*ptrU32_Value[Id])(value);
         Id += 128;
         SERVER_TRUE_VALUE[Id] = SERVER_CONFIG_DEVICE_PACKET;
         DP_GSM("ID:%d, Value:%d, Save: YES\r\n", Id, value);
      }
      else
      {
         Id += 128;
         SERVER_TRUE_VALUE[Id] = SERVER_CONFIG_DEVICE_PACKET;
         DP_GSM("ID:%d, Value:%d, Save: NO\r\n", Id, value);
      }
   }

   return n;
}
/******************************************************************************/

const uint8_t MAX_VALUE_STRING[] = {
   SIZE_PASW,   //Пароль на доступ
   SIZE_TEL,   //Номер телефона пользователя
   SIZE_SERV,   //Имя первично сервера
   SIZE_SERV,   //Имя вторичного сервера
   LOGIN_PASS_SIZE,   //Имя точки доступа для выхода в Internet(APN).
   LOGIN_PASS_SIZE,   //Пароль точки доступа
   LOGIN_PASS_SIZE,   //Логин точки доступа
   SIZE_PIN_CODE,   //Пин код пользовательской СИМ карты
   SIZE_SERV_FTP,   //Адрес сервера
};

const uint8_t MIN_VALUE_STRING[] = {
   MIN_VAL_SIZE_PASW,   //Пароль на доступ
   MIN_VAL_SIZE_TEL,   //Номер телефона пользователя
   NULL,   //Имя первично сервера
   NULL,   //Имя вторичного сервера
   NULL,   //Имя точки доступа для выхода в Internet(APN).
   NULL,   //Пароль точки доступа
   NULL,   //Логин точки доступа
   NULL,   //Пин код пользовательской СИМ карты
   NULL,   //Адрес сервера
};

int ParseStringValue(uint8_t* pIn, int index, uint16_t Len)
{
   int n = index;
   uint8_t Id = 255;   // ID параметра
   uint8_t len_string = 0;
   char str[42];

   void (*ptrSTR_Value[])(const char* ptr) = {
      SetAccessPass,   // 0
      SetUserTel,   // 1
      SetAddrFirstServ,   // 2
      SetAddrSecondServ, SetNameUserSimApn, SetNameUserSimPass, SetNameUserSimLogin, SetUserSimPin, SetDefAddrFirmSer,
   };

   while (1)
   {
      if (n >= (Len + sizeof(Len)))
         break;   //Проверка на переполнение длины
      Id = pIn[n];
      if (Id < 192)
         break;   //Строковые переменные закончились.
      Id -= 192;
      if (Id > MAX_STR)
         break;   //Строковые переменные закончились.
      n++;
      len_string = pIn[n];   //Получаем длину строки.
      n++;
      for (uint8_t i = 0; i < sizeof(str); i++)
      {
         if (i < len_string)
         {
            str[i] = pIn[n++];
         }
         else
         {
            str[i] = 0;
         }
      }
      if (len_string >= MIN_VAL_SIZE_PASW && strlen(str) < MAX_VALUE_STRING[Id])
      {
         if (len_string)
         {
            (*ptrSTR_Value[Id])(str);
         }
         else
         {
            (*ptrSTR_Value[Id])(STR_NULL);   //Параметр пустая строка
         }
         /* Сброс на дефолтные настройки если сервер сменился */
         if (Id == 2)
         {   // проверка установки нового сервера. номер функции SetAddrFirstServ()
            deviceDefConfig();   //сброс к дефолтным настройкам девайса
         }
         Id += 192;
         SERVER_TRUE_VALUE[Id] = SERVER_CONFIG_DEVICE_PACKET;
         DP_GSM("ID:%d, Value:\"%s\", Save: YES\r\n", Id, str);
      }
      else
      {
         Id += 192;
         SERVER_TRUE_VALUE[Id] = SERVER_CONFIG_DEVICE_PACKET;
         DP_GSM("ID:%d, Value:\"%s\", Save: NO\r\n", Id, str);
      }
   }
   return n;
}

/* Пакет конфига всего девайса */
int frame_build_config_packet(uint8_t ucType, char* pOut, int OffsetData)
{
   int n = OffsetData, Id;
   uint8_t bitFree = 8;

   pOut[n++] = ucType;   // Тип пакета
   //длину пока не заполняем, в конце заполним
   n += sizeof(uint16_t);

   /* Byte Parametrs */
   for (Id = 0; Id < 64; Id++)
   {
      if ((SERVER_TRUE_VALUE[Id]) && (Id <= MAX_UI8))
      {
         pOut[n++] = Id;
         switch (Id)
         {
         case 0:   //Маска отправки уведомлений СМС на телефон
            pOut[n++] = (uint8_t)GetMaskMessageUser();
            break;
         case 1:   //Тип и номер рабочего сервера
            pOut[n++] = (uint8_t)GetUseTypeServ();
            break;
         case 2:   //Использование пользовательских настроек при подключении к GPRS
            pOut[n++] = (uint8_t)GetManualModeSimFirst();
            break;
         case 3:   //Использование пин кода для сим карты
            pOut[n++] = (uint8_t)GetPinLock();
            break;
         case 4:   //Настройки типа определения глушения
            pOut[n++] = (uint8_t)GetJamDetect();
            break;
         case 5:   //Время поиска gsm сети в секундах
            pOut[n++] = (uint8_t)GetGsmFindTimeout();
            break;
         case 6:   //Время поиска gprs в секундах
            pOut[n++] = (uint8_t)GetGsmGprsTimeout();
            break;
         case 7:   //Время ожидания информации о GSM станциях
            pOut[n++] = (uint8_t)GetLbsFindTimeout();
            break;
         case 8:   //Количество попыток установления GPRS соединения
            pOut[n++] = (uint8_t)GetGprsOpenCount();
            break;
         case 9:   //Количество попыток установления GPRS соединения
            pOut[n++] = (uint8_t)GetRoamingGprs();
            break;
         case 10:   //Время ожидания ответа от сервера
            pOut[n++] = (uint8_t)GetAnswerTimeout();
            break;
         case 11:   //Ручная установка режима энергопотребления
            pOut[n++] = (uint8_t)GetUserPwrDevice();
            break;
         case 12:   //Разрешение режима энегросбер 1
            pOut[n++] = (uint8_t)GetEnableUseLowPwr1Mode();
            break;
         case 13:   //Разрешение режима энегросбер 2
            pOut[n++] = (uint8_t)GetEnableUseLowPwr2Mode();
            break;
         case 14:   //Алгоритм работы девайса(0 – стандартный, 1 - выход по таймеру, 2 - выход по акселерометру, 3 –
                    //режим трекера)
            pOut[n++] = (uint8_t)GetModeDevice();
            break;
         case 15:   //Протокол по которому будет выходить девайс на сервер (0 – протокол тип dev, 1 – протокол тип
                    //FM911)
            pOut[n++] = (uint8_t)GetModeProtocol();
            break;
         case 16:   //Минимальная рабочая температура в градусах Цельсия.
            pOut[n++] = GetMinTemperaturWorkDevice();
            break;
         case 17:   //Чувствительность Акселерометра.
            pOut[n++] = (uint8_t)GetAccelSensitivity();
            break;
         case 18:   //Фильтр записи по курсу.
            pOut[n++] = (uint8_t)GetGpsRecordCourse();
            break;
         case MAX_UI8:   //Использование акселерометра в режиме поиска.
            pOut[n++] = (uint8_t)GetEnableAccelToFind();
            break;
         }
      }
   }
   /*******************************/

   /* Double Byte Parametrs */
   uint16_t (*ptrU16_Value[])(void) = {
      GetTimeLowPwrMode1,   // 0)Время сна в режиме энергопотребления LOW PWR 1
      GetTimeLowPwrMode2,   // 1)Время сна в режиме энергопотребления LOW PWR 2
      GetAccelTimeCurrState,   // 2)Время, в секундах, сброса флага движения
      GetGpsRealtime,   // 3)Время выхода по GPS с пакетом Real Time
      GetGpsRecordDistanse,   // 4)Фильтр по дистанции
      GetGpsRecordMinSpeed,   // 5)Фильтр по минимальной скорости
      GetGpioRecordTime,   // 6)Период сохранения данных, напряжении в секундах
      GetGpsRecordtime,   // 7)Фильтр записи по времени
   };

   for (; Id < 128; Id++)
   {
      if ((SERVER_TRUE_VALUE[Id]) && ((Id - 64) < MAX_UI16))
      {
         pOut[n++] = Id;
         n += bit_packing(pOut + n, (*ptrU16_Value[Id - 64])(), &bitFree, 16);
      }
   }
   /*******************************/

   /* Word Parametrs */
   uint32_t (*ptrU32_Value[])(void) = {
      GetTimeoutLowPwrMode1,   // 0) Время перехода в режим энергопотребления LOW PWR 1
      GetTimeoutLowPwrMode2,   // 1) Время перехода в режим энергопотребления LOW PWR 2
      GetSleepTimeStandart,   // 2) Время сна в Стандартном режиме
      GetSleepTimeFind,   // 3) Время выхода в режиме Поиск
      GetGpsWait,   // 4) Время поиска GPS спутников в режиме Поиск
      GetTimeReconnect1,   // 5) Время откладывающие соединения с сервером в случае отсутствия связи
      GetTimeReconnect2,   // 6) Время откладывающие соединения с сервером в случае отсутствия связи
      GetTimeReconnect3,   // 7) Время откладывающие соединения с сервером в случае отсутствия связи
      GetTimeReconnect4,   // 8) Время откладывающие соединения с сервером в случае отсутствия связи
      GetTimeReconnect5,   // 9) Время откладывающие соединения с сервером в случае отсутствия связи
      GetLenDataFlashReady,   // 10) Размер данных во флешке для перерехода в режим энергосбережения
   };

   for (; Id < 192; Id++)
   {
      if ((SERVER_TRUE_VALUE[Id]) && ((Id - 128) < MAX_UI32))
      {
         pOut[n++] = Id;
         n += bit_packing(pOut + n, (*ptrU32_Value[Id - 128])(), &bitFree, 32);
      }
   }

   /* String Parametrs */
   int (*ptrSTR_Value[])(char* ptr) = {
      GetAccessPass,      GetUserTel,          GetAddrFirstServ, GetAddrSecondServ, GetNameUserSimApn,
      GetNameUserSimPass, GetNameUserSimLogin, GetUserSimPin,    GetDefAddrFirmSer,
   };

   char str[42] = { 0 };
   for (; Id < 256; Id++)
   {
      if ((SERVER_TRUE_VALUE[Id]) && ((Id - 192) < MAX_STR))
      {
         pOut[n++] = Id;
         int len = (*ptrSTR_Value[Id - 192])(str);
         pOut[n++] = (uint8_t)len;
         for (int i = 0; i < len; i++)
         {
            pOut[n++] = str[i];
         }
      }
   }
   /*******************************/

   /* заполним длину */
   uint16_t len = n - OffsetData - sizeof(uint16_t);
   bit_packing(pOut + OffsetData + 1, len, &bitFree, 16);
   return n - OffsetData;
}
