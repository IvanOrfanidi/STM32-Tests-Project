#ifndef __DEBUG_H
#define __DEBUG_H

//#define USE_RESERVE_BUFF

void cout(const char* fmt_ptr, ...);
void vDebugTask(void* pvParameters);

void UART_Debug_TxReservLoadCallback(void);
void UART_Debug_TxCpltCallback(void);

#endif