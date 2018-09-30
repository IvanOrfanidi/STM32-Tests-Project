#ifndef __PROTOCOL_GENERAL_H
#define __PROTOCOL_GENERAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "includes.h"
#include "gsm_parser.h"

GSM_STEP GprsAckData(void);
RET_INFO GprsSendFailC(void);
RET_INFO GprsSendAckC(void);
GSM_STEP parsingData(uint8_t* ptr);
RET_INFO GprsSendDataInfoDevice(void);
RET_INFO GprsSendAnsOkData(void);
RET_INFO GprsSendAnsErrData(void);
RET_INFO GprsSendDataLogDevice(void);
RET_INFO GprsSendDataAccelStatus(void);
RET_INFO GprsWaitAcknow(uint8_t nProf);

#ifdef __cplusplus
}
#endif

#endif