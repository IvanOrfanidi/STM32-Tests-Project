#include "includes.h"
#include "global_eep.h"

TEepConfig g_stEepConfig;   //—труктура EEP с настройками.

void SetUserTel(const char* ptr)
{
   strcpy(g_stEepConfig.stUser.strTel, ptr);
}

int GetUserTel(char* ptr)
{
   if (strlen(g_stEepConfig.stUser.strTel) == 0)
      return 0;
   strcpy(ptr, g_stEepConfig.stUser.strTel);
   return strlen(g_stEepConfig.stUser.strTel);
}

void SetRoamingGprs(_Bool bRoamingGprs)
{
   g_stEepConfig.stGsm.bRoamingGprsEnable = bRoamingGprs;
}
_Bool GetRoamingGprs(void)
{
   return g_stEepConfig.stGsm.bRoamingGprsEnable;
}

void SetAccelSensitivity(uint8_t ucSensitivity)
{
   g_stEepConfig.stAccel.ucSensitivity = ucSensitivity;
}

uint8_t GetAccelSensitivity(void)
{
   return g_stEepConfig.stAccel.ucSensitivity;
}

void SetAccelTimeCurrState(uint16_t usTimeCurrState)
{
   g_stEepConfig.stAccel.usTimeCurrState = usTimeCurrState;
}

uint16_t GetAccelTimeCurrState(void)
{
   return g_stEepConfig.stAccel.usTimeCurrState;
}

void SetAddrFirstServ(const char* ptr)
{
   strcpy(g_stEepConfig.stConnect.str_name_first_server, ptr);
}

int GetAddrFirstServ(char* ptr)
{
   if (strlen(g_stEepConfig.stConnect.str_name_first_server) == 0)
      return 0;
   strcpy(ptr, g_stEepConfig.stConnect.str_name_first_server);
   return strlen(g_stEepConfig.stConnect.str_name_first_server);
}

void SetAddrSecondServ(const char* ptr)
{
   strcpy(g_stEepConfig.stConnect.str_name_second_server, ptr);
}

int GetAddrSecondServ(char* ptr)
{
   if (strlen(g_stEepConfig.stConnect.str_name_second_server) == 0)
      return 0;
   strcpy(ptr, g_stEepConfig.stConnect.str_name_second_server);
   return strlen(g_stEepConfig.stConnect.str_name_second_server);
}

void SetUseTypeServ(TYPE_SERVER eUseTypeServ)
{
   g_stEepConfig.stConnect.eUseNumSer = eUseTypeServ;
}

TYPE_SERVER GetUseTypeServ(void)
{
   return g_stEepConfig.stConnect.eUseNumSer;
}

void SetNameUserSimApn(const char* ptr)
{
   strcpy(g_stEepConfig.stSim.strApnFirst, ptr);
}

int GetNameUserSimApn(char* ptr)
{
   if (strlen(g_stEepConfig.stSim.strApnFirst) == 0)
      return 0;
   strcpy(ptr, g_stEepConfig.stSim.strApnFirst);
   return strlen(g_stEepConfig.stSim.strApnFirst);
}

void SetNameUserSimPass(const char* ptr)
{
   strcpy(g_stEepConfig.stSim.strPasswordFirst, ptr);
}

int GetNameUserSimPass(char* ptr)
{
   if (strlen(g_stEepConfig.stSim.strPasswordFirst) == 0)
      return 0;
   strcpy(ptr, g_stEepConfig.stSim.strPasswordFirst);
   return strlen(g_stEepConfig.stSim.strPasswordFirst);
}

void SetNameUserSimLogin(const char* ptr)
{
   strcpy(g_stEepConfig.stSim.strLoginFirst, ptr);
}

int GetNameUserSimLogin(char* ptr)
{
   if (strlen(g_stEepConfig.stSim.strLoginFirst) == 0)
      return 0;
   strcpy(ptr, g_stEepConfig.stSim.strLoginFirst);
   return strlen(g_stEepConfig.stSim.strLoginFirst);
}

