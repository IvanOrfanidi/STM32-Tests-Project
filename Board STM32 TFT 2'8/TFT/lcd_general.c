
#include "includes.h"
#include "lcd_general.h"

void vLcdTask (void *pvParameters)
{
  
  while(1)
  {
    //GPIO_TOGGLE(PORT_LED, LED_D2);
    osDelay(SLEEP_MS_1000);
  }
  
}