#ifndef __GSM_SMS_H
#define __GSM_SMS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdtypes.h"

#define CODE_LAT 0
#define CODE_UCS2 1

typedef struct
{
   u8 buf[16];
   u8 size;
} SMS_TN;

typedef struct
{
   _Bool code;
   u8* buf;
   u16 size;
} SMS_TXT;

typedef struct
{
   u8 number;
   SMS_TN tn;
   SMS_TXT txt;
} SMS_INFO;

typedef enum
{
   UNSPECIFIED_ERROR = -128,
   SIM_PIN_REQUIRED = -4,
   OPERATION_NOT_ALLOWED = -3,
   PARESER_FREEZES = -2,
   DMA_OVERFLOW = -1,
   SMS_FALSE = 0,
   SMS_TRUE = 1,
} SMS_RESPONSE;

SMS_RESPONSE PDU_SMGL(u64* mask);
SMS_RESPONSE PDU_SMGL_FM911(char* ptrNumUserTel, char* ptrNameDevice, _Bool* pCode);
int PDU_SMGR(SMS_INFO* sms, u8 second);
int PDU_SMGD(uint8_t sms_number);
int PDU_SendSms(SMS_INFO* sms, char* sms_buf);
int SendTXTSMS(char* pbuf, char* pTelSMS);

#ifdef __cplusplus
}
#endif

#endif