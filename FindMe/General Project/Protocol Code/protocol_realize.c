

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

/* ��������� �������� ������� ������� */
int FrameGeneralBuild(const DATA_FRAME* stTypePack, const char* pInp, uint16_t LenInp, char* pOut)
{
   uint16_t n = 0;
   uint16_t head_size = 0;
   uint16_t crc16 = 0;
   uint64_t ulSerialNumber = stTypePack->ulSerialNumber;
   uint8_t* p = (uint8_t*)&ulSerialNumber;

   pOut[n++] = PROTOCOL_ID;
   pOut[n++] = PROTOCOL_VERSION;

   n++;   // ����� ������ ���� ����� ������,
   n++;   // �������� � � �����.

   // ����������� �������� ����� IMEI //
   pOut[n + head_size++] = *p;
   pOut[n + head_size++] = *(p + 1);
   pOut[n + head_size++] = *(p + 2);
   pOut[n + head_size++] = *(p + 3);
   pOut[n + head_size++] = *(p + 4);
   pOut[n + head_size++] = *(p + 5);
   pOut[n + head_size++] = *(p + 6);
   //

   // ����������� ��� ��������������� ������ //
   pOut[n + head_size++] = stTypePack->ucType;
   //

   // ����������� ���������� ����� ������  //
   pOut[n + head_size++] = (stTypePack->usDevicePaketNumber) & 0xFF;
   pOut[n + head_size++] = (stTypePack->usDevicePaketNumber >> 8) & 0xFF;

   n += 10;
   //
   loop(LenInp)
   {
      pOut[n++] = pInp[i];
   }

   // ����������� ����� ������ �� CRC //
   pOut[2] = (n - 4) & 0xFF;
   pOut[3] = ((n - 4) >> 8) & 0xFF;

   crc16 = get_cs16((uint8_t*)pOut, n);

   pOut[n++] = crc16 & 0xFF;
   pOut[n++] = (crc16 >> 8) & 0xFF;

   return n;
}

/* ��������� ����� �� ������� */
int ack_data_parser(const char* pInp, uint16_t LenInp)
{
   uint16_t crc = get_cs16((uint8_t*)pInp, LenInp);   // Check CRC16

   if ((crc) || (LenInp < 11))
   {
      if (crc)
      {
         GSM_DPD("!CRC Pac Fail!\r\n", strlen("!CRC Pac Fail!\r\n"));
      }
      //�� ������� �RC � ����� �����, ����� ��������� ������� C_FAIL.
      g_stFrame.usFlagsRetDataServer |= fC_FAIL;
      return fC_FAIL;
   }

   //���������� ���������� ����� ������.
   g_stFrame.usServerPaketNumber = g_aucOutDataFrameBuffer[12];
   int temp = g_aucOutDataFrameBuffer[13];
   g_stFrame.usServerPaketNumber |= temp << 8;

   switch (pInp[11])
   {
   case S_ACK:
      g_stFrame.usFlagsRetDataServer |= fS_ACK;
      break;   //������ �������
   case S_REQ:
      g_stFrame.usFlagsRetDataServer |= fS_REQ;
      break;   //������ ����������� ������
   case S_FAIL:
      g_stFrame.usFlagsRetDataServer |= fS_FAIL;
      break;   //������ �� �������
   case S_ACK_REQ:
      g_stFrame.usFlagsRetDataServer |= fS_ACK_REQ;
      break;   //������ ������� � ���� ������ ��� ����������
   case S_DATA:
      g_stFrame.usFlagsRetDataServer |= fS_DATA;
      break;   //������ �� �������
   case S_ACK_DLY:
      g_stFrame.usFlagsRetDataServer |= fS_ACK_DLY;
      break;   //������� ������ ������� � ��������� � ���������� �������
   case S_ASK_DATA:
      g_stFrame.usFlagsRetDataServer |= fS_ASK_DATA;
      break;   //������� ������ ������� � ������ ������� �� �������
   case S_FIN:
      g_stFrame.usFlagsRetDataServer |= fS_FIN;
      break;   //������ ��������� ����� �����
   }

   return (pInp[11]);
}

/* ��������� ����� ������ ������� C_ASK */
int FrameGeneralBuildAckC(uint8_t TypePack, const char* pInp, uint16_t LenInp, char* pOut)
{
   uint16_t n = 0;
   uint16_t head_size = 0;
   uint16_t crc16 = 0;

   pOut[n++] = PROTOCOL_ID;
   pOut[n++] = PROTOCOL_VERSION;

   n++;   // ����� ������ ���� ����� ������,
   n++;   // �������� � � �����.

   g_stFrame.ulSerialNumber = GetIMEI();
   // ����������� �������� ����� IMEI //
   uint8_t* p = (uint8_t*)&g_stFrame.ulSerialNumber;

   pOut[n + head_size++] = *p;
   pOut[n + head_size++] = *(p + 1);
   pOut[n + head_size++] = *(p + 2);
   pOut[n + head_size++] = *(p + 3);
   pOut[n + head_size++] = *(p + 4);
   pOut[n + head_size++] = *(p + 5);
   pOut[n + head_size++] = *(p + 6);
   //

   // ����������� ��� ��������������� ������ //
   pOut[n + head_size++] = TypePack;
   //

   // ����������� ���������� ����� ������  //
   pOut[n + head_size++] = (g_iFramePaketNumber)&0xFF;
   pOut[n + head_size++] = (g_iFramePaketNumber >> 8) & 0xFF;

   n += 10;
   //
   for (uint16_t i = 0; i < LenInp; i++)
   {
      pOut[n++] = pInp[i];
   }

   // ����������� ����� ������ �� CRC //
   pOut[2] = (n - 4) & 0xFF;
   pOut[3] = ((n - 4) >> 8) & 0xFF;

   crc16 = get_cs16((uint8_t*)pOut, n);

   pOut[n++] = crc16 & 0xFF;
   pOut[n++] = (crc16 >> 8) & 0xFF;

   return n;
}