void setNameUserSecondSimApn(const char* ptr)
{
   strcpy(g_stEepConfig.stSim.strApnSecond, ptr);
}

size_t getNameUserSecondSimApn(char* ptr)
{
   if (strlen(g_stEepConfig.stSim.strApnSecond) == 0)
      return 0;
   strcpy(ptr, g_stEepConfig.stSim.strApnSecond);
   return strlen(g_stEepConfig.stSim.strApnSecond);
}

void setNameUserSecondSimPass(const char* ptr)
{
   strcpy(g_stEepConfig.stSim.strPasswordSecond, ptr);
}

size_t getNameUserSecondSimPass(char* ptr)
{
   if (strlen(g_stEepConfig.stSim.strPasswordSecond) == 0)
      return 0;
   strcpy(ptr, g_stEepConfig.stSim.strPasswordSecond);
   return strlen(g_stEepConfig.stSim.strPasswordSecond);
}

void setNameUserSecondSimLogin(const char* ptr)
{
   strcpy(g_stEepConfig.stSim.strLoginSecond, ptr);
}

size_t getNameUserSecondSimLogin(char* ptr)
{
   if (strlen(g_stEepConfig.stSim.strLoginSecond) == 0)
      return 0;
   strcpy(ptr, g_stEepConfig.stSim.strLoginSecond);
   return strlen(g_stEepConfig.stSim.strLoginSecond);
}

void SetManualModeSimFirst(_Bool bManualMode)
{
   g_stEepConfig.stSim.bManualModeSimFirst = bManualMode;
}

_Bool GetManualModeSimFirst(void)
{
   return g_stEepConfig.stSim.bManualModeSimFirst;
}

void SetManualModeSimSecond(_Bool bManualMode)
{
   g_stEepConfig.stSim.bManualModeSimSecond = bManualMode;
}

_Bool GetManualModeSimSecond(void)
{
   return g_stEepConfig.stSim.bManualModeSimSecond;
}

/* ”станавливаем флаг что установлена резервна€ SIM карта и задействуем резервный канал св€зи */
void setFlagSimSecondInstld(_Bool value)
{
   g_stEepConfig.stSim.flagSimSecondInstld = value;
}

/* ѕолучаем флаг что установлена резервна€ SIM карта и задействуем резервный канал св€зи */
_Bool getFlagSimSecondInstld(void)
{
   return g_stEepConfig.stSim.flagSimSecondInstld;
}

void SetUserSimPin(const char* ptr)
{
   if (strlen(ptr))
   {
      strcpy(g_stEepConfig.stSim.strPin, ptr);
   }
   else
   {
      memset(g_stEepConfig.stSim.strPin, 0, sizeof(g_stEepConfig.stSim.strPin));
   }
}

int GetUserSimPin(char* ptr)
{
   if (strlen(g_stEepConfig.stSim.strPin) == 0)
      return 0;
   strcpy(ptr, g_stEepConfig.stSim.strPin);
   return strlen(g_stEepConfig.stSim.strPin);
}

void SetPinLock(_Bool bPinLock)
{
   g_stEepConfig.stSim.bPinLock = bPinLock;
}

_Bool GetPinLock(void)
{
   return g_stEepConfig.stSim.bPinLock;
}

void SetJamDetect(_Bool bJamDetect)
{
   g_stEepConfig.stSim.bJamDetect = bJamDetect;
}

_Bool GetJamDetect(void)
{
   return g_stEepConfig.stSim.bJamDetect;
}

void SetScidFirstSim(const char* ptr)
{
   strcpy(g_stEepConfig.stSim.strFIRST_SCID, ptr);
}

int GetScidFirstSim(char* ptr)
{
   if (strlen(g_stEepConfig.stSim.strFIRST_SCID) == 0)
      return 0;
   strcpy(ptr, g_stEepConfig.stSim.strFIRST_SCID);
   return strlen(g_stEepConfig.stSim.strFIRST_SCID);
}

void SetImeiFirstGsm(const char* ptr)
{
   strcpy(g_stEepConfig.stSim.strFIRST_IMEI, ptr);
}

