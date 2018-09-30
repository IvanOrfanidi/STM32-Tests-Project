
#include "includes.h"
#include "eeprom.h"
#include "ram.h"
#include "periph_general.h"

PWR_STATUS GetPowerStatus(void);

void FillFlashErase(void)
{
   DPS("-=D_ERASE FLASH START=-\r\n");
   FlashBulkErase();
   ledStatus(NORMAL_RUN);
   DPS("-=D_ERASE FLASH END=-\r\n");
}

void vPeriphHandler(void* pvParameters)
{
   portTickType xLastWakeTimerDelay;

   /* Init ADC */
   ADCInit();

   /* Init Timer Led */
   TM4Init();

   while (1)
   {
      AdcHandler();
      PwrHandler();

      xLastWakeTimerDelay = xTaskGetTickCount();
      vTaskDelayUntil(&xLastWakeTimerDelay, (SLEEP_MS_1000 / portTICK_RATE_MS));
   }
}

void PwrHandler(void)
{
   //Обновляем статус энергопотребления девайса
   if (GetModeDevice() == TRACK_ION)
   {
      g_stRam.stDevice.eCurPwrStat = GetPowerStatus();
   }

   switch (GetStatusReset())
   {
   case WARNING_ACCEL_FAIL:
      g_stRam.stDevice.eCurPwrStat = POWER_RUN_MODE;
      break;
   }
}

_Bool ChangeStatusDevice(void)
{
   static PWR_STATUS BackStatusPowerDevice = (PWR_STATUS)0xFF;
   uint32_t temp = 0;
   _Bool Status = 0;

   //Проверяем причину перезагрузки
   static RESET_STATUS_DEVICE BackfGetFlagsResetDevice = (RESET_STATUS_DEVICE)0xFF;

   if (BackfGetFlagsResetDevice == 0xFF && BackStatusPowerDevice == 0xFF)
   {
      g_uiPacStatDevTime = GetWakingTime();
   }
   else
   {
      g_uiPacStatDevTime = time();
   }

   temp = (uint32_t)(GetFlagsResetDevice(0));
   if (temp != BackfGetFlagsResetDevice)
   {
      BackfGetFlagsResetDevice = (RESET_STATUS_DEVICE)(temp);
      Status = 1;
   }

   //Проверяем питание.
   temp = (uint32_t)(g_stRam.stDevice.eCurPwrStat);
   if (temp != BackStatusPowerDevice)
   {
      BackStatusPowerDevice = (PWR_STATUS)(temp);
      Status = 1;
   }

   return Status;
}

_Bool GpioFilterData(void)
{
   _Bool FilterDataTrue = 0;
   uint32_t SecRTC;
   static uint32_t GpioTimeBack = 0;

   //проверим статус энергопотребления, если он LOW PWR2, то отменим запись.
   if ((g_stRam.stDevice.eCurPwrStat == POWER_LOW_PWR2_MODE) || (GetGpioRecordTime() == 0))
   {
      GpioTimeBack = 0;
      return 0;
   }

   SecRTC = get_cur_sec();
   //Проверяем время последней записи
   if (SecRTC >= GpioTimeBack)
   {
      GpioTimeBack = SecRTC + (uint32_t)GetGpioRecordTime();
      FilterDataTrue = 1;
   }

   return FilterDataTrue;
}

