#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "includes.h"
#include "cmd_parser.h"
#include "cmd_func.h"
#include "periph_general.h"

// VER
int CmdGetNameFirmvare(u8* pansw, u8* parg, u16 len)
{
   RTC_t DateCurrentFirmware;

   if ((parg[0] == '?') && (len == 1))
   {
      Sec2Date(&DateCurrentFirmware, flash_read_word(__CONST_FIRM_VER));

      sprintf((char*)(pansw), "HW: %s%s\r\n", DEV_VER, HW_VER);
      sprintf((char*)(pansw + strlen((char*)pansw)), "CURRENT: U01.%s", DEV_VER);

      sprintf((char*)(pansw + strlen((char*)pansw)), "%s.", HW_VER);
      sprintf((char*)(pansw + strlen((char*)pansw)), "%.2i", DateCurrentFirmware.year);
      sprintf((char*)(pansw + strlen((char*)pansw)), "%.2i", DateCurrentFirmware.month);
      sprintf((char*)(pansw + strlen((char*)pansw)), "%.2i", DateCurrentFirmware.mday);
      sprintf((char*)(pansw + strlen((char*)pansw)), "%.2i", DateCurrentFirmware.hour);
      sprintf((char*)(pansw + strlen((char*)pansw)), "%.2i", DateCurrentFirmware.min);
      sprintf((char*)(pansw + strlen((char*)pansw)), "%.2i\r\n", DateCurrentFirmware.sec);

      sprintf((char*)(pansw + strlen((char*)pansw)), "UPDATE: U01.%s", DEV_VER);
      sprintf((char*)(pansw + strlen((char*)pansw)), "%s.", HW_VER);
      sprintf((char*)(pansw + strlen((char*)pansw)), "%.2i", DateCurrentFirmware.year);
      sprintf((char*)(pansw + strlen((char*)pansw)), "%.2i", DateCurrentFirmware.month);
      sprintf((char*)(pansw + strlen((char*)pansw)), "%.2i", DateCurrentFirmware.mday);
      sprintf((char*)(pansw + strlen((char*)pansw)), "%.2i", DateCurrentFirmware.hour);
      sprintf((char*)(pansw + strlen((char*)pansw)), "%.2i", DateCurrentFirmware.min);
      sprintf((char*)(pansw + strlen((char*)pansw)), "%.2i", DateCurrentFirmware.sec);
      sprintf((char*)(pansw + strlen((char*)pansw)), "(%d)\r\n", flash_read_word(__CONST_FIRM_VER));

      return strlen((char*)pansw);
   }
   return CMD_ERR_CMD;
}

int CmdUserTel(u8* pansw, u8* parg, u16 len)
{
   char strTel[SIZE_TEL] = { '\0' };
   if (parg[0] == '?' && len == 1)
   {
      GetUserTel(strTel);
      if (strlen(strTel))
      {
         sprintf((char*)pansw, "TEL#EMRG=+%s\r\n", strTel);
      }
      else
      {
         sprintf((char*)pansw, "TEL#EMRG=NULL\r\n");
      }
      return strlen((char*)pansw);
   }

   if (len < MIN_VAL_SIZE_TEL || len > SIZE_TEL)
   {
      return CMD_ERR_LIM;
   }

   if (parg[0] != '+' || parg[len] != ';')
   {
      return CMD_ERR_CMD;
   }

   for (size_t i = 1; i < len; i++)
   {
      if (parg[i] > '9' || parg[i] < '0')
         return CMD_ERR_CMD;
   }

   parg[len] = '\0';
   strcpy(strTel, (char*)&parg[1]);
   SetUserTel(strTel);
   SaveConfigCMD();
   return strlen((char*)pansw);
}