int GetImeiFirstGsm(char* ptr)
{
   if (strlen(g_stEepConfig.stSim.strFIRST_IMEI) == 0)
      return 0;
   strcpy(ptr, g_stEepConfig.stSim.strFIRST_IMEI);
   return strlen(g_stEepConfig.stSim.strFIRST_IMEI);
}

void SetAccessPass(const char* ptr)
{
   if (strlen(ptr))
   {
      strcpy(g_stEepConfig.stUser.strPassword, ptr);
   }
}

int GetAccessPass(char* ptr)
{
   if (strlen(g_stEepConfig.stUser.strPassword) == 0)
      return 0;
   strcpy(ptr, g_stEepConfig.stUser.strPassword);
   return strlen(g_stEepConfig.stUser.strPassword);
}

void SetMaskMessageUser(TYPE_MESSAGE eMaskMessage)
{
   g_stEepConfig.stUser.eMaskMessage = eMaskMessage;
}

TYPE_MESSAGE GetMaskMessageUser(void)
{
   return g_stEepConfig.stUser.eMaskMessage;
}

void SetGsmFindTimeout(uint8_t uc_gsm_find_timeout)
{
   g_stEepConfig.stGsm.uc_gsm_find_timeout = uc_gsm_find_timeout;
}

uint8_t GetGsmFindTimeout(void)
{
   return g_stEepConfig.stGsm.uc_gsm_find_timeout;
}

void SetLbsFindTimeout(uint8_t uc_lbs_find_timeout)
{
   g_stEepConfig.stGsm.uc_lbs_find_timeout = uc_lbs_find_timeout;
}

uint8_t GetLbsFindTimeout(void)
{
   return g_stEepConfig.stGsm.uc_lbs_find_timeout;
}

void SetGprsOpenCount(uint8_t uc_gprs_open_count)
{
   g_stEepConfig.stGsm.uc_gprs_open_count = uc_gprs_open_count;
}

uint8_t GetGprsOpenCount(void)
{
   return g_stEepConfig.stGsm.uc_gprs_open_count;
}

void SetAnswerTimeout(uint8_t uc_wait_answer_timeout)
{
   g_stEepConfig.stGsm.uc_wait_answer_timeout = uc_wait_answer_timeout;
}

uint8_t GetAnswerTimeout(void)
{
   return g_stEepConfig.stGsm.uc_wait_answer_timeout;
}

void SetGsmGprsTimeout(uint8_t uc_gprs_timeout)
{
   g_stEepConfig.stGsm.uc_gprs_timeout = uc_gprs_timeout;
}

uint8_t GetGsmGprsTimeout(void)
{
   return g_stEepConfig.stGsm.uc_gprs_timeout;
}

void SetUserPwrDevice(USER_PWR_STATUS eUserPwrDevice)
{
   g_stEepConfig.stDevice.eUserPwrDevice = eUserPwrDevice;
}

USER_PWR_STATUS GetUserPwrDevice(void)
{
   return g_stEepConfig.stDevice.eUserPwrDevice;
}

void SetTimeoutLowPwrMode1(uint32_t uiTimeoutLowPwrMode1)
{
   g_stEepConfig.stDevice.uiTimeoutLowPwrMode1 = uiTimeoutLowPwrMode1;
}

uint32_t GetTimeoutLowPwrMode1(void)
{
   return g_stEepConfig.stDevice.uiTimeoutLowPwrMode1;
}

void SetTimeoutLowPwrMode2(uint32_t uiTimeoutLowPwrMode2)
{
   g_stEepConfig.stDevice.uiTimeoutLowPwrMode2 = uiTimeoutLowPwrMode2;
}

uint32_t GetTimeoutLowPwrMode2(void)
{
   return g_stEepConfig.stDevice.uiTimeoutLowPwrMode2;
}

void SetTimeLowPwrMode1(uint16_t usTimeSleepLowPwrMode1)
{
   g_stEepConfig.stDevice.usTimeSleepLowPwrMode1 = usTimeSleepLowPwrMode1;
}