/* ����� ���������� ��������� */
uint32_t frame_build_navigational_not_valid_packet(char* pOut, int OffsetData)
{
   int n = 0;
   uint8_t bitFree = 8;

   pOut += OffsetData;
   pOut[n++] = NAVIGATIONAL_PACKET;

   //�������� ����� �� RTC
   uint32_t SecRTC = time();

   /* TIME */
   n += bit_packing(pOut + n, SecRTC, &bitFree, 32);

   /* �������� �� ��������� ������������(������) */
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

   uint8_t FlagPotocol = 0x40;   // 6 ��� � ���������� ��������� 1- �� ��������.

   //������ ���� �������� �������������(������ ������������, ��� ������� �������� �� GPS).
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

/* ����� ��������� */
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

   //������ ���� �������� �������������(������ ������������, ��� ������� �������� �� GPS).
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

   //������ ���� �������� �������������(������ ������������, ��� ������� �������� �� GPS).
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

/* ����� �� GPIO */
int GetDataPeripheryPacket(const TPortInputCFG* const pAds, char* pOut, int OffsetData)
{
   int n = OffsetData;
   uint8_t bitFree = 8;

   // ��� ������
   pOut[n++] = PERIPHERY_PACKET;
   //����� ���� �� ���������, � ����� ��������
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

/* ����� ��������� ������ � ����� ����� �������� */
GSM_STEP configFirmware(uint8_t* pIn)
{
   uint8_t n = 1;
   uint32_t SizeFirmware = 0;
   int8_t LenTempDataServer = 0;   //����� ������������ ������

   SizeFirmware = pIn[n++] << 24;
   SizeFirmware |= pIn[n++] << 16;
   SizeFirmware |= pIn[n++] << 8;
   SizeFirmware |= pIn[n++];

   g_stRam.stFirmware.bFirmSendServ = TRUE;   // ������ ���� ��� �������� ������ �� �������.

   if (SizeFirmware == 0)
   {   //������ ��������
      return GSM_PROFILE_HTTP_SOCKET;
   }

   LenTempDataServer = pIn[n++];   //����� ������
   // �������� �����
   if (LenTempDataServer > SIZE_FTP_USER)
   {
      return GSM_PROFILE_HTTP_SOCKET;
   }
   // ��������� �����
   volatile char strUserSer[SIZE_FTP_USER] = { 0 };
   for (uint8_t i = 0; i < LenTempDataServer; i++)
   {
      strUserSer[i] = pIn[n++];
   }

   LenTempDataServer = pIn[n++];   //����� ������
   if (LenTempDataServer > SIZE_PASW)
   {
      return GSM_PROFILE_HTTP_SOCKET;
   }
   // ��������� ������
   volatile char strPswSer[SIZE_PASW] = { 0 };
   for (uint8_t i = 0; i < LenTempDataServer; i++)
   {
      strPswSer[i] = pIn[n++];
   }
   LenTempDataServer = pIn[n++];   // ����� ������
   // ��������� ������

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

   //������� ������ '=' ��� HTTP
   uint8_t sym_n = 0;
   for (uint8_t i = 0; i < sizeof(strNameSer); i++)
   {
      if (strNameSer[i] == '=')
         sym_n = i;
   }
   SetAddrFirmSer(strNameSer);

   if (!(sym_n))
   {   //�� ����� ������ '/' ��� '='
      return GSM_PROFILE_HTTP_SOCKET;
   }

   //��������� ������
   SetIntNewFirmware(atol(&strNameSer[++sym_n]));
   SaveConfigCMD();

   return GSM_PROFILE_HTTP_SOCKET;
}

/* ����� ���������� �������� �������� */
int frame_build_received_firmware(char* pOut)
{
   uint8_t n = 0;
   pOut[n++] = RECEIVED_FIRMWARE_PACKET;   // ��� ������.
   uint32_t uiNameNewFirm = GetIntNewFirmware();   // ��� ����� ��������.
   uint8_t bitFree = 8;
   n += bit_packing(pOut + n, uiNameNewFirm, &bitFree, 32);

   if (GetFlagsStatusFirmware())
   {   // 1-���� ������ ��� ����������, 0-��� ������.
      DPS("-ERR: FLAG FIRM-\r\n");
   }
   else
   {
      DPS("-OK: FLAG FIRM-\r\n");
   }

   pOut[n++] = GetFlagsStatusFirmware();   //������ ����� ������� ��������.
   return n;
}

/*  ������� ����� �������� */
GSM_STEP changeFirmware(uint8_t* pIn)
{
   SetStatusReset(UPDATE_FIRM_DEVICE);
   SetStatusDeviceReset(GetStatusReset());
   DPS("-UPDATE FIRMWARE-\r\n");
   return RESTART_NOW;
}

/* ����� ���������� ������� */
int frame_build_hardware_event_packet(char* pOut, int OffsetData)
{
   int n = OffsetData;
   uint8_t bitFree = 8;
   uint8_t FlagPotocol = 0;

   pOut[n++] = HARDWARE_EVENT_PACKET;   // ��� ������
   //����� ���� �� ���������, � ����� ��������
   n++;

   //�������� ����� �� RTC
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

   //���� �������� ����������
   if (!(GetGpsStatus()))
   {
      FlagPotocol |= 1;   // 0-���������� �������, 1-���������� �� �������
   }

   //������ ���� �������� �������������(������ ������������, ��� ������� �������� �� GPS).
   if ((AccelState() == ACC_STATE_MOVE) || (g_stRam.stAccel.eAccelState == ACC_STATE_MOVE))
   {
      g_stRam.stAccel.eAccelState = ACC_STATE_STOP;
      FlagPotocol |= 2;   // 0-��� ��������, 1-���� ��������
   }
   n += bit_packing(pOut + n, FlagPotocol, &bitFree, 6);

   /* ������ ��������� ������ */
   pOut[n++] = ALARM_BUTTON;
   pOut[n++] = (GetFlagsResetDevice(0) == BUTTON_RESET);

   /* ���������� ����� ��������� */
   pOut[OffsetData + 1] = n - OffsetData - 2;
   return n - OffsetData;
}

#define ID_1 1
#define ID_4 4
#define ID_5 5
#define ID_7 7
#define ID_8 8
#define ID_9 9

/* ����� ������� ������� */
int frame_build_status_device_package(char* pOut, int OffsetData)
{
   int n = OffsetData;
   uint8_t bitFree = 8;
   uint8_t temp_byte = 0;
   char strNameFirmware[25];

   pOut[n++] = STATUS_DEVICE_PACKET;   // ��� ������ #1

   //�������� ����� �� RTC
   // RTC_t DateRTC;
   // getSystemDate(&DateRTC);
   // SecRTC = Date2Sec(&DateRTC);
   uint32_t SecRTC = g_uiPacStatDevTime;   //�������� ����� �������
   n += bit_packing(pOut + n, SecRTC, &bitFree, 32);
   // ---===--- //

   //��������� ������ ������� ��������.
   n += bit_packing(pOut + n, 0, &bitFree, 32);

   //��������� ������ ������� ��������.
   n += bit_packing(pOut + n, flash_read_word(__CONST_FIRM_VER), &bitFree, 32);

   // ��������� ���� ����� ��������
   if (GetStatusReset() == UPDATE_FIRM_DEVICE)
   {
      temp_byte = 1;   // (�������� �����������)
   }
   else
   {
      temp_byte = 0;
   }

   pOut[n++] = temp_byte;   //���� ����� ��������

   /* Status Reset */
   temp_byte = GetFlagsResetDevice(1);   // ���� ������� ������������.
   pOut[n++] = temp_byte;
   DP_GSM("Satus Reset %d\r\n", temp_byte);

   /* �������� ���������� ��������� � �������
       1) ����� ������ ����������,
       2) SCID ������ SIM �����,
       3) ������ ������� ��������
       4) ����� ������ �������(������, �����������, �����)
   */
   pOut[n++] = 5;

#if (TWO_SIMCARD)
   pOut[n++] = 6;
   /* �������� ���������� ��������� � �������
     1) ����� ������ ����������,
     2) SCID ������ SIM �����,
     3) SCID ������ SIM �����,
     4) ������ ������� ��������
     5) ����� ������ �������(������, �����������, �����)
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

   pOut[n++] = ID_1;   // ID ����� ������ ����������.  //1
   pOut[n++] = ucPowerStatus;   //�������� ����� ������ ����������(0 � �������� �����,1 � ����� ��. ����. #1
                                // 2 � ����� ��. ����. #2, 3 � ����� �������� ���).

   char str_scid[SIZE_SCID] = { '\0' };
   if (GetScidCurentFirstSim(str_scid))
   {
      pOut[n++] = ID_4;   // SCID ������ SIM �����
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
      pOut[n++] = ID_5;   // SCID ������ SIM �����
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
   pOut[n++] = ID_7;   //������ ������� ��������
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

   /* ����� ������ ������� */
   pOut[n++] = ID_8;
   pOut[n++] = GetModeDevice();   //(������, �����������, �����)

   /* ����� ��������, � ������� �������������� ������� �����. 1 � SIM1, 2 � SIM2 */
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

/* ����� ��������� ������ �� GSM �������� (LBS) */
int GetDataNavigationalGsmPacket(char* pBuf, int iOffsetData)
{
   int offset = 0;
   uint8_t sta_find_timeout = 0;
   GSM_INFO iGsm;

   INIT_GSM_INFO(iGsm);

   char* ptr = pBuf + 3;

   //����� �� ��������� ���������
   iGsm.count = 0;
   memset(&iGsm, 0, sizeof(iGsm));

   while (!(iGsm.count))
   {
      if (sta_find_timeout >= MC_COUNT)
      {
         return 0;   // �� ����� �� ����� GSM ������� ;(
      }
      getActiveLbsInfo(&iGsm, GetLbsFindTimeout());   //"at^smond"
      sta_find_timeout++;
      osDelay(1000);
   }

   // memcpy(&oGsm, &iGsm, sizeof(GSM_INFO));
   DP_GSM("D_LBS COUNT: %d\r\n", iGsm.count);

   /* ��������� ����� �������� ��������� iON Fm */
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

/* ����� ������������ LBS ��� ������ ���� */
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
   memset(&pIn[0], 0, Len);   //�� ������� �������� ����� ������.
   return Len + 2;
}

/* ����� ������������� ������� RTC */
int ServerSynchroTime(uint32_t SecRTC, char* pOut, int OffsetData)
{
   int n = OffsetData;
   uint8_t bitFree = 8;

   pOut[n++] = SYNCHRO_TIME_DEVICE_PACKET;   // ��� ������ 209
   n += bit_packing(pOut + n, SecRTC, &bitFree, 32);   // ��������� �����.
   pOut[n++] = '\0';   // Reserve. NULL
   return n;
}

/* ������� � ������ ��� �������� �� iON Fm */
uint16_t (*ptrLogIonFmScheduler[])() = {
   GetDeviceWakeup,   // ����� ���������� ������ (�����������) ����������
   GetGsmGprsErr,   // ���������� �� �������� GPRS �������
   GetGsmFind,   // ���������� �� ������� ������� ������������������ � ���� ���������
   GetServerErr,   // ���������� �� ���������� ������� �� ������� �� �������� ����� ��������

   GetTimeAllPwrGps,   // ����� ������ ������  GPS (sec)
   GetTimeAllPwrGsm,   // ����� ������ ������  GSM (sec)

   GetGpsAllFind,   // ����� GPS ��������� ����� ��������
   GetGpsErrFind,   // �� ��� ����� �� �������

   GetGsmAllFind,   // ���������� ���� GSM ����� ��������
   GetGsmErrFind,   // �� ��� ������ �� �������

   GetServerAllConnect,   // ���������� � ��������
   GetServerErrConnect,   // �� ��� �� �������

   GetGsmAllErrWork,   // ���������� ���������� GSM ������ �� ����� ������ ��-�� �������� �� �������
   GetGsmAllCoolWork,   // ��-�� ������ �����������
   GetCountReConnect,   // ���������� ����� �������������� �������� �� ������ ���������� � �������� �� ���� ���������
                        // ��������� ��������

   GetCountAllRebootDevice,   // ����� ���������� ������������ ����������
   GetCountLowPwrRebootDevice,   // �� ��� ����� �� LowPower
};

/* ����� ����������� ������� */
/* ID 1-199 �������� ����������. ������ 4 �����.
������ ������ - float ����� � ��������� ������ */
int frame_build_log_device_paket(char* pOut, int OffsetData)
{
   int n = OffsetData;
   uint8_t bitFree = 8;

   pOut[n++] = LOG_DEVICE_PACKET;   //��� ������.
   n++;   //����� ������ �������� � �����.

   //�������� ����� �� RTC
   uint32_t SecRTC = time();
   n += bit_packing(pOut + n, SecRTC, &bitFree, 32);   //���� �����.

   for (uint16_t id = 0; id < (sizeof(ptrLogIonFmScheduler) / sizeof(void*)); id++)
   {
#define OFFSET_ID_LOG 50   //�������� ID ������� ��� ��������� iON FM
      pOut[n++] = id + OFFSET_ID_LOG;   // ID 1-�� �������
      pOut[n++] = 0;   //�������� � ��,
      pOut[n++] = 0;   //�� ��� ����.

      /* ������������ ��� �� float */
      float fData;
      char* pFL = (char*)&fData;
      fData = (float)(*ptrLogIonFmScheduler[id])();
      for (uint8_t i = 0; i < sizeof(float); i++)
      {
         pOut[n++] = pFL[i];
      }
   }

   /* ���������� ����� ��������� */
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
   uint8_t len = pIn[n++];   //������ ������

   if (!(len))
   {
      return GSM_PROFILE_GPRS_ANS_CMD_SERVER_ERR;   //��� ������
   }

   //������ ����� ������ � ��������� ����.
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
      //���� ������� �����, ������� �� �������.
      return GSM_PROFILE_GPRS_ANS_CMD_SERVER_OK;
   }

   //���� ������� �������, ������� ERROR �������.
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
   ParseByteValue,   //������ ������������ ����������
   ParseDoubleByteValue,   //������ ������������ ����������
   ParseWordValue,   //������ ��������������� ����������
   ParseStringValue,   //������ ��������� ����������
};

/* ����� �� ������� �� ������������ ���������� */
GSM_STEP configDevice(uint8_t* pIn)
{
   uint32_t temp;
   int n = 1;
   uint8_t bitFree = 8;
   uint16_t Len = 0;

   /* ����� ������ ������ */
   n += bit_unpacking(&pIn[n], &temp, &bitFree, 16);
   // n += sizeof(Len);
   Len = (uint16_t)temp;
   DP_GSM("D_Pac Config len %02d\r\n", Len);
   memset(SERVER_TRUE_VALUE, 0, MAX_PARAM_CONFIG_VALUE);

   uint8_t num_func_parse = 0;
   while (n <= (Len + 1))
   {
      if (pIn[n] < 64)
         num_func_parse = 0;   //������ ������������ ����������
      if (pIn[n] < 128 && pIn[n] >= 64)
         num_func_parse = 1;   //������ ������������ ����������
      if (pIn[n] < 192 && pIn[n] >= 128)
         num_func_parse = 2;   //������ ��������������� ����������
      if (pIn[n] >= 192)
         num_func_parse = 3;   //������ ��������� ����������
      n = (*ptrParseValue[num_func_parse])(pIn, n, Len);
   }
   SaveConfigCMD();
   return CHECK_SMS;
}

/* ���� ������������ ��������� �� 0 �� 64 */
const uint8_t MIN_VALUE_BYTE[] = {
   0,   //����� �������� ����������� ��� �� �������
   FIRST_SERVER,   //��� � ����� �������� �������
   0,   //������������� ���������������� �������� ��� ����������� � GPRS
   0,   //������������� ��� ���� ��� ��� �����
   FALSE,   //��������� ���� ����������� ��������
   0,   //����� ������ gsm ����
   0,   //����� �������� GPRS ����������
   0,   //����� �������� ���������� � GSM ��������
   1,   //���������� ������� ������������ GPRS ����������
   0,   //���������� �������� ������ � ��������
   60,   //����� �������� ������ �� �������
   USER_POWER_AUTO,   //������ ��������� ������ �����������������
   0,   //���������� ������ ���������� 1
   0,   //���������� ������ ���������� 2
   TRACK_ION,   //�������� ������ �������
   ION_FM,   //�������� �� �������� ����� �������� ������ �� ������
   0xB0,   //����������� ������� ����������� � �������� ������� -80
   1,   //���������������� �������������
   10,   //������ ������ �� �����
   FALSE,   //������������� ������������� � ������ ������.
};
const uint8_t MAX_VALUE_BYTE[] = {
   255,   //����� �������� ����������� ��� �� �������
   SECOND_SERVER,   //��� � ����� �������� �������
   1,   //������������� ���������������� �������� ��� ����������� � GPRS
   1,   //������������� ��� ���� ��� ��� �����
   TRUE,   //��������� ���� ����������� ��������
   255,   //����� ������ gsm ����
   255,   //����� �������� GPRS ����������
   255,   //����� �������� ���������� � GSM ��������
   10,   //���������� ������� ������������ GPRS ����������
   1,   //���������� �������� ������ � ��������
   255,   //����� �������� ������ �� �������
   USER_POWER_LOW_PWR2_MODE,   //������ ��������� ������ �����������������
   1,   //���������� ������ ���������� 1
   1,   //���������� ������ ���������� 2
   TIMER_FIND,   //�������� ������ �������
   FINDME_911,   //�������� �� �������� ����� �������� ������ �� ������
   0xF6,   //����������� ������� ����������� � �������� ������� -10
   8,   //���������������� �������������
   180,   //������ ������ �� �����
   TRUE,   //������������� ������������� � ������ ������.
};

int ParseByteValue(uint8_t* pIn, int index, uint16_t Len)
{
   int n = index;
   uint8_t Id = 255;   // ID ���������

   while (1)
   {
      _Bool val_true = FALSE;
      uint8_t value = 0;
      if (n >= (Len + sizeof(Len)))
         break;   //�������� �� ������������ �����
      Id = pIn[n];
      if (Id > 63)
         break;   //������������ ���������� �����������.
      n++;
      value = pIn[n];
      n += sizeof(value);
      SERVER_TRUE_VALUE[Id] = SERVER_CONFIG_DEVICE_PACKET;
      switch (Id)
      {
      case 0:   //����� �������� ����������� ��� �� �������
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

      case 1:   //��� � ����� �������� �������
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

      case 2:   //������������� ���������������� �������� ��� ����������� � GPRS
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

      case 3:   //������������� ��� ���� ��� ��� �����
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

      case 4:   //��������� ���� ����������� ��������
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

      case 5:   //����� ������ gsm ���� � ��������
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

      case 6:   //����� ������ gprs � ��������
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

      case 7:   //����� �������� ���������� � GSM ��������
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

      case 8:   //���������� ������� ������������ GPRS ����������
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

      case 9:   //���������� �������� ������ � ��������
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

      case 10:   //����� �������� ������ �� �������
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

      case 11:   //������ ��������� ������ �����������������
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

      case 12:   //���������� ������ ���������� 1
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

      case 13:   //���������� ������ ���������� 2
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

      case 14:   //�������� ������ �������(0 � �����������, 1 - ����� �� �������, 2 - ����� �� �������������, 3 � �����
                 //�������)
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

      case 15:   //�������� �� �������� ����� �������� ������ �� ������ (0 � �������� ��� dev, 1 � �������� ��� FM911)
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

      case 16:   //����������� ������� ����������� � �������� �������
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

      case 17:   //���������������� ������������� 1...8
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

      case 18:   //������ ������ �� ����� (�� 10, �� 180 ����)
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

      case 19:   //������������� ������������� � ������ ������
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

/* ���� ������������ ��������� �� 64 �� 128 */
const uint16_t MIN_VALUE_DOUBLE_BYTE[] = {
   MIN_VAL_LOW_POWER1,   //����� ��� � ������ ����������������� LOW PWR 1
   MIN_VAL_LOW_POWER2,   //����� ��� � ������ ����������������� LOW PWR 2
   1,   //�����, � ��������, ������ ����� ��������
   MIN_VAL_RT,   //����� ������ �� GPS � ������� Real Time
   5,   //������ �� ���������
   5,   //������ �� ����������� ��������
   0,   //������ ���������� ������, ����������
   MIN_VAL_FTIME,   //������ ������ �� �������

};
const uint16_t MAX_VALUE_DOUBLE_BYTE[] = {
   MAX_VAL_LOW_POWER1,   // 0)����� ��� � ������ ����������������� LOW PWR 1
   MAX_VAL_LOW_POWER2,   // 1)����� ��� � ������ ����������������� LOW PWR 2
   3600,   // 2)�����, � ��������, ������ ����� ��������
   MAX_VAL_RT,   // 3)����� ������ �� GPS � ������� Real Time
   65535,   // 4)������ �� ���������
   30,   // 5)������ �� ����������� ��������
   65535,   // 6)������ ���������� ������, ����������
   MAX_VAL_FTIME,   // 7)������ ������ �� �������
};

int ParseDoubleByteValue(uint8_t* pIn, int index, uint16_t Len)
{
   int n = index;
   uint8_t Id = 255;   // ID ���������
   uint32_t temp;
   uint8_t bitFree = 8;

   void (*ptrU16_Value[])(uint16_t) = {
      SetTimeLowPwrMode1,   // 0)����� ��� � ������ ����������������� LOW PWR 1
      SetTimeLowPwrMode2,   // 1)����� ��� � ������ ����������������� LOW PWR 2
      SetAccelTimeCurrState,   // 2)�����, � ��������, ������ ����� ��������
      SetGpsRealtime,   // 3)����� ������ �� GPS � ������� Real Time
      SetGpsRecordDistanse,   // 4)������ �� ���������
      SetGpsRecordMinSpeed,   // 5)������ �� ����������� ��������
      SetGpioRecordTime,   // 6)������ ���������� ������, ���������� � ��������
      SetGpsRecordtime,   // 7)������ ������ �� �������
   };

   while (1)
   {
      uint16_t value = 0;
      if (n >= (Len + sizeof(Len)))
         break;   //�������� �� ������������ �����
      Id = pIn[n];
      if (Id > 127 || Id < 64)
         break;   //������������ ���������� �����������.
      Id -= 64;
      if (Id > MAX_UI16)
         break;   //������������ ���������� �����������.
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

/* ���� ��������������� ��������� �� 128 �� 192 */
const uint32_t MIN_VALUE_WORD[] = {
   MIN_VAL_LOW_POWER1,   // 0)����� �������� � ����� ����������������� LOW PWR 1
   MIN_VAL_LOW_POWER2,   // 1)����� �������� � ����� ����������������� LOW PWR 2
   MIN_VAL_SLEEP_TIME_STANDART,   // 2)����� ��� � ����������� ������
   MIN_VAL_SLEEP_TIME_FIND,   // 3)����� ������ � ������ �����
   MIN_VAL_FIND_GPS_SAT,   // 4)����� ������ GPS ��������� � ������ �����
   600,   // 5)����� ������������� ���������� � �������� � ������ ���������� �����
   600,   // 6)����� ������������� ���������� � �������� � ������ ���������� �����
   600,   // 7)����� ������������� ���������� � �������� � ������ ���������� �����
   600,   // 8)����� ������������� ���������� � �������� � ������ ���������� �����
   600,   // 9)����� ������������� ���������� � �������� � ������ ���������� �����
   1024,   // 10)������ ������ �� ������ ��� ���������� � ����� ����������������
};
const uint32_t MAX_VALUE_WORD[] = {
   MAX_VAL_LOW_POWER1,   // 0)����� �������� � ����� ����������������� LOW PWR 1
   MAX_VAL_LOW_POWER2,   // 1)����� �������� � ����� ����������������� LOW PWR 2
   MAX_VAL_SLEEP_TIME_STANDART,   // 2)����� ��� � ����������� ������
   MAX_VAL_SLEEP_TIME_FIND,   // 3)����� ������ � ������ �����
   MAX_VAL_FIND_GPS_SAT,   // 4)����� ������ GPS ��������� � ������ �����
   86400,   // 5)����� ������������� ���������� � �������� � ������ ���������� �����
   86400,   // 6)����� ������������� ���������� � �������� � ������ ���������� �����
   86400,   // 7)����� ������������� ���������� � �������� � ������ ���������� �����
   86400,   // 8)����� ������������� ���������� � �������� � ������ ���������� �����
   86400,   // 9)����� ������������� ���������� � �������� � ������ ���������� �����
   32768,   // 10)������ ������ �� ������ ��� ���������� � ����� ����������������
};

int ParseWordValue(uint8_t* pIn, int index, uint16_t Len)
{
   int n = index;
   uint8_t Id = 255;   // ID ���������
   uint8_t bitFree = 8;

   void (*ptrU32_Value[])(uint32_t) = {
      SetTimeoutLowPwrMode1,   // 0)����� �������� � ����� ����������������� LOW PWR 1
      SetTimeoutLowPwrMode2,   // 1)����� �������� � ����� ����������������� LOW PWR 2
      SetSleepTimeStandart,   // 2)����� ��� � ����������� ������
      SetSleepTimeFind,   // 3)����� ������ � ������ �����
      SetGpsWait,   // 4)����� ������ GPS ��������� � ������ �����
      SetTimeReconnect1,   // 5)����� ������������� ���������� � �������� � ������ ���������� �����
      SetTimeReconnect2,   // 6)����� ������������� ���������� � �������� � ������ ���������� �����
      SetTimeReconnect3,   // 7)����� ������������� ���������� � �������� � ������ ���������� �����
      SetTimeReconnect4,   // 8)����� ������������� ���������� � �������� � ������ ���������� �����
      SetTimeReconnect5,   // 9)����� ������������� ���������� � �������� � ������ ���������� �����
      SetLenDataFlashReady,   // 10)������ ������ �� ������ ��� ���������� � ����� ����������������
   };

   while (1)
   {
      uint32_t value = 0;
      if (n >= (Len + sizeof(Len)))
         break;   //�������� �� ������������ �����
      Id = pIn[n];
      if (Id > 191 || Id < 128)
         break;   //������������ ���������� �����������.
      Id -= 128;
      if (Id > MAX_UI32)
         break;   //������������ ���������� �����������.
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
   SIZE_PASW,   //������ �� ������
   SIZE_TEL,   //����� �������� ������������
   SIZE_SERV,   //��� �������� �������
   SIZE_SERV,   //��� ���������� �������
   LOGIN_PASS_SIZE,   //��� ����� ������� ��� ������ � Internet(APN).
   LOGIN_PASS_SIZE,   //������ ����� �������
   LOGIN_PASS_SIZE,   //����� ����� �������
   SIZE_PIN_CODE,   //��� ��� ���������������� ��� �����
   SIZE_SERV_FTP,   //����� �������
};

const uint8_t MIN_VALUE_STRING[] = {
   MIN_VAL_SIZE_PASW,   //������ �� ������
   MIN_VAL_SIZE_TEL,   //����� �������� ������������
   NULL,   //��� �������� �������
   NULL,   //��� ���������� �������
   NULL,   //��� ����� ������� ��� ������ � Internet(APN).
   NULL,   //������ ����� �������
   NULL,   //����� ����� �������
   NULL,   //��� ��� ���������������� ��� �����
   NULL,   //����� �������
};

int ParseStringValue(uint8_t* pIn, int index, uint16_t Len)
{
   int n = index;
   uint8_t Id = 255;   // ID ���������
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
         break;   //�������� �� ������������ �����
      Id = pIn[n];
      if (Id < 192)
         break;   //��������� ���������� �����������.
      Id -= 192;
      if (Id > MAX_STR)
         break;   //��������� ���������� �����������.
      n++;
      len_string = pIn[n];   //�������� ����� ������.
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
            (*ptrSTR_Value[Id])(STR_NULL);   //�������� ������ ������
         }
         /* ����� �� ��������� ��������� ���� ������ �������� */
         if (Id == 2)
         {   // �������� ��������� ������ �������. ����� ������� SetAddrFirstServ()
            deviceDefConfig();   //����� � ��������� ���������� �������
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

/* ����� ������� ����� ������� */
int frame_build_config_packet(uint8_t ucType, char* pOut, int OffsetData)
{
   int n = OffsetData, Id;
   uint8_t bitFree = 8;

   pOut[n++] = ucType;   // ��� ������
   //����� ���� �� ���������, � ����� ��������
   n += sizeof(uint16_t);

   /* Byte Parametrs */
   for (Id = 0; Id < 64; Id++)
   {
      if ((SERVER_TRUE_VALUE[Id]) && (Id <= MAX_UI8))
      {
         pOut[n++] = Id;
         switch (Id)
         {
         case 0:   //����� �������� ����������� ��� �� �������
            pOut[n++] = (uint8_t)GetMaskMessageUser();
            break;
         case 1:   //��� � ����� �������� �������
            pOut[n++] = (uint8_t)GetUseTypeServ();
            break;
         case 2:   //������������� ���������������� �������� ��� ����������� � GPRS
            pOut[n++] = (uint8_t)GetManualModeSimFirst();
            break;
         case 3:   //������������� ��� ���� ��� ��� �����
            pOut[n++] = (uint8_t)GetPinLock();
            break;
         case 4:   //��������� ���� ����������� ��������
            pOut[n++] = (uint8_t)GetJamDetect();
            break;
         case 5:   //����� ������ gsm ���� � ��������
            pOut[n++] = (uint8_t)GetGsmFindTimeout();
            break;
         case 6:   //����� ������ gprs � ��������
            pOut[n++] = (uint8_t)GetGsmGprsTimeout();
            break;
         case 7:   //����� �������� ���������� � GSM ��������
            pOut[n++] = (uint8_t)GetLbsFindTimeout();
            break;
         case 8:   //���������� ������� ������������ GPRS ����������
            pOut[n++] = (uint8_t)GetGprsOpenCount();
            break;
         case 9:   //���������� ������� ������������ GPRS ����������
            pOut[n++] = (uint8_t)GetRoamingGprs();
            break;
         case 10:   //����� �������� ������ �� �������
            pOut[n++] = (uint8_t)GetAnswerTimeout();
            break;
         case 11:   //������ ��������� ������ �����������������
            pOut[n++] = (uint8_t)GetUserPwrDevice();
            break;
         case 12:   //���������� ������ ���������� 1
            pOut[n++] = (uint8_t)GetEnableUseLowPwr1Mode();
            break;
         case 13:   //���������� ������ ���������� 2
            pOut[n++] = (uint8_t)GetEnableUseLowPwr2Mode();
            break;
         case 14:   //�������� ������ �������(0 � �����������, 1 - ����� �� �������, 2 - ����� �� �������������, 3 �
                    //����� �������)
            pOut[n++] = (uint8_t)GetModeDevice();
            break;
         case 15:   //�������� �� �������� ����� �������� ������ �� ������ (0 � �������� ��� dev, 1 � �������� ���
                    //FM911)
            pOut[n++] = (uint8_t)GetModeProtocol();
            break;
         case 16:   //����������� ������� ����������� � �������� �������.
            pOut[n++] = GetMinTemperaturWorkDevice();
            break;
         case 17:   //���������������� �������������.
            pOut[n++] = (uint8_t)GetAccelSensitivity();
            break;
         case 18:   //������ ������ �� �����.
            pOut[n++] = (uint8_t)GetGpsRecordCourse();
            break;
         case MAX_UI8:   //������������� ������������� � ������ ������.
            pOut[n++] = (uint8_t)GetEnableAccelToFind();
            break;
         }
      }
   }
   /*******************************/

   /* Double Byte Parametrs */
   uint16_t (*ptrU16_Value[])(void) = {
      GetTimeLowPwrMode1,   // 0)����� ��� � ������ ����������������� LOW PWR 1
      GetTimeLowPwrMode2,   // 1)����� ��� � ������ ����������������� LOW PWR 2
      GetAccelTimeCurrState,   // 2)�����, � ��������, ������ ����� ��������
      GetGpsRealtime,   // 3)����� ������ �� GPS � ������� Real Time
      GetGpsRecordDistanse,   // 4)������ �� ���������
      GetGpsRecordMinSpeed,   // 5)������ �� ����������� ��������
      GetGpioRecordTime,   // 6)������ ���������� ������, ���������� � ��������
      GetGpsRecordtime,   // 7)������ ������ �� �������
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
      GetTimeoutLowPwrMode1,   // 0) ����� �������� � ����� ����������������� LOW PWR 1
      GetTimeoutLowPwrMode2,   // 1) ����� �������� � ����� ����������������� LOW PWR 2
      GetSleepTimeStandart,   // 2) ����� ��� � ����������� ������
      GetSleepTimeFind,   // 3) ����� ������ � ������ �����
      GetGpsWait,   // 4) ����� ������ GPS ��������� � ������ �����
      GetTimeReconnect1,   // 5) ����� ������������� ���������� � �������� � ������ ���������� �����
      GetTimeReconnect2,   // 6) ����� ������������� ���������� � �������� � ������ ���������� �����
      GetTimeReconnect3,   // 7) ����� ������������� ���������� � �������� � ������ ���������� �����
      GetTimeReconnect4,   // 8) ����� ������������� ���������� � �������� � ������ ���������� �����
      GetTimeReconnect5,   // 9) ����� ������������� ���������� � �������� � ������ ���������� �����
      GetLenDataFlashReady,   // 10) ������ ������ �� ������ ��� ���������� � ����� ����������������
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

   /* �������� ����� */
   uint16_t len = n - OffsetData - sizeof(uint16_t);
   bit_packing(pOut + OffsetData + 1, len, &bitFree, 16);
   return n - OffsetData;
}
