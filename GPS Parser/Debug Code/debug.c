
#include "debug.h"
#include "includes.h"

void vDebugTask(void* pvParameters)
{
#ifdef DEBUG_OUTPUT_USART
    InitUSART(UART_DBG, DBG_BAUDRATE);
    InitDMA(UART_DBG);
#endif

    while(1) {
        osDelay(1000);
    }
}