int CmdPinSimCard(u8* pansw, u8* parg, u16 len)
{
   char strPin[SIZE_PIN_CODE] = { '\0' };   // User Pin Code SIM.
   if ((parg[0] == '?') && (len == 1))
   {
      if (GetUserSimPin(strPin))
      {
         sprintf((char*)pansw, "GSM#1#PIN=%s\r\n", strPin);
      }
      else
      {
         sprintf((char*)pansw, "GSM#1#PIN=NULL\r\n");
      }
      return strlen((char*)pansw);
   }

   if (strstr((char*)parg, "NULL"))
   {
      SetUserSimPin(strPin);
      SaveConfigCMD();
      return strlen((char*)pansw);
   }
   if ((len < 4) || (len > SIZE_PIN_CODE))
   {
      return CMD_ERR_LIM;
   }

   for (uint16_t i = 0; i < len; i++)
   {
      if ((parg[i] > '9') || (parg[i] < '0'))
         return CMD_ERR_CMD;
   }
   strcpy(strPin, (char*)parg);
   SetUserSimPin(strPin);
   SaveConfigCMD();
   return strlen((char*)pansw);
}

int CmdResetDevice(u8* pansw, u8* parg, u16 len)
{
   if (len)
   {
      return CMD_ERR_CMD;
   }

   SetStatusReset(CMD_RESET);
   SetStatusDeviceReset(GetStatusReset());
   sprintf((char*)pansw, "CMD RESET DEVICE\r\n");
   setFlagReset();
   return strlen((char*)pansw);
}

int CmdResetConfig(u8* pansw, u8* parg, u16 len)
{
   if (len)
   {
      return CMD_ERR_CMD;
   }

   deviceDefConfig();

   SetStatusReset(CMD_RESET);
   SetStatusDeviceReset(GetStatusReset());
   sprintf((char*)pansw, "RESET CONFIG\r\n");
   setGsmStep(GSM_PROFILE_GPRS_CONNECT);
   return strlen((char*)pansw);
}

int CmdFilterCourse(u8* pansw, u8* parg, u16 len)
{
   if (len == 0)
   {
      return CMD_ERR_LIM;
   }

   if (parg[0] == '?')
   {
      sprintf((char*)pansw, "GPS#F1=%d\r\n", GetGpsRecordCourse());
      return strlen((char*)pansw);
   }

   for (uint16_t i = 0; i < len; i++)
   {
      if ((parg[i] > '9') || (parg[i] < '0'))
         return CMD_ERR_CMD;
   }

   uint32_t uiCourse = (uint32_t)atof((char*)parg);

   if ((uiCourse > 180) || ((uiCourse < 10) && (uiCourse != 0)))
   {
      return CMD_ERR_LIM;
   }

   SetGpsRecordCourse(uiCourse);
   SaveConfigCMD();
   return strlen((char*)pansw);
}

int CmdFilterDistance(u8* pansw, u8* parg, u16 len)
{
   if (len == 0)
      return CMD_ERR_LIM;

   if (parg[0] == '?')
   {
      sprintf((char*)pansw, "GPS#F2=%d\r\n", GetGpsRecordDistanse());
      return strlen((char*)pansw);
   }

   for (uint16_t i = 0; i < len; i++)
   {
      if (parg[i] > '9' || parg[i] < '0')
         return CMD_ERR_CMD;
   }

   uint32_t uiDist = (uint32_t)atof((char*)parg);

   if ((uiDist > 65535) || ((uiDist < 5) && (uiDist != 0)))
   {
      return CMD_ERR_LIM;
   }

   SetGpsRecordDistanse(uiDist);
   SaveConfigCMD();
   return strlen((char*)pansw);
}

int CmdFilterMinSpeed(u8* pansw, u8* parg, u16 len)
{
   if (len == 0)
      return CMD_ERR_LIM;

   if (parg[0] == '?')
   {
      sprintf((char*)pansw, "GPS#F4=%d\r\n", GetGpsRecordMinSpeed());
      return strlen((char*)pansw);
   }

   for (uint16_t i = 0; i < len; i++)
   {
      if ((parg[i] > '9') || (parg[i] < '0'))
         return CMD_ERR_CMD;
   }

   uint32_t uiMinSpeed = (uint32_t)atof((char*)parg);

   if ((uiMinSpeed > 30) || ((uiMinSpeed < 5) && (uiMinSpeed != 0)))
   {
      return CMD_ERR_LIM;
   }

   SetGpsRecordMinSpeed(uiMinSpeed);
   SaveConfigCMD();
   return strlen((char*)pansw);
}

