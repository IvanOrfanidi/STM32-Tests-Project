#include "includes.h"
/*
 static _Bool testFirm(char *, _Bool *);
 static _Bool testImei(char *, _Bool *);
 static _Bool testType(char *, _Bool *);
 static _Bool testPwr(char *, _Bool *);
 static _Bool testTemper(char *, _Bool *);
 static _Bool testCsq(char *, _Bool *);
 static _Bool testHdop(char *, _Bool *);
 static _Bool testHse(char *, _Bool *);
 static _Bool testLse(char *, _Bool *);
 static _Bool testEnd(char *, _Bool *);
*/

static _Bool testFirm(char* ptr, _Bool* flag_err)
{
   sprintf(ptr, "f=%d&", flash_read_word(__CONST_FIRM_VER));
   return 1;
}
static _Bool testImei(char* ptr, _Bool* flag_err)
{
   sprintf(ptr, "i=%s&", g_stRam.stGsm.strIMEI);
   return 1;
}
static _Bool testType(char* ptr, _Bool* flag_err)
{
   sprintf(ptr, "t=%s%s&", DEV_VER, HW_VER);
   return 1;
}

#if (TEST_UPWR)
static _Bool testPwr(char* ptr, _Bool* flag_err)
{
   sprintf(ptr, "upwr=%d&", GetMeasVin());
   return 1;
}
#endif

#if (TEST_TEMPER)
static _Bool testTemper(char* ptr, _Bool* flag_err)
{
   sprintf(ptr, "tm=%d&", GetTemperatur());
   return 1;
}
#endif

#if (TEST_CSQ)
static _Bool testCsq(char* ptr, _Bool* flag_err)
{
   if (GetQualityGsm() == 0 || GetQualityGsm() == 99)
   {
      flag_err[0] |= TRUE;   // Err
   }
   else
   {
      sprintf(ptr, "csq=%d&", GetQualityGsm());
   }
   return 1;
}
#endif

#if (TEST_HDOP)
static _Bool testHdop(char* ptr, _Bool* flag_err)
{
   if (!(FindPositionGps() > 0))
   {
      flag_err[0] |= TRUE;
      strcpy(ptr, "hdop=9");   // Err
   }
   sprintf(ptr, "hdop=%i&", (int)getHdopGps());
   return 1;
}
#endif

#if (TEST_HSE)
static _Bool testHse(char* ptr, _Bool* flag_err)
{
   if (RCC->CR & RCC_CR_CSSON)
   {
      strcpy(ptr, "hse=0&");   // Ok
   }
   else
   {
      flag_err[0] |= TRUE;
      strcpy(ptr, "hse=1&");   // Err
   }
   return 1;
}
#endif

#if (TEST_LSE)
static _Bool testLse(char* ptr, _Bool* flag_err)
{
   if ((GetWakingTime() + 10) < time())
   {
      strcpy(ptr, "lse=0&");   // Ok
   }
   else
   {
      flag_err[0] |= TRUE;
      strcpy(ptr, "lse=1&");   // Err
   }
   return 1;
}
#endif

static _Bool testEnd(char* ptr, _Bool* flag_err)
{
   return 0;
}

_Bool (*ptrCallbackTestDevice[])(char* ptr,
                                 _Bool* flag_err) = { testFirm,   testImei, testType,
#if (TEST_UPWR)
                                                      testPwr,   // напряжение питания  (upwr=11000)
#endif
#if (TEST_TEMPER)
                                                      testTemper,   // температура  (t=27)
#endif
#if (TEST_CSQ)
                                                      testCsq,   // уровень GSM сигнала (csq=20)
#endif
#if (TEST_HDOP)
                                                      testHdop,   // погрешность GPS   (hdop=2)
#endif
#if (TEST_HSE)
                                                      testHse,   // отказ внешнего высокочастотного кварца (hse=1)
#endif
#if (TEST_LSE)
                                                      testLse,   // отказ внешнего низкочастотного кварца (lse=1)
#endif
                                                      testEnd };

/* return:
 0 - тест пройден
 1 - тест не пройден
*/
int testDevice(char* pOut)
{
   strcpy(pOut, ADDR_SERVER_FOR_DEVICE_TEST);   //Заносим ссылку на тестовый сервак.
   DPS("D_TEST:\r\n");
   _Bool flag_err = 0;   //Признак ошибки теста
   uint8_t count = 0;
   while ((*ptrCallbackTestDevice[count++])(InpDataBuffer, &flag_err))
   {
      strcat(pOut, InpDataBuffer);
      DPS(InpDataBuffer);
      DPS("\r\n");
   }
   pOut[strlen(pOut) - 1] = '\0';
   return flag_err;
}

_Bool checkTwoSIM(void)
{
#if (TWO_SIMCARD)
   static _Bool sim_all_check = 0;
   portTickType xLastWakeTimerDelay;
   if ((GetTypeConnectFm911() == TYPE_REG_TO_BASE) && (!(sim_all_check)))
   {
      SimPwr(ON);
      GSM_State(OFF);
      xLastWakeTimerDelay = xTaskGetTickCount();
      vTaskDelayUntil(&xLastWakeTimerDelay, (1000 / portTICK_RATE_MS));
      GSM_State(ON);
      xLastWakeTimerDelay = xTaskGetTickCount();
      vTaskDelayUntil(&xLastWakeTimerDelay, (1000 / portTICK_RATE_MS));
      SelectNumSim(SECOND_SIMCARD_OK);
      setTimeContactSecondSim(time());
      uint8_t err = 2;
      while (1)
      {
         if (GetStatusAllSim() != FIRST_AND_SECOND_SIMCARD_OK)
         {
            if (!(err--))
            {
               return TRUE;
            }
            xLastWakeTimerDelay = xTaskGetTickCount();
            vTaskDelayUntil(&xLastWakeTimerDelay, (1000 / portTICK_RATE_MS));
         }
         else
         {
            sim_all_check = TRUE;   //Все ок
            SimPwr(OFF);
            break;
         }
      }
   }
#endif   // TWO_SIMCARD
   return FALSE;
}