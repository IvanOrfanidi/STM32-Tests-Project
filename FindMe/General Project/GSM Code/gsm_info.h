#ifndef __GSM_INFO_H
#define __GSM_INFO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "includes.h"
#include "stdtypes.h"
#include "gsm_parser.h"

typedef struct
{
    // csq
    u8 rssi;
    u8 ber;

    // creg
    u8 urcMode;
    u8 regStatus;
    u8 netLac;
    u8 netCellId;

    // cops
    u8 mode;
    u8 format;
    u32 opName;

} GSM_PARAM;

int getAllLbsInfo(GSM_INFO* ptr, int second);
int getActiveLbsInfo(GSM_INFO* ptr, int second);
int lbsInfo2buffer(GSM_INFO* pLbsCeng, uint32_t time, char* pOut, int iOffset, _Bool bAddLbs);
int gsm_getTemperature(GSM_INFO* pDataGsmInfo);
void SimPwr(VALUE eVal);
SIM_CARD GetNumSim(void);
SIM_CARD SelectNumSim(SIM_CARD eNumSim);
RET_INFO GetCellularNetwork(void);
int check_csq(GSM_INFO* out);
int check_creg(GSM_INFO* out);
char* generPinSim(const char* pSour, char* pGenPin);
uint8_t GetQualityGsm(void);
int LockSimCard(void);
EBrandOperator GetBrandOperator(void);
int CallEventTel(const char* pTelUser);
SIM_CARD GetStatusAllSim(void);
void SetScidCurentSecondSim(const char* ptr);

void setInfoLbsData(const GSM_INFO* pGsm);
_Bool getInfoLbsData(GSM_INFO* pGsm);
void setAllLbsData(const GSM_INFO* pLbsCnetscan);
_Bool getAllLbsData(GSM_INFO* pLbsCnetscan);
uint32_t additionOtherLbsForFm911(GSM_INFO* pLbsCeng);
uint32_t additionOtherLbsForIon(GSM_INFO* pLbsCeng);
M_INFO getSimStatus(char* ptr);
_Bool switchingEventSim(M_INFO eSimStatus);

void setUseSecondSim(void);
_Bool getUseSecondSim(void);

_Bool expireTimeContactSecSim(void);
#ifdef __cplusplus
}
#endif

#endif