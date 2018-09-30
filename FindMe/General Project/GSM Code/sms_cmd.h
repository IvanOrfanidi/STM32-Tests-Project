#ifndef _SMS_CMD_H_
#define _SMS_CMD_H_
#include "gsm_mc52iT.h"

int CheckSmsCommand(void);
void SendMessage(char* ptrTelUser);
_Bool alarmTrue(void);
void setEventTrue(void);
void resetEventTrue(void);
void sendSmsForUser911(char* ptrTelUser);

#endif