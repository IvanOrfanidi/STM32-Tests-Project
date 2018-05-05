
#include "debug.h"
#include "includes.h"

void vDebugTask(void* pvParameters)
{
   int Res = 1;
   char strMsgDebug[64];
   uint16_t usValueLx;

#ifdef DEBUG_OUTPUT_USART
   InitUSART(UART_DBG, DBG_BAUDRATE);
   InitDMA(UART_DBG);
#endif

   PCF8574_I2C_Init();
   LcdPcf_Init();
   LcdPcf_Backlight(1);

   BH1750_I2C_Init();
   BH1750_Init();

   while (1)
   {
      memset(strMsgDebug, 0, sizeof(strMsgDebug));
      usValueLx = BH1750_Read();
      sprintf(strMsgDebug, " %05i Lx", usValueLx);
      LcdPcf_Goto(1, 0);
      prit_lcd(strMsgDebug);

      if (!(Res))
      {
#ifdef DEBUG_OUTPUT_USART
         USART_Write(UART_DBG, strMsgDebug, strlen(strMsgDebug));
#endif

#ifdef DEBUG_OUTPUT_USB
         if (bDeviceState == CONFIGURED)
         {
            CDC_Send_DATA((unsigned char*)strMsgDebug, strlen(strMsgDebug));
            NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
         }
#endif
      }

      osDelay(1000);
   }
}