uint16_t GetTimeLowPwrMode1(void)
{
   return g_stEepConfig.stDevice.usTimeSleepLowPwrMode1;
}

void SetTimeLowPwrMode2(uint16_t usTimeSleepLowPwrMode2)
{
   g_stEepConfig.stDevice.usTimeSleepLowPwrMode2 = usTimeSleepLowPwrMode2;
}

uint16_t GetTimeLowPwrMode2(void)
{
   return g_stEepConfig.stDevice.usTimeSleepLowPwrMode2;
}

void SetLenDataFlashReady(uint32_t uiLenDataFlashReady)
{
   g_stEepConfig.stDevice.uiLenDataFlashReady = uiLenDataFlashReady;
}

uint32_t GetLenDataFlashReady(void)
{
   return g_stEepConfig.stDevice.uiLenDataFlashReady;
}

void SetEnableUseLowPwr1Mode(_Bool bEnableUseLowPwr1Mode)
{
   g_stEepConfig.stDevice.bEnableUseLowPwr1Mode = bEnableUseLowPwr1Mode;
}

_Bool GetEnableUseLowPwr1Mode(void)
{
   return g_stEepConfig.stDevice.bEnableUseLowPwr1Mode;
}

void SetEnableUseLowPwr2Mode(_Bool bEnableUseLowPwr2Mode)
{
   g_stEepConfig.stDevice.bEnableUseLowPwr2Mode = bEnableUseLowPwr2Mode;
}

_Bool GetEnableUseLowPwr2Mode(void)
{
   return g_stEepConfig.stDevice.bEnableUseLowPwr2Mode;
}

void SetSleepTimeStandart(uint32_t uiSleepTimeStandart)
{
   g_stEepConfig.stDevice.uiSleepTimeStandart = uiSleepTimeStandart;
}

uint32_t GetSleepTimeStandart(void)
{
   return g_stEepConfig.stDevice.uiSleepTimeStandart;
}

void SetSleepTimeFind(uint32_t uiSleepTimeFind)
{
   g_stEepConfig.stDevice.uiSleepTimeFind = uiSleepTimeFind;
}

uint32_t GetSleepTimeFind(void)
{
   return g_stEepConfig.stDevice.uiSleepTimeFind;
}

void SetEnableAccelToFind(_Bool bEnableAccelToFind)
{
   g_stEepConfig.stDevice.bEnableAccelToFind = bEnableAccelToFind;
}

_Bool GetEnableAccelToFind(void)
{
   return g_stEepConfig.stDevice.bEnableAccelToFind;
}

void SetModeDevice(TYPE_MODE_DEV eModeDevice)
{
   g_stEepConfig.stDevice.eModeDevice = eModeDevice;
}

TYPE_MODE_DEV GetModeDevice(void)
{
   return g_stEepConfig.stDevice.eModeDevice;
}

void SetModeProtocol(TYPE_MODE_PROTOCOL eModeProtocol)
{
   g_stEepConfig.stDevice.eModeProtocol = eModeProtocol;
}

TYPE_MODE_PROTOCOL GetModeProtocol(void)
{
   return g_stEepConfig.stDevice.eModeProtocol;
}

void SetTimeReconnect1(uint32_t time_reconnect)
{
   SetTimeReconnect(0, time_reconnect);
}
void SetTimeReconnect2(uint32_t time_reconnect)
{
   SetTimeReconnect(1, time_reconnect);
}
void SetTimeReconnect3(uint32_t time_reconnect)
{
   SetTimeReconnect(2, time_reconnect);
}
void SetTimeReconnect4(uint32_t time_reconnect)
{
   SetTimeReconnect(3, time_reconnect);
}
void SetTimeReconnect5(uint32_t time_reconnect)
{
   SetTimeReconnect(4, time_reconnect);
}

