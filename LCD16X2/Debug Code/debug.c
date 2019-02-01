
#include "debug.h"
#include "includes.h"

void vDebugTask(void* pvParameters)
{
    int Res;
    char strMsgDebug[64];

#ifdef DEBUG_OUTPUT_USART
    InitUSART(UART_DBG, DBG_BAUDRATE);
    InitDMA(UART_DBG);
#endif

    while(1) {
        memset(strMsgDebug, 0, sizeof(strMsgDebug));
        LED_TOGGLE;
        Res = 1;
        strcat(strMsgDebug, "\r\n");

        if(!(Res)) {
#ifdef DEBUG_OUTPUT_USART
            USART_Write(UART_DBG, strMsgDebug, strlen(strMsgDebug));
#endif

#ifdef DEBUG_OUTPUT_USB
            if(bDeviceState == CONFIGURED) {
                CDC_Send_DATA((unsigned char*)strMsgDebug, strlen(strMsgDebug));
                NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
            }
#endif
        }

        _delay_ms(1000);
    }
}