int CmdTimeFindGps(u8* pansw, u8* parg, u16 len)
{
   if (len == 0)
      return CMD_ERR_LIM;

   if (parg[0] == '?')
   {
      sprintf((char*)pansw, "GPS#T2=%d\r\n", GetGpsWait());
      return strlen((char*)pansw);
   }

   for (uint16_t i = 0; i < len; i++)
   {
      if ((parg[i] > '9') || (parg[i] < '0'))
         return CMD_ERR_CMD;
   }

   uint32_t uiTimeFindGps = (uint32_t)atof((char*)parg);

   if ((uiTimeFindGps > MAX_VAL_FIND_GPS_SAT) || ((uiTimeFindGps < MIN_VAL_FIND_GPS_SAT) && (uiTimeFindGps != 0)))
   {
      return CMD_ERR_LIM;
   }

   SetGpsWait(uiTimeFindGps);
   SaveConfigCMD();
   return strlen((char*)pansw);
}

int CmdEraseArchive(u8* pansw, u8* parg, u16 len)
{
   if (!(len))
   {
      OnChangeArchiveErase();
      sprintf((char*)pansw, "TRACK ERASE\r\n");
      return strlen((char*)pansw);
   }
   return CMD_ERR_CMD;
}

int CmdEmrg(u8* pansw, u8* parg, u16 len)
{
   if ((parg[0] == '?') && (len == 1))
   {
      TYPE_MESSAGE eMaskMessage = GetMaskMessageUser();
      if (!(eMaskMessage))
      {
         sprintf((char*)pansw, "EMRG#M=0\r\n");
         return strlen((char*)pansw);
      }
      uint8_t temp = 0;
      if (eMaskMessage & fBUTTON_MESS_TEL)
         temp |= 1;
      if (eMaskMessage & fBUTTON_CALL_TEL)
         temp |= 2;
      sprintf((char*)pansw, "EMRG#M=%i\r\n", temp);
      return strlen((char*)pansw);
   }

   if ((len != 1) || (parg[0] > 0x33) || (parg[0] < 0x30))
      return CMD_ERR_LIM;
   TYPE_MESSAGE mask_but_and_call = fBUTTON_MESS_TEL;
   mask_but_and_call |= fBUTTON_CALL_TEL;
   _Bool cmd_true;
   switch (parg[0])
   {
   case '0':
      SetMaskMessageUser(NO_MESS);
      cmd_true = TRUE;
      break;
   case '1':
      SetMaskMessageUser(fBUTTON_MESS_TEL);
      cmd_true = TRUE;
      break;
   case '2':
      SetMaskMessageUser(fBUTTON_CALL_TEL);
      cmd_true = TRUE;
      break;
   case '3':
      SetMaskMessageUser(mask_but_and_call);
      cmd_true = TRUE;
      break;
   default:
      cmd_true = FALSE;
   }
   if (cmd_true)
   {
      SaveConfigCMD();
      return strlen((char*)pansw);
   }
   return CMD_ERR_CMD;
}

