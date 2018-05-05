#ifndef _GSM_SMS_H_
#define _GSM_SMS_H_

#include "stdtypes.h"
#include "gsm_mc52iT.h"

typedef struct
{
   u8 buf[16];
   u8 size;
} SMS_TN;

typedef struct
{
   u8 code;
   u8* buf;
   u16 size;
} SMS_TXT;

typedef struct
{
   u8 number;
   SMS_TN tn;
   SMS_TXT txt;
} SMS_INFO;

int ucs2_to_koi8(u8* out, u8* inp, int size);
int PDU_SMGL(u64* mask);
int PDU_SMGR(SMS_INFO* sms, u8 second);
RET_INFO PDU_SMGD(uint8_t sms_number);
int PDU_SendSms(SMS_INFO* sms, char* sms_buf);
int SendTXTSMS(u8* pbuf, char* pTelSMS);

#endif