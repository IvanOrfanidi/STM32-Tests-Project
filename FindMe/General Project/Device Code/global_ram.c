
#include "includes.h"
#include "global_ram.h"

uint64_t GetIMEI(void)
{
   return g_stRam.stGsm.ulIMEI;
}
void SetIMEI(uint64_t ulIMEI)
{
   g_stRam.stGsm.ulIMEI = ulIMEI;
}

int GetStrIMEI(char* ptr)
{
   if (strlen(g_stRam.stGsm.strIMEI) == 0)
      return 0;
   strcpy(ptr, g_stRam.stGsm.strIMEI);
   return strlen(g_stRam.stGsm.strIMEI);
}
void SetStrIMEI(const char* ptr)
{
   strcpy(g_stRam.stGsm.strIMEI, ptr);
}

uint8_t GetCSQ(void)
{
   return g_stRam.stGsm.aucGsmCSQ;
}
void SetCSQ(uint8_t csq)
{
   g_stRam.stGsm.aucGsmCSQ = csq;
}

int GetModemIdentification(char* ptr)
{
   if (strlen(g_stRam.stGsm.strGsmModemIdentification) == 0)
      return 0;
   strcpy(ptr, g_stRam.stGsm.strGsmModemIdentification);
   return strlen(g_stRam.stGsm.strGsmModemIdentification);
}
void SetModemIdentification(const char* ptr)
{
   strcpy(g_stRam.stGsm.strGsmModemIdentification, ptr);
}

int GetGsmModemSoftware(char* ptr)
{
   if (strlen(g_stRam.stGsm.strGsmModemSoftware) == 0)
      return 0;
   strcpy(ptr, g_stRam.stGsm.strGsmModemSoftware);
   return strlen(g_stRam.stGsm.strGsmModemSoftware);
}
void SetGsmModemSoftware(const char* ptr)
{
   strcpy(g_stRam.stGsm.strGsmModemSoftware, ptr);
}

int GetScidCurentFirstSim(char* ptr)
{
   if (strlen(g_stRam.stSim.strFIRST_SCID) == 0)
      return 0;
   strcpy(ptr, g_stRam.stSim.strFIRST_SCID);
   return strlen(g_stRam.stSim.strFIRST_SCID);
}
void SetScidCurentFirstSim(const char* ptr)
{
   strcpy(g_stRam.stSim.strFIRST_SCID, ptr);
}

int GetScidCurentSecondSim(char* ptr)
{
   if (strlen(g_stRam.stSim.strSECOND_SCID) == 0)
      return 0;
   strcpy(ptr, g_stRam.stSim.strSECOND_SCID);
   return strlen(g_stRam.stSim.strSECOND_SCID);
}
void SetScidCurentSecondSim(const char* ptr)
{
   strcpy(g_stRam.stSim.strSECOND_SCID, ptr);
}

void ledStatus(LED_Status led_stat)
{
   g_stRam.stDevice.eLedStatus = led_stat;
}

uint16_t GetTimeAllPwrGps(void)
{
   return g_stRam.stDevDiag.stHard.uiTimePwrGps + GetTimePwrGps();
}

uint16_t GetTimeAllPwrGsm(void)
{
   return GetTimePwrGsm();
}

uint16_t GetGpsAllFind(void)
{
   uint16_t ucTemp1 = 0, ucTemp2 = 0;
   GetGpsFind(&ucTemp1, &ucTemp2);
   return ucTemp1;
}

uint16_t GetGpsErrFind(void)
{
   uint16_t ucTemp1 = 0, ucTemp2 = 0;
   GetGpsFind(&ucTemp1, &ucTemp2);
   return ucTemp2;
}

uint16_t GetGsmAllFind(void)
{
   uint16_t ucTemp1 = 0, ucTemp2 = 0;
   GetGsmLog(&ucTemp1, &ucTemp2);
   return ucTemp1;
}

uint16_t GetGsmErrFind(void)
{
   uint16_t ucTemp1 = 0, ucTemp2 = 0;
   GetGsmLog(&ucTemp1, &ucTemp2);
   return ucTemp2;
}

uint16_t GetServerAllConnect(void)
{
   uint16_t ucTemp1 = 0, ucTemp2 = 0;
   GetServerConnect(&ucTemp1, &ucTemp2);
   return ucTemp1;
}

uint16_t GetServerErrConnect(void)
{
   uint16_t ucTemp1 = 0, ucTemp2 = 0;
   GetServerConnect(&ucTemp1, &ucTemp2);
   return ucTemp2;
}

uint16_t GetGsmAllErrWork(void)
{
   uint16_t ucTemp1, ucTemp2;
   GetGsmWorkErr(&ucTemp1, &ucTemp2);
   return ucTemp1;
}

uint16_t GetGsmAllCoolWork(void)
{
   uint16_t ucTemp1 = 0, ucTemp2 = 0;
   GetGsmWorkErr(&ucTemp1, &ucTemp2);
   return ucTemp2;
}

uint16_t GetCountAllRebootDevice(void)
{
   uint16_t usTemp1 = 0, usTemp2 = 0;
   GetCountRebootDevice(&usTemp1, &usTemp2);
   return usTemp1;
}

uint16_t GetCountLowPwrRebootDevice(void)
{
   uint16_t usTemp1 = 0, usTemp2 = 0;
   GetCountRebootDevice(&usTemp1, &usTemp2);
   return usTemp2;
}