uint32_t GetTimeReconnect1(void)
{
   return GetTimeReconnect(0);
}
uint32_t GetTimeReconnect2(void)
{
   return GetTimeReconnect(1);
}
uint32_t GetTimeReconnect3(void)
{
   return GetTimeReconnect(2);
}
uint32_t GetTimeReconnect4(void)
{
   return GetTimeReconnect(3);
}
uint32_t GetTimeReconnect5(void)
{
   return GetTimeReconnect(4);
}

void SetTimeReconnect(size_t index, uint32_t time_reconnect)
{
   /* «ащита от переполнени€ индекса массива времени переоткладывани€ */
   const size_t max_index = (sizeof(g_stEepConfig.stDevice.a_uiTimeReConnect) / sizeof(uint32_t)) - 1;
   if (index > max_index)
   {
      index = max_index;
   }
   g_stEepConfig.stDevice.a_uiTimeReConnect[index] = time_reconnect;
}

uint32_t GetTimeReconnect(size_t index)
{
   /* «ащита от переполнени€ индекса массива времени переоткладывани€ */
   const size_t max_index = (sizeof(g_stEepConfig.stDevice.a_uiTimeReConnect) / sizeof(uint32_t)) - 1;
   if (index > max_index)
   {
      index = max_index;
   }
   return g_stEepConfig.stDevice.a_uiTimeReConnect[index];
}

void SetCountReConnect(uint8_t ucCountReConnect)
{
   /* «ащита от переполнени€ индекса массива времени переоткладывани€ */
   const size_t max_index = sizeof(g_stEepConfig.stDevice.a_uiTimeReConnect) / sizeof(uint32_t);
   if (ucCountReConnect > max_index)
   {
      ucCountReConnect = (uint8_t)(max_index);
   }
   g_stEepConfig.stDevice.ucCountReConnect = ucCountReConnect;
}

uint16_t GetCountReConnect(void)
{
   /* «ащита от переполнени€ индекса массива времени переоткладывани€ */
   const size_t max_index = sizeof(g_stEepConfig.stDevice.a_uiTimeReConnect) / sizeof(uint32_t);
   if (g_stEepConfig.stDevice.ucCountReConnect > max_index)
   {
      g_stEepConfig.stDevice.ucCountReConnect = (uint8_t)(max_index);
   }
   return (uint16_t)(g_stEepConfig.stDevice.ucCountReConnect);
}

void SetGpsWait(uint32_t uiGpsWait)
{
   g_stEepConfig.stDevice.uiGpsWait = uiGpsWait;
}

uint32_t GetGpsWait(void)
{
   return g_stEepConfig.stDevice.uiGpsWait;
}

void SetLedEnable(_Bool bLedEnable)
{
   g_stEepConfig.stDevice.bLedEnable = bLedEnable;
}

_Bool GetLedEnable(void)
{
   return g_stEepConfig.stDevice.bLedEnable;
}

void SetMinTemperaturWorkDevice(int8_t cMinTemperaturWork)
{
   g_stEepConfig.stDevice.cMinTemperaturWork = cMinTemperaturWork;
}

int8_t GetMinTemperaturWorkDevice(void)
{
   return g_stEepConfig.stDevice.cMinTemperaturWork;
}

void SetGpsRealtime(uint16_t us_gps_real_time_record_data)
{
   g_stEepConfig.stGps.us_gps_real_time_record_data = us_gps_real_time_record_data;
}

uint16_t GetGpsRealtime(void)
{
   return g_stEepConfig.stGps.us_gps_real_time_record_data;
}

uint16_t GetGpsRecordtime(void)
{
   return g_stEepConfig.stGps.us_gps_time_record_data;
}

void SetGpsRecordtime(uint16_t us_gps_time_record_data)
{
   g_stEepConfig.stGps.us_gps_time_record_data = us_gps_time_record_data;
}

void SetRecordAccel(_Bool b_gps_record_accel_data)
{
   g_stEepConfig.stGps.b_gps_record_accel_data = b_gps_record_accel_data;
}

_Bool GetRecordAccel(void)
{
   return g_stEepConfig.stGps.b_gps_record_accel_data;
}

void SetGpsRecordCourse(uint8_t uc_gps_record_course)
{
   g_stEepConfig.stGps.uc_gps_record_course = uc_gps_record_course;
}

