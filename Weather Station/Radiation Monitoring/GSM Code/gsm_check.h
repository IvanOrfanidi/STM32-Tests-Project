#ifndef _GSM_CHECK_H
#define _GSM_CHECK_H

#include "includes.h"
#include "gsm_parser.h"

RET_INFO GetCellularNetwork(void);
int check_csq(GSM_INFO* out);
void csq_save(GSM_INFO* out);
int check_creg(GSM_INFO* out);

#endif