/* Смена сервера и типа протокола с 911 на dev(выполняется только если СИМ карта не заводская) */
int CmdChangeServAndProt(u8* pansw, u8* parg, u16 len)
{
   char str_name_first_server[SIZE_SERV] = { '\0' };
   if (len >= SIZE_SERV)
   {
      return CMD_ERR_LIM;
   }

   if ((parg[0] == '?') && (len == 1))
   {
      GetAddrFirstServ(str_name_first_server);
      sprintf((char*)pansw, "SRV#1=%s\r\n", str_name_first_server);
      return strlen((char*)pansw);
   }

   if (parg[len] == ';')
   {
      parg[len] = '\0';
   }

   char* pFindNULL = strstr((char*)parg, "NULL");
   if (pFindNULL)
   {
      return CMD_ERR_LIM;
   }

   if ((GetModeProtocol() == FINDME_911) && (GetTypeRegBase911() == TYPE_REG_TO_BASE) ||
       (GetTypeRegBase911() == TYPE_REG_USER))
   {
      //Если девайс не зареган на 911
      return CMD_ERR_ACCESS;
   }

   SetAddrFirstServ((const char*)parg);   //устанавливаем сервер
   deviceDefConfig();   //сброс к дефолтным настройкам девайса
   if (GetModeProtocol() == FINDME_911)
   {
      SetTypeRegBase911(TYPE_GOODBYE);   //ставим тип соединения с 911 для отправки последних данных
   }
   setGsmStep(GSM_PROFILE_GPRS_CONNECT);

   SaveConfigCMD();
   return strlen((char*)pansw);
}

int CmdPwrMode(u8* pansw, u8* parg, u16 len)
{
   if ((parg[0] == '?') && (len == 1))
   {
      int iUserPwrDevice = (int)GetUserPwrDevice();
      sprintf((char*)(pansw + strlen((char*)pansw)), "PW#MODE=%i\r\n", iUserPwrDevice);
      return strlen((char*)pansw);
   }

   if ((parg[0] >= (USER_POWER_AUTO + 0x30)) && (parg[0] <= (USER_POWER_LOW_PWR2_MODE + 0x30)) && (len == 1))
   {
      SetUserPwrDevice((USER_PWR_STATUS)(parg[0] - 0x30));
      SaveConfigCMD();
      return strlen((char*)pansw);
   }

   return CMD_ERR_CMD;
}

int CmdSetTimeModeLowPwr1(u8* pansw, u8* parg, u16 len)
{
   if (len > 4)
      return CMD_ERR_LIM;
   if ((parg[0] == '?') && (len == 1))
   {
      sprintf((char*)pansw, "PW#TW1=%d\r\n", GetTimeoutLowPwrMode1());
      return strlen((char*)pansw);
   }

   for (uint16_t i = 0; i < len; i++)
   {
      if ((parg[i] > '9') || (parg[i] < '0'))
         return CMD_ERR_CMD;
   }

   uint32_t uiTime = (uint32_t)atof((char*)parg);

   if ((uiTime >= GetTimeoutLowPwrMode2()) || ((uiTime < MIN_VAL_LOW_POWER1) || (uiTime > MAX_VAL_LOW_POWER1)))
   {
      return CMD_ERR_LIM;
   }

   SetTimeoutLowPwrMode1(uiTime);
   SaveConfigCMD();
   return strlen((char*)pansw);
}

int CmdSetTimeModeLowPwr2(u8* pansw, u8* parg, u16 len)
{
   if (len > 4)
      return CMD_ERR_LIM;

   if ((parg[0] == '?') && (len == 1))
   {
      sprintf((char*)pansw, "PW#TW2=%d\r\n", GetTimeoutLowPwrMode2());
      return strlen((char*)pansw);
   }

   for (uint16_t i = 0; i < len; i++)
   {
      if ((parg[i] > '9') || (parg[i] < '0'))
         return CMD_ERR_CMD;
   }

   uint32_t uiTime = (uint32_t)atof((char*)parg);

   if ((uiTime <= GetTimeoutLowPwrMode1()) || (uiTime < MIN_VAL_LOW_POWER2) || (uiTime > MAX_VAL_LOW_POWER2))
   {
      return CMD_ERR_LIM;
   }

   SetTimeoutLowPwrMode2(uiTime);
   SaveConfigCMD();
   return strlen((char*)pansw);
}

