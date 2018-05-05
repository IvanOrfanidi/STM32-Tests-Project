
/* Includes ------------------------------------------------------------------*/
#include "uart.h"
#include "stm32l0xx_hal.h"

#include <string.h>
#include <ctype.h>

#ifdef __USE_DEBUG_UART__
/* Private function prototypes -----------------------------------------------*/
#   ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#      define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#   else
#      define PUTCHAR_PROTOTYPE int fputc(int ch, FILE* f)
#   endif /* __GNUC__ */
#else
#   warning "Debugging will be done by the standard stdio.h"
#endif

extern TIM_HandleTypeDef modbus_htim;

static uint8_t dbg_receive_buffer[DBG_UART_RX_BUF_SIZE];   // Buffer for debug.
uint8_t* ptrBuffer = NULL;

uint16_t comm_index_rx = 0;
static uint8_t comm_receive_buffer[COMM_UART_RX_BUF_SIZE];   // Buffer for communicator.

UART_HandleTypeDef dbg_uart;
UART_HandleTypeDef comm_uart;

/* LPUSART1 init function */
void MX_LPUSART_DEBUG_Init(void)
{
   dbg_uart.Instance = LPUART1;
   dbg_uart.Init.BaudRate = DBG_UART_BAUDRATE;
   dbg_uart.Init.WordLength = UART_WORDLENGTH_8B;
   dbg_uart.Init.StopBits = UART_STOPBITS_1;
   dbg_uart.Init.Parity = UART_PARITY_NONE;
   dbg_uart.Init.Mode = UART_MODE_TX_RX;
   dbg_uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
   dbg_uart.Init.OneBitSampling = UART_ONEBIT_SAMPLING_DISABLED;
   dbg_uart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
   if (HAL_UART_Init(&dbg_uart) != HAL_OK)
   {
      _Error_Handler(__FILE__, __LINE__);
   }
}

/* USART1 init function */
void MX_USART1_UART_Init(void)
{
   comm_uart.Instance = USART1;
   comm_uart.Init.BaudRate = COMM_UART_BAUDRATE << 1;
   comm_uart.Init.WordLength = UART_WORDLENGTH_8B;
   comm_uart.Init.StopBits = UART_STOPBITS_1;
   comm_uart.Init.Parity = UART_PARITY_NONE;
   comm_uart.Init.Mode = UART_MODE_TX_RX;
   comm_uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
   comm_uart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
   comm_uart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
   if (HAL_UART_Init(&comm_uart) != HAL_OK)
   {
      _Error_Handler(__FILE__, __LINE__);
   }
}

void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
   GPIO_InitTypeDef GPIO_InitStruct;
   if (huart->Instance == LPUART1)
   {
      /* Peripheral clock enable */
      __LPUART1_CLK_ENABLE();

      /**LPUART1 GPIO Configuration
      PC10     ------> LPUART1_RX
      PC11     ------> LPUART1_TX
      */
      GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
      GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull = GPIO_PULLUP;
      GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
      GPIO_InitStruct.Alternate = GPIO_AF0_LPUART1;
      HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

      /* Peripheral interrupt init*/
      HAL_NVIC_SetPriority(LPUART1_IRQn, 0, 0);
      HAL_NVIC_EnableIRQ(LPUART1_IRQn);
      __HAL_UART_ENABLE_IT(&dbg_uart, UART_IT_RXNE);
   }
   else if (huart->Instance == USART1)
   {
      /* Peripheral clock enable */
      __HAL_RCC_USART1_CLK_ENABLE();

      /**USART1 GPIO Configuration
      PA9     ------> USART1_TX
      PA10     ------> USART1_RX
      */
      GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
      GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull = GPIO_PULLUP;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
      GPIO_InitStruct.Alternate = GPIO_AF4_USART1;
      HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

      /* Peripheral interrupt init*/
      HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
      HAL_NVIC_EnableIRQ(USART1_IRQn);
      __HAL_UART_ENABLE_IT(&comm_uart, UART_IT_RXNE);
   }
}

/*
Отправка запросов на тепловычетатель через UART.
@param1 указатель на буффер данных.
@param2 размер передаваемых данных.
@retval None.
*/
void sendRequest(uint8_t* pData, uint16_t Size)
{
   HAL_UART_Transmit(&comm_uart, pData, Size, 0xFFFF);
}

int acceptAnswer(uint8_t* pData, uint16_t Size)
{
   if (Size > lenCommRxBuf())
   {
      return -1;
   }
   loop(Size)
   {
      pData[i] = comm_receive_buffer[i];
      comm_index_rx--;
   }
   return 0;
}

uint16_t lenCommRxBuf(void)
{
   return comm_index_rx;
}

#ifdef __USE_DEBUG_UART__
PUTCHAR_PROTOTYPE
{
   HAL_UART_Transmit(&dbg_uart, (uint8_t*)&ch, 1, 0xFFFF);
   return ch;
}
#endif

/* INTERRUPT ******************************************************************/
/**
 * @brief Handle UART interrupt request.
 * @param huart: UART handle.
 * @retval None
 */
void LPUART1_IRQHandler(void)
{
   static uint16_t dbg_index_rx = 0;
   dbg_receive_buffer[dbg_index_rx] = (uint16_t)READ_REG(dbg_uart.Instance->RDR);

   if (dbg_receive_buffer[dbg_index_rx] == '\r')
   {
      // ParsingCallback(dbg_receive_buffer, index_rx);
      dbg_index_rx = 0;
   }
   else
   {
      dbg_index_rx++;
   }

   if (dbg_index_rx > sizeof(dbg_receive_buffer))
   {   //переполнение буфера
      dbg_index_rx = 0;
   }
}

/**
 * @brief Handle UART interrupt request.
 * @param huart: UART handle.
 * @retval None
 */
void USART1_IRQHandler(void)
{
   comm_receive_buffer[comm_index_rx] = (uint16_t)READ_REG(comm_uart.Instance->RDR);
   comm_index_rx++;
   if (comm_index_rx > sizeof(comm_receive_buffer))
   {   //переполнение буфера
      comm_index_rx = 0;
   }
}