uint8_t GetGpsRecordCourse(void)
{
   return g_stEepConfig.stGps.uc_gps_record_course;
}

void SetGpsRecordDistanse(uint16_t us_gps_record_distance)
{
   g_stEepConfig.stGps.us_gps_record_distance = us_gps_record_distance;
}

uint16_t GetGpsRecordDistanse(void)
{
   return g_stEepConfig.stGps.us_gps_record_distance;
}

void SetGpsRecordMinSpeed(uint16_t us_gps_record_min_speed)
{
   g_stEepConfig.stGps.us_gps_record_min_speed = us_gps_record_min_speed;
}

uint16_t GetGpsRecordMinSpeed(void)
{
   return g_stEepConfig.stGps.us_gps_record_min_speed;
}

void SetGpsHdopFixCoordinates(float uc_hdop_fix_coordinates)
{
   g_stEepConfig.stGps.uc_hdop_fix_coordinates = uc_hdop_fix_coordinates;
}

float GetGpsHdopFixCoordinates(void)
{
   return g_stEepConfig.stGps.uc_hdop_fix_coordinates;
}

void SetGpioRecordTime(uint16_t usTimeRecord)
{
   g_stEepConfig.stGpio.usTimeRecord = usTimeRecord;
}

uint16_t GetGpioRecordTime(void)
{
   return g_stEepConfig.stGpio.usTimeRecord;
}

void SetFlagsStatusFirmware(FRAME_FIRMWARE_TYPE eFlagsStatusFirmware)
{
   g_stEepConfig.stFirmware.eFlagsStatusFirmware = eFlagsStatusFirmware;
}

FRAME_FIRMWARE_TYPE GetFlagsStatusFirmware(void)
{
   return g_stEepConfig.stFirmware.eFlagsStatusFirmware;
}

void SetIntNewFirmware(uint32_t uiNameNewFirmware)
{
   g_stEepConfig.stFirmware.uiNameNewFirmware = uiNameNewFirmware;
}

uint32_t GetIntNewFirmware(void)
{
   return g_stEepConfig.stFirmware.uiNameNewFirmware;
}

void SetAddrFirmSer(const char* ptr)
{
   strcpy(g_stEepConfig.stFirmware.strNameFirmSer, ptr);
}

int GetAddrFirmSer(char* ptr)
{
   if (strlen(g_stEepConfig.stFirmware.strNameFirmSer) == 0)
      return 0;
   strcpy(ptr, g_stEepConfig.stFirmware.strNameFirmSer);
   return strlen(g_stEepConfig.stFirmware.strNameFirmSer);
}

void SetDefAddrFirmSer(const char* ptr)
{
   strcpy(g_stEepConfig.stFirmware.strDefNameFirmSer, ptr);
}

int GetDefAddrFirmSer(char* ptr)
{
   if (strlen(g_stEepConfig.stFirmware.strDefNameFirmSer) == 0)
      return 0;
   strcpy(ptr, g_stEepConfig.stFirmware.strDefNameFirmSer);
   return strlen(g_stEepConfig.stFirmware.strDefNameFirmSer);
}

void DriveSimpleInit(void)
{
#ifdef _DRIVE_SIMPLE_
   DSm_ConfigInit(&g_stEepConfig.stDSm_Data);
#endif
}

/* LOG FM911 */
/* ѕервое число это сумма всех нештатных перезагрузок: кнопка + питание. ¬торое число это только кнопка */
void GetCountRebootDevice(uint16_t* pCountAll, uint16_t* pCountLowPwr)
{
   *pCountAll = g_stEepConfig.stLogDevice.auiCountRebootDevice[0];
   *pCountLowPwr = g_stEepConfig.stLogDevice.auiCountRebootDevice[1];
}
void SetCountRebootDevice(const uint16_t usCountAll, const uint16_t usCountLowPwr)
{
   g_stEepConfig.stLogDevice.auiCountRebootDevice[0] = usCountAll;
   g_stEepConfig.stLogDevice.auiCountRebootDevice[1] = usCountLowPwr;
}

