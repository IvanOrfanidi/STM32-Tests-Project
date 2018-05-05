
#include "debug.h"
#include "includes.h"

int HandlerHumidity(char* pOut);

void vDebugTask(void* pvParameters)
{
   int Res;
   char strMsgDebug[64];

#ifdef DEBUG_OUTPUT_USART
   InitUSART(UART_DBG, DBG_BAUDRATE);
   InitDMA(UART_DBG);
#endif

   while (1)
   {
      LED_TOGGLE;
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

      _delay_ms(1000);
   }
}
