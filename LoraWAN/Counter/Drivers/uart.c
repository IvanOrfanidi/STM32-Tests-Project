#include "uart.h"
#include "stm32l0xx_hal.h"
#include "parser.h"

#include <string.h>
#include <ctype.h>

#ifdef __USE_DEBUG_UART__
/* Private function prototypes -----------------------------------------------*/
#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE* f)
#endif /* __GNUC__ */
#else
#warning "Debugging will be done by the standard stdio.h"
#endif

uint8_t receiveBuffer[RX_BUFFER_SIZE];
uint8_t* ptrBuffer = NULL;

UART_HandleTypeDef dbg_uart;

/* USART2 init function */
void MX_USART_DEBUG_Init(void)
{
    dbg_uart.Instance = LPUART1;
    dbg_uart.Init.BaudRate = DBG_UART;
    dbg_uart.Init.WordLength = UART_WORDLENGTH_8B;
    dbg_uart.Init.StopBits = UART_STOPBITS_1;
    dbg_uart.Init.Parity = UART_PARITY_NONE;
    dbg_uart.Init.Mode = UART_MODE_TX_RX;
    dbg_uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    dbg_uart.Init.OneBitSampling = UART_ONEBIT_SAMPLING_DISABLED;
    dbg_uart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if(HAL_UART_Init(&dbg_uart) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }
}

void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    if(huart->Instance == LPUART1) {
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
        /* USER CODE BEGIN LPUART1_MspInit 1 */

        __HAL_UART_ENABLE_IT(&hlpuart1, UART_IT_RXNE);
    }
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
    static uint16_t index_rx = 0;
    receiveBuffer[index_rx] = (uint16_t)READ_REG(dbg_uart.Instance->RDR);

    if(receiveBuffer[index_rx] == '\r') {
        ParsingCallback(receiveBuffer, index_rx);
        index_rx = 0;
    }
    else {
        index_rx++;
    }

    if(index_rx > sizeof(receiveBuffer)) {    //переполнение буфера
        index_rx = 0;
    }
}