/* поиск GPS координат всего запросов */
void GetGpsFind(uint16_t* pGpsValid, uint16_t* pGpsFail)
{
   *pGpsValid = g_stEepConfig.stLogDevice.ausGpsFind[0];
   *pGpsFail = g_stEepConfig.stLogDevice.ausGpsFind[1];
}
void SetGpsFind(uint16_t pGpsValid, uint16_t pGpsFail)
{
   g_stEepConfig.stLogDevice.ausGpsFind[0] = pGpsValid;
   g_stEepConfig.stLogDevice.ausGpsFind[1] = pGpsFail;
}

void GetGsmLog(uint16_t* pGsmOk, uint16_t* pGsmErr)
{
   *pGsmOk = g_stEepConfig.stLogDevice.ausGsmLog[0];
   *pGsmErr = g_stEepConfig.stLogDevice.ausGsmLog[1];
}

void SetGsmLog(const uint8_t pGsmOk, const uint8_t pGsmErr)
{
   g_stEepConfig.stLogDevice.ausGsmLog[0] = pGsmOk;
   g_stEepConfig.stLogDevice.ausGsmLog[1] = pGsmErr;
}

void GetGsmPwrErr(uint16_t* pGsmOk, uint16_t* pGsmErr, uint16_t* pLowCsq, uint16_t* pCelFail)
{
   *pGsmOk = g_stEepConfig.stLogDevice.ausGsmPwrErr[0];
   *pGsmErr = g_stEepConfig.stLogDevice.ausGsmPwrErr[1];
   *pLowCsq = g_stEepConfig.stLogDevice.ausGsmPwrErr[2];
   *pCelFail = g_stEepConfig.stLogDevice.ausGsmPwrErr[3];
}

void SetGsmPwrErr(uint8_t GsmOk, uint8_t GsmErr, uint16_t LowCsq, uint16_t CelFail)
{
   g_stEepConfig.stLogDevice.ausGsmPwrErr[0] = GsmOk;
   g_stEepConfig.stLogDevice.ausGsmPwrErr[1] = GsmErr;
   g_stEepConfig.stLogDevice.ausGsmPwrErr[2] = LowCsq;
   g_stEepConfig.stLogDevice.ausGsmPwrErr[3] = CelFail;
}

void GetServerConnect(uint16_t* pServConnOk, uint16_t* pServConnErr)
{
   *pServConnOk = g_stEepConfig.stLogDevice.ausServerConnectErr[0];
   *pServConnErr = g_stEepConfig.stLogDevice.ausServerConnectErr[1];
}

void SetServerConnect(uint16_t pServConnOk, uint16_t pServConnErr)
{
   g_stEepConfig.stLogDevice.ausServerConnectErr[0] = pServConnOk;
   g_stEepConfig.stLogDevice.ausServerConnectErr[1] = pServConnErr;
}

/* выключение gsm из-за просадки по питанию. выключение gsm из-за низкой температуры.  */
void GetGsmWorkErr(uint16_t* pGsmLowPwr, uint16_t* pGsmLowTemp)
{
   *pGsmLowPwr = g_stEepConfig.stLogDevice.ausGsmWorkErr[0];
   *pGsmLowTemp = g_stEepConfig.stLogDevice.ausGsmWorkErr[1];
}
void SetGsmWorkErr(uint16_t GsmLowPwr, uint16_t GsmLowTemp)
{
   g_stEepConfig.stLogDevice.ausGsmWorkErr[0] = GsmLowPwr;
   g_stEepConfig.stLogDevice.ausGsmWorkErr[1] = GsmLowTemp;
}

/* количество не удачных попыток зарегистрироватьс€ в сети оператора  */
uint16_t GetGsmFind(void)
{
   return g_stEepConfig.stLogDevice.usGsmFindErr;
}
void SetGsmFind(uint16_t usGsmFindErr)
{
   g_stEepConfig.stLogDevice.usGsmFindErr = usGsmFindErr;
}

