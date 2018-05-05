
#include "debug.h"
#include "includes.h"

int HandlerAccel(char* pOut);

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
      memset(strMsgDebug, 0, sizeof(strMsgDebug));
      LED_TOGGLE;
      Res = HandlerAccel(strMsgDebug);
      strcat(strMsgDebug, "\r\n");

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

int HandlerAccel(char* pOut)
{
   TAccel_Data stAccelData;
   char* pSt = (char*)&stAccelData;
   char toSend;

   //Проверяем создана ли очередь и читаем из нее.
   if (xQueueAccelDataToUsart != 0)
   {
      for (uint8_t i = 0; i < sizeof(stAccelData); i++)
      {
         if (xQueueReceive(xQueueAccelDataToUsart, (void*)&toSend, (portTickType)10))
         {
            pSt[sizeof(stAccelData) - 1 - i] = toSend;
         }
         else
         {
            //В очереде пока ничего нет.
            return -1;
         }
      }
      if (stAccelData.bDataValid)
      {
         sprintf(pOut,
                 "AXIS X: %i\r\nAXIS Y: %i\r\nAXIS Z: %i\r\nINTERRUPT: %i\r\n",
                 stAccelData.sValueAxisX,
                 stAccelData.sValueAxisY,
                 stAccelData.sValueAxisZ,
                 stAccelData.ucInterrupt);
      }
      else
      {
         sprintf(pOut, "ACCEL ERROR\r\n");
      }
      return 0;
   }
   //Очереди вообще не удалось создаться.
   return -2;
}