int InfoGsmModemImei(u8* pansw, u8* parg, u16 len)
{
   if ((parg[0] == '?') && (len == 1))
   {
      char strIMEI[SIZE_IMEI];
      GetStrIMEI(strIMEI);
      sprintf((char*)pansw, "GSM#IMEI=%s\r\n", strIMEI);
      return strlen((char*)pansw);
   }
   return CMD_ERR_CMD;
}

int InfoSimCardScidFirst(u8* pansw, u8* parg, u16 len)
{
   if ((parg[0] == '?') && (len == 1))
   {
      sprintf((char*)pansw, "GSM#1#SCID=%s\r\n", g_stRam.stSim.strFIRST_SCID);
      return strlen((char*)pansw);
   }
   return CMD_ERR_CMD;
}

int InfoSimCardScidSecond(u8* pansw, u8* parg, u16 len)
{
   char strSECOND_SCID[SIZE_SCID] = { '\0' };
   if ((parg[0] == '?') && (len == 1))
   {
      if (GetScidCurentSecondSim(strSECOND_SCID))
      {
         sprintf((char*)pansw, "GSM#2#SCID=%s\r\n", strSECOND_SCID);
      }
      else
      {
         sprintf((char*)pansw, "GSM#2#SCID=NULL\r\n");
      }
      return strlen((char*)pansw);
   }
   return CMD_ERR_CMD;
}

int CmdSensitivityAccel(u8* pansw, u8* parg, u16 len)
{
   if (len != 1)
      return CMD_ERR_LIM;

   if ((parg[0] == '?'))
   {
      sprintf((char*)pansw, "MOVE#F2=%i\r\n", GetAccelSensitivity());
      return strlen((char*)pansw);
   }

   for (uint16_t i = 0; i < len; i++)
   {
      if ((parg[i] > '9') || (parg[i] < '0'))
         return CMD_ERR_CMD;
   }

   int ucSensitivity = atoi((char*)parg);
   if ((ucSensitivity < 1) || (ucSensitivity > 8))
      return CMD_ERR_LIM;

   SetAccelSensitivity(ucSensitivity);
   SaveConfigCMD();
   DPS("-ACC CMD INIT-\r\n");
   getFlagReset();
   pansw += sprintf((char*)pansw, "OK\r\n");
   return strlen((char*)pansw);
}

