
#define __ALLOCATE_NOW__
#include "includes.h"
#include "stm32l1xx.h"

#pragma location = "ConstDate"
__root const char RELEASEDATE[] = __DATE__;

#pragma location = "ConstFirmVer"
__root const uint32_t FIRMWARE = NAME_FIRMWARE;

#pragma location = "ConstBuild"
__root const uint32_t BUILD = 00;

#pragma location = "ConstDevVer"
__root const char US_DEV_VER[2] = HW_VER;
#pragma location = "ConstHwVer"
__root const char US_HW_VER[2] = DEV_VER;
#pragma location = "ConstFF"
__root const uint16_t FF = 0xFFFF;

xTaskHandle xHandleGPS;
xTaskHandle xHandleFLASH;
xTaskHandle xHandlePERIF;
xTaskHandle xHandleGSM;

int main(void)   // Start Applications
{
   SystemCoreClock = SYSCLK_FREQ;
   RCC_Configuration();

   if (readConfig())
   {
      SetEepromFail(GetEepromFail() + 1);
   }
   controlSleepMode();   // контролируем пробуждение девайса

   CheckFuses();   // Включает защиту чипа от чтения прошивки с flash
   GPIO_Configuration();

   IWDGInit();   // Config Watchdog
   SPI1_LowLevel_Init(); /* FM4 - FLASH & ACCEL. FM3 - ACCEL. */
   SPI2_LowLevel_Init(); /* FM4 - NO USE. FM3 - FLASH. */
   DelayResolution100us(1000);
   TM7Init();   // Timer Sec.
   PVD_Config();

   setDebugAll(0);

#if __DEBUG__
   setDebug(1);
   setDebugGsm(1);
   setDebugAccel(1);
#endif

   if (getDebug())
   {
      InitUSART(UART_DBG, DBG_BAUDRATE);
      InitDMA(UART_DBG);
   }

   DPS("\r\n-=D_RUN APPL=-\r\n");

   GetFlagsControlRegister();

   if (RTC_Configuration())
   {
      DPS("\r\nD_RESET DATE RTC\r\n\r\n");
      resetSystemDateTime();   // Сбрасываем время по умолчанию
   }
   updateStatusReset();   //Получаем причину перезагрузки
   RCC_ClearFlag();

   g_uiPacStatDevTime = GetWakingTime();   //Сохраняем время пробуждения.
   g_stRam.stFirmware.uiNameNewFirmware = ReadNameNewFirmware();
   DP_GSM("D_FIRMWARE WEB CUR: < %d >\r\n", flash_read_word(__CONST_FIRM_VER));
   DP_GSM("D_FIRMWARE NEW FLS: < %d >\r\n", g_stRam.stFirmware.uiNameNewFirmware);

#ifdef FM3
   char strFmVersia[20] = { '\0' };
   getNameFmVersia(strFmVersia);
   DP_GSM("D_FIRMWARE FM911: %s\r\n", strFmVersia);
#endif

   printDeviceTime();
   // DP_GSM("D_TIME SLEEP REC: %d\r\n", GetDeviceSleep() - GetTimeSleepDevice());
   DP_GSM("D_COUNT RECONNECT: %d\r\n", GetCountReConnect());

   /*SetModeDevice(TIMER_FIND);
   SetModeProtocol(FINDME_911);
   SetUseTypeServ(SECOND_SERVER);
   SetTypeRegBase911(TYPE_WU_START);
   SetSleepTimeFind(10);
   extern char g_aucBufDownHttpFirm[];
   #define SERVER_TRUE_VALUE       g_aucBufDownHttpFirm
   memset(SERVER_TRUE_VALUE, USER_CONFIG_DEVICE_PACKET, MAX_PARAM_CONFIG_VALUE);
   
   SetTimeoutLowPwrMode1(DEF_TIMEOUT_LOW_POWER_MODE1);
   SetTimeoutLowPwrMode2(DEF_TIMEOUT_LOW_POWER_MODE2);
   SetTimeLowPwrMode1(DEF_SLEEP_LOW_POWER_MODE1);
   SetTimeLowPwrMode2(DEF_SLEEP_LOW_POWER_MODE2);*/
   SaveConfigCMD();

   if (GetStatusReset() == BUTTON_RESET && GetModeProtocol() == ION_FM)
   {
      SetTimeWaitSms(DEF_TIME_WAIT_SMS);
   }

   SetLedEnable(FALSE);
   const char* const pstrNameMode[] = { "<TRACK ION>", "<STANDART>", "<TIMER FIND>" };
   DP_GSM("D_MODE: ");
   DP_GSM(pstrNameMode[GetModeDevice()]);
   DP_GSM("\r\n");

   DP_GSM("D_ACCEL FIND: ");
   if (GetEnableAccelToFind())
   {
      DP_GSM("ON\r\n");
   }
   else
   {
      DP_GSM("OFF\r\n");
   }

   /* если протокол девайса FM911, то выведем его режим работы  */
   if (GetModeProtocol() == FINDME_911)
   {
      SetLedEnable(FALSE);
      DP_GSM("D_TYPE CONNECT FM911: ");
      switch (GetTypeConnectFm911())
      {
      case TYPE_REG_TO_BASE:   //Регистрация девайса в БД
         SetLedEnable(TRUE);
         DP_GSM("<REG TO BASE>");
         break;
      case TYPE_REG_USER:   //Регистрация пользователя
         SetLedEnable(TRUE);
         DP_GSM("<REG USER>");
         break;
      case TYPE_ERR_CONSRV:   //Предыдущий выход был с ошибкой
         DP_GSM("<ERR CONNECT SRV>");
         break;
      case TYPE_ERR_COLD:   //Выход по низкой температуры
         DP_GSM("<ERR COLD>");
         break;

      default:
         DP_GSM("<MAIN MODE>");   //Основной режим работы
      }
      DP_GSM("\r\n");
   }
   else
   {
      if (GetTypeConnectFm911() == TYPE_REG_TO_BASE)
      {   //Включаем светодиод при первом включении в режиме проверки на производстве
         SetLedEnable(TRUE);
      }
   }

   if (GetModeDevice() == TRACK_ION)
   {
      SetLedEnable(TRUE);   //Если режим трекера, то включим светодиод.
   }
   else
   {
      if (GetStatusReset() == BUTTON_RESET)
      {
         SetLedEnable(TRUE);   //Если нажали на кнопку, то включим светодиод.
      }
   }

   SetDeviceWakeup(GetDeviceWakeup() + 1);

   uint16_t usTemp1 = 0;   //сумма всех нештатных перезагрузок: кнопка + питание
   uint16_t usTemp2 = 0;   //только нажатие на кнопку
   GetCountRebootDevice(&usTemp1, &usTemp2);
   RESET_STATUS_DEVICE eStatusReset =
      GetStatusReset();   //получаем причину перезагрузки и устанавливаем счетчик для 911
   if (eStatusReset == LOW_POWER)
   {
      usTemp1++;
   }
   else if (eStatusReset == BUTTON_RESET)
   {
      usTemp1++;
      usTemp2++;
   }
   SetCountRebootDevice(usTemp1, usTemp2);

   osStatus InitOS();
   if (InitOS() == osOK)
   {
      DPS("\r\n-D_START SCHEDULER RTOS-\r\n\r\n");
      osKernelStart(NULL, NULL);   // Start scheduler
   }

   DPS("\r\n-RTOS FATAL ERROR-\r\n");
   DelayResolution100us(10000);
   RebootDevice();
}

