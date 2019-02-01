
#include "debug.h"
#include "includes.h"

int HandlerPressure(char* pOut);

void vDebugTask(void* pvParameters)
{
    int Res;
    char strMsgTemp[64];

#ifdef DEBUG_OUTPUT_USART
    InitUSART(UART_DBG, DBG_BAUDRATE);
    InitDMA(UART_DBG);
#endif

    while(1) {
        memset(strMsgTemp, 0, sizeof(strMsgTemp));
        LED_TOGGLE;
        Res = HandlerPressure(strMsgTemp);
        if(!(Res)) {
#ifdef DEBUG_OUTPUT_USART
            USART_Write(UART_DBG, strMsgTemp, strlen(strMsgTemp));
#endif

#ifdef DEBUG_OUTPUT_USB
            if(bDeviceState == CONFIGURED) {
                CDC_Send_DATA((unsigned char*)strMsgTemp, strlen(strMsgTemp));
                NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
            }
#endif
        }

        _delay_ms(1000);
    }
}

int HandlerPressure(char* pOut)
{
    TPressure_Data stPressureData;
    char* pSt = (char*)&stPressureData;
    char toSend;

    float fTemperatur;        //Температура в С
    float fRealPressurePa;    //Давление в кПа
    int iRealPressureMMHG;    //Давление в ммРс

    //Проверяем создана ли очередь и читаем из нее.
    if(xQueuePressureDataToUsart != 0) {
        for(uint8_t i = 0; i < sizeof(stPressureData); i++) {
            if(xQueueReceive(xQueuePressureDataToUsart, (void*)&toSend, (portTickType)10)) {
                pSt[sizeof(stPressureData) - 1 - i] = toSend;
            }
            else {
                //В очереде пока ничего нет.
                return -1;
            }
        }

        fTemperatur = stPressureData.iRealTemperatur;
        fTemperatur /= 10;

        fRealPressurePa = stPressureData.iRealPressurePa;
        fRealPressurePa /= 1000;

        iRealPressureMMHG = stPressureData.iRealPressureHg;

        if(fTemperatur < 0) {
            sprintf(pOut,
                "TEMPERATUR BMP: -%.01f C\r\nPRESSURE: %.03f kPa (%i mmHg)\r\n",
                fTemperatur,
                fRealPressurePa,
                iRealPressureMMHG);
        }
        else {
            sprintf(pOut,
                "TEMPERATUR BMP: +%.01f C\r\nPRESSURE: %.03f kPa (%i mmHg)\r\n",
                fTemperatur,
                fRealPressurePa,
                iRealPressureMMHG);
        }

        return 0;
    }

    //Очереди вообще не удалось создаться.
    return -2;
}
