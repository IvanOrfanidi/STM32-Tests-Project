
#ifndef __FM_911_H
#define __FM_911_H

#ifdef __cplusplus
extern "C" {
#endif

#include "includes.h"

#define _FAST_REGISTRATION_TO_BASE_ 0    // Регистрация в БС сервера FM911 без проверки GPS.
#define _FAST_REGISTRATION_USER_ 0       // Регистрация пользователя в базе сервера FM911.

#if(_FAST_REGISTRATION_USER_)
#define TEL_REG_USER "79119008502"
#define NAME_DEVICE "MyDevice"
#endif

typedef __packed struct
{
    char strNameDevice[MAX_SIZE_NAME_DEVICE * 4];
    char strNumUserTel[SIZE_TEL];
} TUserConfigFm911;

void stepGsmFindme911DataReady(TGsmStatus* pGsmStatus);
void stepGsmFindme911GprsAckData(TGsmStatus* pGsmStatus);
void stepGsmFindme911WaitRegUser(TGsmStatus* pGsmStatus);
void stepGsmFindme911AddSendData(TGsmStatus* pGsmStatus);
void stepGsmEndSms(TGsmStatus* pGsmStatus);
void changeServerFm2iON(void);

#ifdef __cplusplus
}
#endif

#endif