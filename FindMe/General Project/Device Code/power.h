#ifndef __POWER_H
#define __POWER_H

#ifdef __cplusplus
extern "C" {
#endif

#define PIN_RSTF 0x4000000

#define DEF_TIME_DATA 1483314600

void SetTimeSleepDevice(void);
void PVD_Config(void);
void updateStatusReset(void);
void RebootDevice(void);
uint32_t GetWakingTime(void);
void SetStatusReset(RESET_STATUS_DEVICE eStatReset);
RESET_STATUS_DEVICE GetStatusReset(void);
uint32_t GetFlagsControlRegister(void);
void controlSleepMode(void);
void UpdateTimeSleepDevice(void);
void SystemUpdateMonitor(void);
uint32_t GetSystemMonitor(void);
RESET_STATUS_DEVICE GetFlagsResetDevice(_Bool reset_status_device);
void setFlagReset(void);
_Bool getFlagReset(void);

#ifdef __cplusplus
}
#endif

#endif