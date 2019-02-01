
#include "debug.h"
#include "includes.h"

void vDebugTask(void* pvParameters)
{
    // InitUSART(UART_DBG, DBG_BAUDRATE);
    // InitDMA(UART_DBG);

    LCD_Init();
    LCD_Send_CMD(DISP_ON);
    LCD_Send_CMD(CLR_DISP);
    _delay_ms(250);
    LCD_Send_STR("-Settings WIFI-");
    _delay_ms(1000);

    while(1) {
        LCD_Send_CMD(CLR_DISP);
        LCD_Send_STR(BufferI2C_Rx1);
        osDelay(1000);
    }
}
