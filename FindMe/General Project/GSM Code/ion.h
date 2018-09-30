
#ifndef __ION_H
#define __ION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "includes.h"

void stepGsmProfileGprsSendStatusDevice(TGsmStatus* pGsmStatus);
void stepGsmProfileGprsSendLogDevice(TGsmStatus* pGsmStatus);
void stepGsmProfileGprsAccelStatus(TGsmStatus* pGsmStatus);
void stepGsmProfileGprsSendDataION(TGsmStatus* pGsmStatus);
void stepGsmProfileGprsSendDataConfig(TGsmStatus* pGsmStatus);
void stepGsmProfileGprsSendArchiveData(TGsmStatus* pGsmStatus);
void stepGsmProfileGprsSendDataFm(TGsmStatus* pGsmStatus);

void resetFmStepCount(void);

#ifdef __cplusplus
}
#endif

#endif