//Отладочные данные по GPS
int CmdDebugGps(u8* pansw, u8* parg, u16 len)
{
   RTC_t GpsData;
   // char strTemp[50];
   int ucDeg_lt = 0;
   int ucDeg_lg = 0;
   int uiDeg_lt_fr = 0;
   int uiDeg_lg_fr = 0;
   GPS_INFO stGpsDataTemp;
   if (!(GetPositionGps(&stGpsDataTemp)))
   {
      memset(&stGpsDataTemp, 0, sizeof(GPS_INFO));
   }

   float latitude = ConvertLatitudeGpsFindMe((float)stGpsDataTemp.latitude);
   float longitude = ConvertLongitudeGpsFindMe((float)stGpsDataTemp.longitude);

   pansw[0] = 0;
   Sec2Date(&GpsData, stGpsDataTemp.time);

   /**************************************************/

   if ((stGpsDataTemp.latitude) && (stGpsDataTemp.longitude))
   {
      /* Преобразование координат к форме google maps */
      ucDeg_lt = (int)latitude / 100;
      uiDeg_lt_fr = (int)((latitude - ucDeg_lt * 100) * 1000 * 1000) / 60;
      ucDeg_lg = (int)longitude / 100;
      uiDeg_lg_fr = (int)((longitude - ucDeg_lg * 100) * 1000 * 1000) / 60;
      /**************************************************/
   }

   if ((parg[0] == '?') && (len == 1))
   {
      sprintf((char*)pansw, "ANT=int\r\n");
      sprintf((char*)(pansw + strlen((char*)pansw)), "VAL=%d\r\n", GetGpsStatus());
      sprintf((char*)(pansw + strlen((char*)pansw)), "HDOP=%.1f\r\n", stGpsDataTemp.hdop);

      sprintf((char*)(pansw + strlen((char*)pansw)), "TIME=%.2d/", GpsData.mday);
      sprintf((char*)(pansw + strlen((char*)pansw)), "%.2d/", GpsData.month);
      if (GetGpsStatus())
      {
         sprintf((char*)(pansw + strlen((char*)pansw)), "20%d ", GpsData.year);
      }
      else
      {
         sprintf((char*)(pansw + strlen((char*)pansw)), "19%d ", GpsData.year);
      }
      int eUserPwrDevice = (int)GetUserPwrDevice();
      sprintf((char*)(pansw + strlen((char*)pansw)), "%.2d:", GpsData.hour);
      sprintf((char*)(pansw + strlen((char*)pansw)), "%.2d:", GpsData.min);
      sprintf((char*)(pansw + strlen((char*)pansw)), "%.2d\r\n", GpsData.sec);

      sprintf((char*)(pansw + strlen((char*)pansw)), "LAT=%02d.", ucDeg_lt);
      sprintf((char*)(pansw + strlen((char*)pansw)), "%06d,S\r\n", uiDeg_lt_fr);
      sprintf((char*)(pansw + strlen((char*)pansw)), "LON=%03d.", ucDeg_lg);
      sprintf((char*)(pansw + strlen((char*)pansw)), "%06d,W\r\n", uiDeg_lg_fr);
      sprintf((char*)(pansw + strlen((char*)pansw)), "CURS=%.2f\r\n", stGpsDataTemp.course);
      sprintf((char*)(pansw + strlen((char*)pansw)), "SPEED=%.2f\r\n", stGpsDataTemp.speed);
      sprintf((char*)(pansw + strlen((char*)pansw)), "SAT=%d\r\n", stGpsDataTemp.sat);
      sprintf((char*)(pansw + strlen((char*)pansw)), "LPM=%d\r\n", eUserPwrDevice);
      return strlen((char*)pansw);
   }

   return CMD_ERR_CMD;
}

//Отладочные данные по GSM
int CmdDebugGsm(u8* pansw, u8* parg, u16 len)
{
   if ((parg[0] == '?') && (len == 1))
   {
      switch (g_stRam.stSim.eRegStatus)
      {
      case SIM_WAIT:
         sprintf((char*)pansw, "SIM=1\r\n 1:down\r\n");
         break;

      case SIM_ERROR:
         sprintf((char*)pansw, "SIM=1\r\n 1:error,NONE\r\n");
         break;

      case SIM_NO_READY:
         sprintf((char*)pansw, "SIM=1\r\n 1:error,NONE\r\n");
         break;

      case SIM_PIN:
         sprintf((char*)pansw, "SIM=1\r\n 1:error,ACCESS\r\n");
         break;

      case SIM_PIN_NO_SET:
         sprintf((char*)pansw, "SIM=1\r\n 1:error,PIN\r\n");
         break;

      case SIM_PUK:
         sprintf((char*)pansw, "SIM=1\r\n 1:error,PUK\r\n");
         break;

      case SIM_PIN_GUESSING:
         sprintf((char*)pansw, "SIM=1\r\n 1:error,PUK\r\n");
         break;

      default:
         sprintf((char*)pansw, "SIM=1\r\n 1:ok\r\n");
      }

      sprintf((char*)(pansw + strlen((char*)pansw)), "ANT=int\r\n");

      sprintf((char*)(pansw + strlen((char*)pansw)), "CSQ=%d\r\n", GetCSQ());

      switch (g_stRam.stSim.eRegStatus)
      {
      case HOME_NET:
         sprintf((char*)(pansw + strlen((char*)pansw)), "REG=ok\r\n");
         break;

      case ROAMING_NET:
         sprintf((char*)(pansw + strlen((char*)pansw)), "REG=roaming\r\n");
         break;

      case FIND_NET:
         sprintf((char*)(pansw + strlen((char*)pansw)), "REG=search\r\n");
         break;

      default:
         sprintf((char*)(pansw + strlen((char*)pansw)), "REG=down\r\n");
      }

      if (strlen(g_stRam.stSim.acMobCountCode))
      {
         sprintf((char*)(pansw + strlen((char*)pansw)), "OPS=%s\r\n", g_stRam.stSim.acMobCountCode);
      }
      else
      {
         sprintf((char*)(pansw + strlen((char*)pansw)), "OPS=0\r\n");
      }

      if (g_stRam.stSim.bGprsTrue == ON)
      {
         sprintf((char*)(pansw + strlen((char*)pansw)), "GPRS=up\r\n");
      }
      else
      {
         sprintf((char*)(pansw + strlen((char*)pansw)), "GPRS=down\r\n");
      }

      int eUserPwrDevice = (int)GetUserPwrDevice();
      sprintf((char*)(pansw + strlen((char*)pansw)), "LPM=%d\r\n", eUserPwrDevice);

      return strlen((char*)pansw);
   }

   return CMD_ERR_CMD;
}

