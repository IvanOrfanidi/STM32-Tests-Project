#include "includes.h"

uint8_t UsbTxBuffer[SIZE_USB_DATA_BUFFER] = { 0 };
uint8_t UsbRxBuffer[SIZE_USB_DATA_BUFFER] = { 0 };

#if (DBG_RX_BUFFER_SIZE)
char dbg_rx_buf[DBG_RX_BUFFER_SIZE] = { 0 };   // //����� �� ����
uint16_t dbg_rx_coun = 0;
uint16_t dbg_rx_wr_index = 0;
#endif
#if (DBG_TX_BUFFER_SIZE)
char dbg_tx_buf[DBG_TX_BUFFER_SIZE] = { 0 };   // ����� �� ��������
uint16_t dbg_tx_coun = 0;
uint16_t dbg_tx_rd_index = 0;
#endif

#ifdef USE_RESERVE_BUFF
char* ptrReserveBuf = NULL;   //��������� �� ��������� �����
#endif

/* ���������� ������������ ������ ������� � UART ��� ������ � FreeRTOS */
void cout(const char* fmt_ptr, ...)
{
#if defined(__USE_DEBUG_UART__) || defined(__USE_DEBUG_USB__)
   if (osKernelRunning())
   {   //���� �������� OS
      if (xSemaphoreTake(sBinSemDbg, 1) != pdTRUE)
      {   //�������� ����������� ����������� ����������
#   ifdef USE_RESERVE_BUFF
         if (ptrReserveBuf)
            return;   // �������� ��� ������ ���������� ������ ���� �� ��������.
         ptrReserveBuf = pvPortMalloc(strlen(fmt_ptr));   // �������� ����� � ���� ��� ���������� ������ ������ �������
         if (ptrReserveBuf)
            strcpy(ptrReserveBuf, fmt_ptr);
#   endif
         return;
      }
   }
   else
   {
      uint32_t delay = portMAX_DELAY;
      while (dbg_tx_coun && delay)
      {
         delay--;
      }
      if (!(delay))
         return;   // ��������� ��������� ������
   }

   va_list ap;
   va_start(ap, fmt_ptr);
   int check_overflow = vsprintf(dbg_tx_buf, fmt_ptr, ap);
   va_end(ap);

   /* �������� ������������ ����������� ������ */
   if (check_overflow > DBG_TX_BUFFER_SIZE)
   {
      assert_param(dbg_tx_buf[check_overflow]);   //����� � ������� � �� ��� �� ������
   }
   else
   {
      dbg_tx_coun = (uint16_t)check_overflow;
#   ifdef __USE_DEBUG_UART__
      RUN_TRANSMIT_UART_DBG;   //���������� �� �������� ������
#   endif

#   ifdef __USE_DEBUG_USB__
      int iOffsetLenAnswer = 0;
      int len = strlen(dbg_tx_buf);
      while (len > VIRTUAL_COM_PORT_DATA_SIZE - 1)
      {
         CDC_Send_DATA((unsigned char*)&dbg_tx_buf[iOffsetLenAnswer], VIRTUAL_COM_PORT_DATA_SIZE - 1);
         iOffsetLenAnswer += VIRTUAL_COM_PORT_DATA_SIZE - 1;
         len -= VIRTUAL_COM_PORT_DATA_SIZE;
         len++;
      }
      if (len)
      {
         len = strlen(&dbg_tx_buf[iOffsetLenAnswer]);
         CDC_Send_DATA((unsigned char*)&dbg_tx_buf[iOffsetLenAnswer], len);
      }
      xSemaphoreGive(sBinSemDbg);
#   endif
   }
#endif
}

void UART_Debug_TxReservLoadCallback(void)
{
#ifdef USE_RESERVE_BUFF
   if (ptrReserveBuf)
   {   //���� ���� ������ ��� ��������, �� ������� ��
      cout(ptrReserveBuf);
      vPortFree(ptrReserveBuf);   //���������� ����� � ����
      ptrReserveBuf = NULL;
   }
#endif
}

/* ������ �� ��������� �������� ������ �� UART */
void UART_Debug_TxCpltCallback(void)
{
#if (DBG_TX_BUFFER_SIZE)
   if (dbg_tx_coun)
   {   //���� ���� ��� ���������, �� ��������, ����� ��������� ����������
      USART_SendData(UART_DBG_INTRERFACE, dbg_tx_buf[dbg_tx_rd_index++]);
      if (dbg_tx_rd_index >= dbg_tx_coun)
      {   //����� ����
         dbg_tx_coun = NULL;
         dbg_tx_rd_index = NULL;
         STOP_TRANSMIT_UART_DBG;   //������������� ����������
         if (osKernelRunning())
         {
            BaseType_t xHigherPriorityTaskWoken = pdTRUE;
#   ifdef USE_RESERVE_BUFF
            if (ptrReserveBuf)
            {   //���� ���� ��������� ���������� ������, �� �������� ������� ��������.
               xSemaphoreGiveFromISR(sBinSemReservDbgBufe, &xHigherPriorityTaskWoken);
            }
#   endif
            //������� ������� �� ��������� �������� �� ���������� �������. ����������� �������
            xSemaphoreGiveFromISR(sBinSemDbg, &xHigherPriorityTaskWoken);
         }
      }
   }
#endif
}