/* количество не открытых GPRS сеансов */
uint16_t GetGsmGprsErr(void)
{
   return g_stEepConfig.stLogDevice.usGsmGprsErr;
}
void SetGsmGprsErr(uint16_t usGsmGprsErr)
{
   g_stEepConfig.stLogDevice.usGsmGprsErr = usGsmGprsErr;
}

/* количество не полученных ответов от сервера за заданное врем€ ожидани€ */
uint16_t GetServerErr(void)
{
   return g_stEepConfig.stLogDevice.usServerErr;
}

void SetServerErr(uint16_t usServerErr)
{
   g_stEepConfig.stLogDevice.usGsmGprsErr = usServerErr;
}

/* общее количество циклов (пробуждений) устройства */
uint16_t GetDeviceWakeup(void)
{
   return g_stEepConfig.stLogDevice.usDeviceWakeup;
}
void SetDeviceWakeup(uint16_t usDeviceWakeup)
{
   g_stEepConfig.stLogDevice.usDeviceWakeup = usDeviceWakeup;
}

uint32_t GetDeviceWorkTime(void)
{
   return g_stEepConfig.stLogDevice.uiDeviceWorkTime;
}

void SetDeviceWorkTime(uint32_t uiDeviceWorkTime)
{
   g_stEepConfig.stLogDevice.uiDeviceWorkTime = uiDeviceWorkTime;
}

/* суммарное врем€ работы модул€  GPS */
uint32_t GetTimePwrGps(void)
{
   return g_stEepConfig.stLogDevice.uiTimePwrGps;
}

void SetTimePwrGps(uint32_t uiTimePwrGps)
{
   g_stEepConfig.stLogDevice.uiTimePwrGps = uiTimePwrGps;
}

/* суммарное врем€ работы модул€  GSM */
uint32_t GetTimePwrGsm(void)
{
   return g_stEepConfig.stLogDevice.uiTimePwrGsm;
}

void SetTimePwrGsm(uint32_t uiTimePwrGsm)
{
   g_stEepConfig.stLogDevice.uiTimePwrGsm = uiTimePwrGsm;
}

/* счетчик того, сколько раз в ма€ке радионаводкой был сбит специальный бит */
uint16_t GetRtcFail(void)
{
   return g_stEepConfig.stLogDevice.usRtcFail;
}

void SetRtcFail(uint16_t usRtcFail)
{
   g_stEepConfig.stLogDevice.usRtcFail = usRtcFail;
}

/* счетчик битой конфигурации в Eeprom */
uint16_t GetEepromFail(void)
{
   return g_stEepConfig.stLogDevice.usEepromFail;
}

void SetEepromFail(uint16_t usEepromFail)
{
   g_stEepConfig.stLogDevice.usEepromFail = usEepromFail;
}

uint32_t GetTimeWaitSms(void)
{
   return g_stEepConfig.stSim.uiWaitSMS;
}

void SetTimeWaitSms(uint32_t uiWaitSMS)
{
   g_stEepConfig.stSim.uiWaitSMS = uiWaitSMS;
}

void setTimeContactSecondSim(uint32_t value)
{
   g_stEepConfig.stSim.uiTimeContactSecondSim = value;
}

uint32_t getTimeContactSecondSim(void)
{
   return g_stEepConfig.stSim.uiTimeContactSecondSim;
}

void setOldPositionGps(const GPS_INFO* const ptr)
{
   memcpy(&g_stEepConfig.stGps.stGpsOldData, ptr, sizeof(GPS_INFO));
}

int getOldPositionGps(GPS_INFO* ptr)
{
   if (g_stEepConfig.stGps.stGpsOldData.latitude && g_stEepConfig.stGps.stGpsOldData.longitude &&
       g_stEepConfig.stGps.stGpsOldData.time && g_stEepConfig.stGps.stGpsOldData.sat &&
       g_stEepConfig.stGps.stGpsOldData.status)
   {
      memcpy(ptr, &g_stEepConfig.stGps.stGpsOldData, sizeof(GPS_INFO));
      return 1;
   }
   return 0;
}