/* Update Firmware Command */
int CmdFirmLoad(u8* pansw, u8* parg, u16 len)
{
   /* Проверяем длину символов */
   if ((!(len)) && (len > (SIZE_NAME_FIRMWARE - 1)))
   {
      return CMD_ERR_LIM;
   }

   /* Проверяем корректность прошивки */
   if (parg[len] == ';')
   {
      parg[len] = '\0';
   }
   extern const uint32_t FIRMWARE;
   uint32_t firm_cur = FIRMWARE;
   uint32_t firm_upd = (uint32_t)atof((char*)parg);

   if (firm_cur != firm_upd)
   {
      char strNameNewFirm[SIZE_SERV_FTP] = { '\0' };
      GetDefAddrFirmSer(strNameNewFirm);
      char* ptr = strNameNewFirm;
      sprintf(ptr + strlen(strNameNewFirm), "%s", parg);
      SetAddrFirmSer(strNameNewFirm);
      setGsmStep(GSM_PROFILE_GPRS_CONNECT);
   }

   return strlen((char*)pansw);
}

#ifdef _DRIVE_SIMPLE_
extern TEepConfig g_stEepConfig;
int ds_init(uint8_t* pansw, uint8_t* parg, uint16_t len)
{
   u8* pst = pansw;

   DSm_ConfigInit(&g_stEepConfig.stDSm_Data);
   DSm_Init();
   SaveConfigCMD();

   return pansw - pst;
}

int ds_print_calib(uint8_t* pansw, uint8_t* parg, uint16_t len)
{
   u8* pst = pansw;

   if (parg[0] == 'B')
   {
      for (u8 i = 0; i < DSM_CALIB_VEC_NUM; i++)
         pansw += sprintf((char*)pansw,
                          "B X:%i\t Y:%i\t Z:%i\r\n",
                          g_stEepConfig.stDSm_Data.stCalib.brake_vector[i][0],
                          g_stEepConfig.stDSm_Data.stCalib.brake_vector[i][1],
                          g_stEepConfig.stDSm_Data.stCalib.brake_vector[i][2]);
   }
   if (parg[0] == 'G')
   {
      for (u8 i = 0; i < DSM_CALIB_VEC_NUM; i++)
         pansw += sprintf((char*)pansw,
                          "G X:%i\t Y:%i\t Z:%i\r\n",
                          g_stEepConfig.stDSm_Data.stCalib.g_vector[i][0],
                          g_stEepConfig.stDSm_Data.stCalib.g_vector[i][1],
                          g_stEepConfig.stDSm_Data.stCalib.g_vector[i][2]);
   }
   if (parg[0] == 'N')
   {
      pansw += sprintf((char*)pansw,
                       "brake n: %i\t G n: %i\r\n",
                       g_stEepConfig.stDSm_Data.stCalib.brake_num,
                       g_stEepConfig.stDSm_Data.stCalib.g_num);
   }
   return pansw - pst;
}
#endif
