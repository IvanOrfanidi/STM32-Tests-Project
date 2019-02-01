#ifndef __TEST_DEVICE_H
#define __TEST_DEVICE_H

#include "includes.h"

#if(USE_TEST_DEVICE)
#define TEST_UPWR 1
#define TEST_TEMPER 1
#define TEST_CSQ 1
#define TEST_HDOP 1
#define TEST_HSE 1
#define TEST_LSE 1
#else
#define TEST_UPWR 0
#define TEST_TEMPER 0
#define TEST_CSQ 0
#define TEST_HDOP 0
#define TEST_HSE 0
#define TEST_LSE 0
#endif

/*
1 версия прошивки 		f=1458917674
2 IMEI				i=353437069574298
3 тип девайса			t=16
4 напряжение питания 	        upwr=11000
5 температура			tm=27
6 уровень GSM сигнала	        csq=20
7 погрешность GPS		hdop=2
8 отказ внешнего ВЧ кварца      hse=1
9 отказ внешнего НЧ кварца      lse=1
*/

#define ADDR_SERVER_FOR_DEVICE_TEST "online.irz.net/findme_production.php?"

int testDevice(char* pOut);
_Bool checkTwoSIM(void);

#endif