// Получение статуса энергопотребления устройства.
PWR_STATUS GetPowerStatus(void)
{
   static PWR_STATUS eDebMsg = (PWR_STATUS)0xFF;
   char msg_buf_pwr_status[30];
   memset(msg_buf_pwr_status, 0, sizeof(msg_buf_pwr_status));

   static PWR_STATUS ePowerStatus = POWER_RUN_MODE;   //По умолчанию Первый режим энергопотребления.
   if (g_stRamAccState.sec_state_stop == 0)
      ePowerStatus = POWER_RUN_MODE;
   if (ePowerStatus != POWER_SLEEP_MODE)
   {
      //Проверим автоматический режим работы контороля энергопотребления
      //(пользователь может сам устанавливать различные режимы энергопотребления).
      if (GetUserPwrDevice() == USER_POWER_AUTO)
      {
         uint8_t PwrLowStatusTrue = 0;   //Показывает статус POWER_LOW_PWR1_MODE и POWER_LOW_PWR2_MODE
         if (GetEnableUseLowPwr1Mode())   //Если пользователь разрешил режим энергопотребления LOW PWR1
         {
            //Если движения нет в течении первого таймаута, то переходим в первый режим энергопотребления.
            if (g_stRamAccState.sec_state_stop >= GetTimeoutLowPwrMode1() * 60)
            {
               ePowerStatus = POWER_LOW_PWR1_MODE;
               PwrLowStatusTrue = 1;
            }
         }

         if (GetEnableUseLowPwr2Mode())   //Если пользователь разрешил режим энергопотребления LOW PWR2
         {
            //Если движения нет в течении второго таймаута, то переходим во второй режим энергопотребления.
            if ((g_stRamAccState.sec_state_stop >= GetTimeoutLowPwrMode2() * 60))
            {
               ePowerStatus = POWER_LOW_PWR2_MODE;
               PwrLowStatusTrue = 2;
            }
         }

         /* Алгоритм: Не засыпать при наличии данных на флешке
         и не просыпаться при их наличии если уже спим  */
         if ((PwrLowStatusTrue == 1))
         {
            if ((g_stRam.stFlash.uiFlashDataLen > GetLenDataFlashReady()) && (GetGsmStep() != GSM_OFF_LOW_PWR))
            {
               ePowerStatus = POWER_RUN_MODE;
            }
         }
      }
      else
      {
         switch (GetUserPwrDevice())
         {
         case USER_POWER_RUN_MODE:
            ePowerStatus = POWER_RUN_MODE;
            break;
         case USER_POWER_LOW_PWR1_MODE:
            ePowerStatus = POWER_LOW_PWR1_MODE;
            break;
         case USER_POWER_LOW_PWR2_MODE:
            ePowerStatus = POWER_LOW_PWR2_MODE;
            break;
         }
      }

      if (eDebMsg != ePowerStatus)
      {
         if (GetUserPwrDevice() == USER_POWER_AUTO)
         {
            switch (ePowerStatus)
            {
            case POWER_RUN_MODE:
               strcpy(msg_buf_pwr_status, "D_POWER RUN MODE ^_^\r\n");
               break;
            case POWER_LOW_PWR1_MODE:
               strcpy(msg_buf_pwr_status, "D_POWER LOW PWR1 -_-\r\n");
               break;
            case POWER_LOW_PWR2_MODE:
               strcpy(msg_buf_pwr_status, "D_POWER LOW PWR2 -_-oO\r\n");
               break;
            }
         }
         else
         {
            switch (ePowerStatus)
            {
            case POWER_RUN_MODE:
               strcpy(msg_buf_pwr_status, "D_USER POWER RUN MODE ^_^\r\n");
               break;
            case POWER_LOW_PWR1_MODE:
               strcpy(msg_buf_pwr_status, "D_USER POWER LOW PWR1 -_-\r\n");
               break;
            case POWER_LOW_PWR2_MODE:
               strcpy(msg_buf_pwr_status, "D_USER POWER LOW PWR2 -_-oO\r\n");
               break;
            }
         }
         eDebMsg = ePowerStatus;
         DPS(msg_buf_pwr_status);
      }
   }
   return ePowerStatus;
}

//Обработчик мигания светодиода SYS.
void LedVisual(void)
{
   static uint16_t Led_On = 0;
   static uint16_t Led_Off = SLEEP_MS_100 / 100;
   static LED_Status LedStatusBack = LOAD_DEVICE;

   if (LedStatusBack != g_stRam.stDevice.eLedStatus)
   {
      LedStatusBack = g_stRam.stDevice.eLedStatus;
      Led_On = 0;
      Led_Off = SLEEP_MS_100 / 100;
   }

   if (Led_On)
   {
      LED_ON;
      Led_On--;
      switch (g_stRam.stDevice.eLedStatus)
      {
      case LOAD_DEVICE:
         Led_Off = SLEEP_MS_100 / 100;
         break;

      case FIND_SIM:
         Led_Off = SLEEP_MS_500 / 100;
         break;

      case REG_GSM:
         Led_Off = SLEEP_MS_500 / 100;
         break;

      case SERVER_CONNECT:
         Led_Off = SLEEP_MS_500 / 100;
         break;

      case NORMAL_RUN:
         Led_Off = SLEEP_MS_100 / 100;
         Led_On++;
         break;

      case LOW_PWR1:
         Led_Off = SLEEP_MS_3000 / 100;
         break;

      case LOW_PWR2:
         Led_Off = SLEEP_MS_10000 / 100;
         break;
      }
   }
   else
   {
      LED_OFF;
      Led_Off--;
      if (!(Led_Off))
      {
         switch (g_stRam.stDevice.eLedStatus)
         {
         case LOAD_DEVICE:
            Led_On = SLEEP_MS_100 / 100;
            break;

         case FIND_SIM:
            Led_On = SLEEP_MS_100 / 100;
            break;

         case REG_GSM:
            Led_On = SLEEP_MS_500 / 100;
            break;

         case SERVER_CONNECT:
            Led_On = SLEEP_MS_1500 / 100;
            break;

         case NORMAL_RUN:
            Led_On = SLEEP_MS_1000 / 100;
            break;

         case LOW_PWR1:
            Led_On = SLEEP_MS_500 / 100;
            break;

         case LOW_PWR2:
            Led_On = SLEEP_MS_500 / 100;
            break;
         }
      }
   }
}

int8_t GetTemperatur(void)
{
#ifdef TEMPERATURE_ADC
   return GetTemperaturAdc();
#endif
#if (TEMPERATURE_ACCEL)
   return GetTemperaturAccel();
#endif
}