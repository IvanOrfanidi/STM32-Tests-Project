#ifndef _GLOBAL_RAM_H_
#define _GLOBAL_RAM_H_

uint64_t GetIMEI(void);
void SetIMEI(uint64_t ulIMEI);
int GetStrIMEI(char* ptr);
void SetStrIMEI(const char* ptr);
uint8_t GetCSQ(void);
void SetCSQ(uint8_t csq);
int GetModemIdentification(char* ptr);
void SetModemIdentification(const char* ptr);
int GetGsmModemSoftware(char* ptr);
void SetGsmModemSoftware(const char* ptr);
int GetScidCurentFirstSim(char* ptr);
void SetScidCurentFirstSim(const char* ptr);
int GetScidCurentSecondSim(char* ptr);
void ledStatus(LED_Status led_stat);
uint16_t GetTimeAllPwrGps(void);
uint16_t GetTimeAllPwrGsm(void);
uint16_t GetGpsAllFind(void);
uint16_t GetGpsErrFind(void);
uint16_t GetGsmAllFind(void);
uint16_t GetGsmErrFind(void);
uint16_t GetServerAllConnect(void);
uint16_t GetServerErrConnect(void);
uint16_t GetGsmAllErrWork(void);
uint16_t GetGsmAllCoolWork(void);
uint16_t GetGpsValidWait(void);
uint16_t GetCountAllRebootDevice(void);
uint16_t GetCountLowPwrRebootDevice(void);

#endif