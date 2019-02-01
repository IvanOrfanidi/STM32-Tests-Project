
#include "debug.h"
#include "includes.h"

int HandlerCompass(char* pOut);

void vDebugTask(void* pvParameters)
{
    int Res;
    char strMsgDebug[64];

#ifdef DEBUG_OUTPUT_USART
    InitUSART(UART_DBG, DBG_BAUDRATE);
    InitDMA(UART_DBG);
#endif

    while(1) {
        LED_TOGGLE;
        Res = HandlerCompass(strMsgDebug);
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

int HandlerCompass(char* pOut)
{
    TCompass_Data stCompassData;
    char* pSt = (char*)&stCompassData;
    char toSend;

    //Проверяем создана ли очередь и читаем из нее.
    if(xQueueCompassDataToUsart != 0) {
        for(uint8_t i = 0; i < sizeof(stCompassData); i++) {
            if(xQueueReceive(xQueueCompassDataToUsart, (void*)&toSend, (portTickType)10)) {
                pSt[sizeof(stCompassData) - 1 - i] = toSend;
            }
            else {
                //В очереде пока ничего нет.
                return -1;
            }
        }
        if(stCompassData.bDataValid == TRUE) {
            sprintf(pOut,
                "AXIS X: %i\r\nAXIS Y: %i\r\nAXIS Z: %i\r\n",
                stCompassData.sValueAxisX,
                stCompassData.sValueAxisY,
                stCompassData.sValueAxisZ);
        }
        else {
            sprintf(pOut, "COMPASS ERROR\r\n");
        }
        return 0;
    }
    //Очереди вообще не удалось создаться.
    return -2;
}