/*
  Return osStatus: osOK - Rtos OK, osErrorOS - Rtos Fail
*/
static osStatus InitOS(void)
{
   mGPS_DATA_ARRIVAL = osMutexCreate(0);
   mINIT_GPS_MODULE = osMutexCreate(0);
   mDEINIT_GPS_MODULE = osMutexCreate(0);

   sBinSPI = xSemaphoreCreateMutex();
   sBinSemFLASH = xSemaphoreCreateMutex();
   sBinSemFLASH_BUFF = xSemaphoreCreateMutex();
   sBinSemFLASH = xSemaphoreCreateMutex();
   sBinSemDATA_WRITE = xSemaphoreCreateMutex();
   sBinSemUSART = xSemaphoreCreateMutex();

   mFLASH_SAVE_DATA_FM = osMutexCreate(0);
   mFLASH_COMPLETE = osMutexCreate(0);

   xSemaphoreTake(mGPS_DATA_ARRIVAL, 0);
   xSemaphoreTake(mINIT_GPS_MODULE, 0);
   xSemaphoreTake(mDEINIT_GPS_MODULE, 0);
   xSemaphoreTake(mFLASH_SAVE_DATA_FM, 0);
   xSemaphoreTake(mFLASH_COMPLETE, 0);

   xEepromQueue = xQueueCreate(SIZE_QUEUE_EEPROM, sizeof(uint8_t));
   if (xEepromQueue == NULL)
      return osErrorOS;

   xAccelQueue = xQueueCreate(sizeof(uint8_t), sizeof(uint8_t));
   if (xAccelQueue == NULL)
      return osErrorOS;

   xFlashQueue = xQueueCreate(sizeof(uint8_t), sizeof(int));
   if (xFlashQueue == NULL)
      return osErrorOS;

   if (sizeof(TEepConfig) > SIZE_CONFIG_EEPROM)
      return osErrorOS;

   if (xTaskCreate(vGsmTask, "vGsmTask", configMINIMAL_STACK_SIZE * 5, NULL, tskIDLE_PRIORITY + 2, &xHandleGSM) == NULL)
   {
      return osErrorOS;
   }
   if (xTaskCreate(
          vPeriphHandler, "vPeriphHandler", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 1, &xHandlePERIF) ==
       NULL)
   {
      return osErrorOS;
   }
   if (xTaskCreate(vFlashTask, "vFlashTask", configMINIMAL_STACK_SIZE * 3, NULL, tskIDLE_PRIORITY + 3, &xHandleFLASH) ==
       NULL)
   {
      return osErrorOS;
   }
   if (xTaskCreate(vGpsHandler, "vGpsHandler", configMINIMAL_STACK_SIZE * 3, NULL, tskIDLE_PRIORITY + 3, &xHandleGPS) ==
       NULL)
   {
      return osErrorOS;
   }
   return osOK;
}

