#define __ALLOCATE_NOW__

// User functions //
#include "includes.h"
#include "uip.h"
#include "uip_arp.h"
#include "enc28j60.h"

int main()
{
   InitGPIO();

   InitBKP();

   Set_System();
   Set_USBClock();
   USB_Interrupts_Config();
   USB_Init();

   rtc_init();

   /*
   RTC_t date;
   date.year = 2015;
   date.month = 10;
   date.mday = 24;
   
   date.hour = 23;
   date.min = 20;
   date.sec = 0;
   rtc_settime(&date);
   */

   // spi_sd_card_init();

   // InitTIM4();

   InitRTOS();

   while (1)
      ;
}
