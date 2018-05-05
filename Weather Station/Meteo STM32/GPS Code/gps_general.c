

#include "includes.h"
#include "gps_general.h"
#include "USART.h"

uint32_t GpsTimeBack = 0;
uint32_t GpsRealTimeBack = 0;

char g_aucGpsBuffOut[100];

void vGpsHandler(void* pvParameters)
{
   memset(&stGpsData, 0, sizeof(stGpsData));

   uint8_t ucTimeoutMutexUartGps = 0;
   uint8_t ucDalayGpsNotValid = 0;

   // Init UART GPS
   InitUSART(UART_GPS, GPS_BAUDRATE);
   InitDMA(UART_GPS);

   while (1)
   {
      if (osMutexWait(mGPS_DATA_ARRIVAL, SLEEP_MS_1000) == osOK)
      {
         gps_parser(&stGpsData, g_aucGpsBuffOut, strlen(g_aucGpsBuffOut));

         if (stGpsData.time && stGpsData.latitude && stGpsData.longitude)
         {
            ucDalayGpsNotValid = 0;
            RTC_t stDateTime;
#define TIME_ZONE 3 * 60 * 60
            stGpsData.time += TIME_ZONE;
            Sec2Date(&stDateTime, stGpsData.time);
            // rtc_settime(&stDateTime);
            LED_OFF;
         }
         else
         {
            ucDalayGpsNotValid++;
         }

         if (ucDalayGpsNotValid > DELAY_GPS_VALID)
         {
            LED_TOGGLE;
            memset(&stGpsData, 0, sizeof(stGpsData));
         }

         ucTimeoutMutexUartGps = 0;
      }
      else
      {
         ucTimeoutMutexUartGps++;
         if (ucTimeoutMutexUartGps >= MAX_TIME_GPS_NO_MUTEX)
         {
            ucTimeoutMutexUartGps = MAX_TIME_GPS_NO_MUTEX;
            LED_TOGGLE;
            memset(&stGpsData, 0, sizeof(stGpsData));
         }
      }
   }
}

void CpyGpsBuf(const char* pBuf, const int Len)
{
   memset(g_aucGpsBuffOut, 0, sizeof(g_aucGpsBuffOut));
   memcpy(g_aucGpsBuffOut, pBuf, Len);
}