void vApplicationMallocFailedHook(void)
{
   for (;;)
      ;
}
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char* pcTaskName)
{
   for (;;)
      ;
}

void vApplicationIdleHook(void)
{
   portTickType WakeTick = 0;
   volatile static uint32_t sys_monitor = 0;
   volatile static uint32_t timer_sys_monitor = 0xFFFFFFFF;

   WakeTick = xTaskGetTickCount() + configTICK_RATE_HZ;
   while (1)
   {
      if (xTaskGetTickCount() >= WakeTick)
      {
         if ((GPIO_ReadInputDataBit(GSM_REF_PORT, GSM_REF_PIN) == ON) && (GetSleepGsm() == FALSE))
         {
            g_stRam.stDevDiag.stHard.uiTimePwrGsm++;
         }
#if (INCLUDE_uxTaskGetStackHighWaterMark == 1)
         /* Контроль свободного места в стеках задач */
         g_stRam.stDevDiag.stRtos.ulFreeSizeStackGsmTask = uxTaskGetStackHighWaterMark(xHandleGSM);
         g_stRam.stDevDiag.stRtos.ulFreeSizeStackGsmTask = g_stRam.stDevDiag.stRtos.ulFreeSizeStackGsmTask << 2;
         g_stRam.stDevDiag.stRtos.ulFreeSizeStackPeriphHandler = uxTaskGetStackHighWaterMark(xHandlePERIF);
         g_stRam.stDevDiag.stRtos.ulFreeSizeStackPeriphHandler = g_stRam.stDevDiag.stRtos.ulFreeSizeStackPeriphHandler
                                                                 << 2;
         g_stRam.stDevDiag.stRtos.ulFreeSizeStackGpsHandler = uxTaskGetStackHighWaterMark(xHandleGPS);
         g_stRam.stDevDiag.stRtos.ulFreeSizeStackGpsHandler = g_stRam.stDevDiag.stRtos.ulFreeSizeStackGpsHandler << 2;
         g_stRam.stDevDiag.stRtos.ulFreeSizeStackApplicationIdleHook = uxTaskGetStackHighWaterMark(NULL);
         g_stRam.stDevDiag.stRtos.ulFreeSizeStackApplicationIdleHook =
            g_stRam.stDevDiag.stRtos.ulFreeSizeStackApplicationIdleHook << 2;
#endif
         // g_stRam.stDevDiag.stRtos.ulFreeHeapSize = (unsigned long) xPortGetFreeHeapSize(); //in Byte

         if (sys_monitor != GetSystemMonitor())
         {
            sys_monitor = GetSystemMonitor();
#define USER_SYS_TIME_WTG 600
            timer_sys_monitor = USER_SYS_TIME_WTG;
         }
         if (timer_sys_monitor)
         {
            timer_sys_monitor--;
            IWDG_ReloadCounter();
         }
         else
         {
            DPS("\r\n-D_WARNUNG FREEZES RTOS-\r\n");
            RebootDevice();
         }

         WakeTick = xTaskGetTickCount() + configTICK_RATE_HZ;
      }
      if (WakeTick > xTaskGetTickCount() + configTICK_RATE_HZ << 1)
      {
         WakeTick = configTICK_RATE_HZ + xTaskGetTickCount();
      }
   }
}

#ifdef USE_FULL_ASSERT

/**
 * @brief  Reports the name of the source file and the source line number
 *   where the assert_param error has occurred.
 * @param file: pointer to the source file name
 * @param line: assert_param error line source number
 * @retval : None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
   /* User can add his own implementation to report the file name and line number,
      ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

   /* Infinite loop */
   while (1)
   {
   }
}
#endif