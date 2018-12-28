/**
 ******************************************************************************
 * @file    Examples_LL/SPI/SPI_OneBoard_HalfDuplex_DMA_Init/Src/main.c
 * @author  MCD Application Team
 * @brief   This example describes how to send/receive bytes over SPI IP using
 *          the STM32F7xx SPI LL API.
 *          Peripheral initialization done using LL initialization function.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/** @addtogroup STM32F7xx_LL_Examples
 * @{
 */

/** @addtogroup SPI_OneBoard_HalfDuplex_DMA_Init
 * @{
 */

/* Private typedef -----------------------------------------------------------*/
LL_SPI_InitTypeDef spi_initstruct;

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint8_t ubButtonPress = 0;

/* Buffer used for transmission */
uint8_t aTxBuffer[] = "**** SPI_OneBoard_HalfDuplex_DMA_Init communication **** SPI_OneBoard_HalfDuplex_DMA_Init "
                      "communication **** SPI_OneBoard_HalfDuplex_DMA_Init communication ****";
uint8_t ubNbDataToTransmit = sizeof(aTxBuffer);
__IO uint8_t ubTransmissionComplete = 0;

/* Buffer used for reception */
uint8_t aRxBuffer[sizeof(aTxBuffer)];
uint8_t ubNbDataToReceive = sizeof(aTxBuffer);
__IO uint8_t ubReceptionComplete = 0;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void Configure_DMA(void);
void Configure_SPI1(void);
void Configure_SPI3(void);
void Activate_SPI1(void);
void Activate_SPI3(void);
void LED_Init(void);
void LED_On(void);
void LED_Blinking(uint32_t Period);
void LED_Off(void);
void UserButton_Init(void);
void WaitForUserButtonPress(void);
void WaitAndCheckEndOfTransfer(void);
uint8_t Buffercmp8(uint8_t* pBuffer1, uint8_t* pBuffer2, uint8_t BufferLength);
static void CPU_CACHE_Enable(void);
/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Main program
 * @param  None
 * @retval None
 */
int main(void)
{
   /* Enable the CPU Cache */
   CPU_CACHE_Enable();
   /* Configure the system clock to 216 MHz */
   SystemClock_Config();

   /* Initialize LED1 */
   LED_Init();

   /* Configure the SPI1 parameters */
   Configure_SPI1();

   /* Configure the SPI3 parameters */
   Configure_SPI3();

   /* Configure DMA channels for the SPI1  */
   Configure_DMA();

   /* Initialize User push-button in EXTI mode */
   UserButton_Init();

   /* Enable the SPI3 peripheral */
   Activate_SPI3();

   /* Wait for User push-button press to start transfer */
   WaitForUserButtonPress();

   /* Enable the SPI1 peripheral */
   Activate_SPI1();

   /* Wait for the end of the transfer and check received data */
   /* LED blinking FAST during waiting time */
   WaitAndCheckEndOfTransfer();

   /* Infinite loop */
   while (1)
   {
   }
}

/**
 * @brief  This function configures the DMA Channels for SPI1 and SPI3
 * @note  This function is used to :
 *        -1- Enable DMA2 and DMA1 clock
 *        -2- Configure NVIC for DMA2 and DMA1 transfer complete/error interrupts
 *        -3- Configure the DMA2_Stream3 functional parameters
 *        -4- Configure the DMA1_Stream0 functional parameters
 *        -5- Enable DMA2 and DMA1 interrupts complete/error
 * @param   None
 * @retval  None
 */
void Configure_DMA(void)
{
   /* DMA2 used for SPI1 Transmission
    * DMA1 used for SPI3 Reception
    */
   /* (1) Enable the clock of DMA2 and DMA1 */
   LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);
   LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);

   /* (2) Configure NVIC for DMA transfer complete/error interrupts */
   NVIC_SetPriority(DMA2_Stream3_IRQn, 0);
   NVIC_EnableIRQ(DMA2_Stream3_IRQn);
   NVIC_SetPriority(DMA1_Stream0_IRQn, 0);
   NVIC_EnableIRQ(DMA1_Stream0_IRQn);

   /* (3) Configure the DMA2_Stream3 functional parameters */
   LL_DMA_ConfigTransfer(DMA2,
                         LL_DMA_STREAM_3,
                         LL_DMA_DIRECTION_MEMORY_TO_PERIPH | LL_DMA_PRIORITY_HIGH | LL_DMA_MODE_NORMAL |
                            LL_DMA_PERIPH_NOINCREMENT | LL_DMA_MEMORY_INCREMENT | LL_DMA_PDATAALIGN_BYTE |
                            LL_DMA_MDATAALIGN_BYTE);
   LL_DMA_ConfigAddresses(DMA2,
                          LL_DMA_STREAM_3,
                          (uint32_t)aTxBuffer,
                          LL_SPI_DMA_GetRegAddr(SPI1),
                          LL_DMA_GetDataTransferDirection(DMA2, LL_DMA_STREAM_3));

   LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_3, ubNbDataToTransmit);

   LL_DMA_SetChannelSelection(DMA2, LL_DMA_STREAM_3, LL_DMA_CHANNEL_3);
   /* (4) Configure the DMA1_Stream0 functional parameters */
   LL_DMA_ConfigTransfer(DMA1,
                         LL_DMA_STREAM_0,
                         LL_DMA_DIRECTION_PERIPH_TO_MEMORY | LL_DMA_PRIORITY_HIGH | LL_DMA_MODE_NORMAL |
                            LL_DMA_PERIPH_NOINCREMENT | LL_DMA_MEMORY_INCREMENT | LL_DMA_PDATAALIGN_BYTE |
                            LL_DMA_MDATAALIGN_BYTE);
   LL_DMA_ConfigAddresses(DMA1,
                          LL_DMA_CHANNEL_0,
                          LL_SPI_DMA_GetRegAddr(SPI3),
                          (uint32_t)aRxBuffer,
                          LL_DMA_GetDataTransferDirection(DMA1, LL_DMA_STREAM_0));
   LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_0, ubNbDataToTransmit);

   LL_DMA_SetChannelSelection(DMA1, LL_DMA_STREAM_0, LL_DMA_CHANNEL_0);

   /* (5) Enable DMA interrupts complete/error */
   LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_3);
   LL_DMA_EnableIT_TE(DMA2, LL_DMA_STREAM_3);
   LL_DMA_EnableIT_TC(DMA1, LL_DMA_STREAM_0);
   LL_DMA_EnableIT_TE(DMA1, LL_DMA_STREAM_0);
}

/**
 * @brief This function configures SPI1.
 * @note  This function is used to :
 *        -1- Enables GPIO clock and configures the SPI1 pins.
 *        -2- Configure SPI1 functional parameters.
 * @note   Peripheral configuration is minimal configuration from reset values.
 *         Thus, some useless LL unitary functions calls below are provided as
 *         commented examples - setting is default configuration from reset.
 * @param  None
 * @retval None
 */
void Configure_SPI1(void)
{
   /* (1) Enables GPIO clock and configures the SPI1 pins ********************/
   /* Enable the peripheral clock of GPIOA */
   LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

   /* Configure SCK Pin connected to pin 10 of CN7 connector */
   LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_5, LL_GPIO_MODE_ALTERNATE);
   LL_GPIO_SetAFPin_0_7(GPIOA, LL_GPIO_PIN_5, LL_GPIO_AF_5);
   LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_5, LL_GPIO_SPEED_FREQ_HIGH);
   LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_5, LL_GPIO_PULL_DOWN);

   /* Configure MOSI Pin connected to pin 14 of CN7 connector */
   LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_7, LL_GPIO_MODE_ALTERNATE);
   LL_GPIO_SetAFPin_0_7(GPIOA, LL_GPIO_PIN_7, LL_GPIO_AF_5);
   LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_7, LL_GPIO_SPEED_FREQ_HIGH);
   LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_7, LL_GPIO_PULL_DOWN);

   /* (2) Configure SPI1 functional parameters ********************************/
   /* Enable the peripheral clock of GPIOA */
   LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);

   /* Configure SPI1 communication */
   spi_initstruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV32;
   spi_initstruct.TransferDirection = LL_SPI_HALF_DUPLEX_TX;
   spi_initstruct.ClockPhase = LL_SPI_PHASE_2EDGE;
   spi_initstruct.ClockPolarity = LL_SPI_POLARITY_HIGH;
   spi_initstruct.BitOrder = LL_SPI_MSB_FIRST;
   spi_initstruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
   spi_initstruct.NSS = LL_SPI_NSS_SOFT;
   spi_initstruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
   spi_initstruct.Mode = LL_SPI_MODE_MASTER;

   /* Initialize SPI instance according to parameters defined in initialization structure. */
   if (LL_SPI_Init(SPI1, &spi_initstruct) != SUCCESS)
   {
      /* Initialization Error */
      LED_Blinking(LED_BLINK_ERROR);
   }

   /* Initialize FFIFO Threshold */
   LL_SPI_SetRxFIFOThreshold(SPI1, LL_SPI_RX_FIFO_TH_QUARTER);

   /* Configure SPI1 DMA transfer interrupts */
   /* Enable DMA TX Interrupt */
   LL_SPI_EnableDMAReq_TX(SPI1);
}

/**
 * @brief  This function configures SPI3.
 * @note  This function is used to :
 *        -1- Enables GPIO clock and configures the SPI3 pins.
 *        -2- Configure SPI3 functional parameters.
 * @note   Peripheral configuration is minimal configuration from reset values.
 *         Thus, some useless LL unitary functions calls below are provided as
 *         commented examples - setting is default configuration from reset.
 * @param  None
 * @retval None
 */
void Configure_SPI3(void)
{
   /* (1) Enables GPIO clock and configures the SPI3 pins ********************/

   /* Enable the peripheral clock of GPIOB */
   LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

   /* Configure SCK Pin connected to pin 15 of CN7 connector */
   LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_3, LL_GPIO_MODE_ALTERNATE);
   LL_GPIO_SetAFPin_0_7(GPIOB, LL_GPIO_PIN_3, LL_GPIO_AF_6);
   LL_GPIO_SetPinSpeed(GPIOB, LL_GPIO_PIN_3, LL_GPIO_SPEED_FREQ_HIGH);
   LL_GPIO_SetPinPull(GPIOB, LL_GPIO_PIN_3, LL_GPIO_PULL_DOWN);

   /* Configure MISO Pin connected to pin 19 of CN7 connector */
   LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_4, LL_GPIO_MODE_ALTERNATE);
   LL_GPIO_SetAFPin_0_7(GPIOB, LL_GPIO_PIN_4, LL_GPIO_AF_6);
   LL_GPIO_SetPinSpeed(GPIOB, LL_GPIO_PIN_4, LL_GPIO_SPEED_FREQ_HIGH);
   LL_GPIO_SetPinPull(GPIOB, LL_GPIO_PIN_4, LL_GPIO_PULL_DOWN);

   /* (2) Configure SPI3 functional parameters ********************************/
   /* Enable the peripheral clock of GPIOB */
   LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI3);

   /* Configure SPI3 communication */
   spi_initstruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV32;
   spi_initstruct.TransferDirection = LL_SPI_HALF_DUPLEX_RX;
   spi_initstruct.ClockPhase = LL_SPI_PHASE_2EDGE;
   spi_initstruct.ClockPolarity = LL_SPI_POLARITY_HIGH;
   spi_initstruct.BitOrder = LL_SPI_MSB_FIRST;
   spi_initstruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
   spi_initstruct.NSS = LL_SPI_NSS_SOFT;
   spi_initstruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
   spi_initstruct.Mode = LL_SPI_MODE_SLAVE;

   /* Initialize SPI instance according to parameters defined in initialization structure. */
   if (LL_SPI_Init(SPI3, &spi_initstruct) != SUCCESS)
   {
      /* Initialization Error */
      LED_Blinking(LED_BLINK_ERROR);
   }

   /* Initialize FFIFO Threshold */
   LL_SPI_SetRxFIFOThreshold(SPI3, LL_SPI_RX_FIFO_TH_QUARTER);

   /* Configure SPI1 DMA transfer interrupts */
   /* Enable DMA RX Interrupt */
   LL_SPI_EnableDMAReq_RX(SPI3);
}

/**
 * @brief  This function Activate SPI1
 * @param  None
 * @retval None
 */
void Activate_SPI1(void)
{
   /* Enable SPI1 */
   LL_SPI_Enable(SPI1);
   /* Enable DMA Stream Tx */
   LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_3);
}

/**
 * @brief  This function Activate SPI3
 * @param  None
 * @retval None
 */
void Activate_SPI3(void)
{
   /* Enable SPI3 */
   LL_SPI_Enable(SPI3);
   /* Enable DMA Stream Rx */
   LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_0);
}

/**
 * @brief  Initialize LED1.
 * @param  None
 * @retval None
 */
void LED_Init(void)
{
   /* Enable the LED1 Clock */
   LED1_GPIO_CLK_ENABLE();

   /* Configure IO in output push-pull mode to drive external LED1 */
   LL_GPIO_SetPinMode(LED1_GPIO_PORT, LED1_PIN, LL_GPIO_MODE_OUTPUT);
   /* Reset value is LL_GPIO_OUTPUT_PUSHPULL */
   // LL_GPIO_SetPinOutputType(LED1_GPIO_PORT, LED1_PIN, LL_GPIO_OUTPUT_PUSHPULL);
   /* Reset value is LL_GPIO_SPEED_FREQ_LOW */
   // LL_GPIO_SetPinSpeed(LED1_GPIO_PORT, LED1_PIN, LL_GPIO_SPEED_FREQ_LOW);
   /* Reset value is LL_GPIO_PULL_NO */
   // LL_GPIO_SetPinPull(LED1_GPIO_PORT, LED1_PIN, LL_GPIO_PULL_NO);
}

/**
 * @brief  Turn-on LED1.
 * @param  None
 * @retval None
 */
void LED_On(void)
{
   /* Turn LED1 on */
   LL_GPIO_SetOutputPin(LED1_GPIO_PORT, LED1_PIN);
}

/**
 * @brief  Turn-off LED1.
 * @param  None
 * @retval None
 */
void LED_Off(void)
{
   /* Turn LED1 off */
   LL_GPIO_ResetOutputPin(LED1_GPIO_PORT, LED1_PIN);
}

/**
 * @brief  Set LED1 to Blinking mode for an infinite loop (toggle period based on value provided as input parameter).
 * @param  Period : Period of time (in ms) between each toggling of LED
 *   This parameter can be user defined values. Pre-defined values used in that example are :
 *     @arg LED_BLINK_FAST : Fast Blinking
 *     @arg LED_BLINK_SLOW : Slow Blinking
 *     @arg LED_BLINK_ERROR : Error specific Blinking
 * @retval None
 */
void LED_Blinking(uint32_t Period)
{
   /* Toggle LED1 in an infinite loop */
   while (1)
   {
      LL_GPIO_TogglePin(LED1_GPIO_PORT, LED1_PIN);
      LL_mDelay(Period);
   }
}

/**
 * @brief  Configures User push-button in GPIO or EXTI Line Mode.
 * @param  None
 * @retval None
 */
void UserButton_Init(void)
{
   /* Enable the BUTTON Clock */
   USER_BUTTON_GPIO_CLK_ENABLE();

   /* Configure GPIO for BUTTON */
   LL_GPIO_SetPinMode(USER_BUTTON_GPIO_PORT, USER_BUTTON_PIN, LL_GPIO_MODE_INPUT);
   LL_GPIO_SetPinPull(USER_BUTTON_GPIO_PORT, USER_BUTTON_PIN, LL_GPIO_PULL_NO);

   /* Connect External Line to the GPIO*/
   USER_BUTTON_SYSCFG_SET_EXTI();

   /* Enable a rising trigger External line 13 Interrupt */
   USER_BUTTON_EXTI_LINE_ENABLE();
   USER_BUTTON_EXTI_FALLING_TRIG_ENABLE();

   /* Configure NVIC for USER_BUTTON_EXTI_IRQn */
   NVIC_EnableIRQ(USER_BUTTON_EXTI_IRQn);
   NVIC_SetPriority(USER_BUTTON_EXTI_IRQn, 0x03);
}

/**
 * @brief  Wait for User push-button press to start transfer.
 * @param  None
 * @retval None
 */
/*  */
void WaitForUserButtonPress(void)
{
   while (ubButtonPress == 0)
   {
      LL_GPIO_TogglePin(LED1_GPIO_PORT, LED1_PIN);
      LL_mDelay(LED_BLINK_FAST);
   }
   /* Ensure that LED1 is turned Off */
   LED_Off();
}

/**
 * @brief  Wait end of transfer and check if received Data are well.
 * @param  None
 * @retval None
 */
void WaitAndCheckEndOfTransfer(void)
{
   /* 1 - Wait end of transmission */
   while (ubTransmissionComplete != 1)
   {
   }
   /* Disable DMA2 Tx Channel */
   LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_3);
   /* 2 - Wait end of reception */
   while (ubReceptionComplete != 1)
   {
   }
   /* Disable DMA1 Rx Channel */
   LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_0);
   /* 3 - Compare Transmit data to receive data */
   if (Buffercmp8((uint8_t*)aTxBuffer, (uint8_t*)aRxBuffer, ubNbDataToTransmit))
   {
      /* Processing Error */
      LED_Blinking(LED_BLINK_ERROR);
   }
   else
   {
      /* Turn On Led if data are well received */
      LED_On();
   }
}

/**
 * @brief Compares two 8-bit buffers and returns the comparison result.
 * @param pBuffer1: pointer to the source buffer to be compared to.
 * @param pBuffer2: pointer to the second source buffer to be compared to the first.
 * @param BufferLength: buffer's length.
 * @retval   0: Comparison is OK (the two Buffers are identical)
 *           Value different from 0: Comparison is NOK (Buffers are different)
 */
uint8_t Buffercmp8(uint8_t* pBuffer1, uint8_t* pBuffer2, uint8_t BufferLength)
{
   while (BufferLength--)
   {
      if (*pBuffer1 != *pBuffer2)
      {
         return 1;
      }

      pBuffer1++;
      pBuffer2++;
   }

   return 0;
}

/**
 * @brief  System Clock Configuration
 *         The system Clock is configured as follow :
 *            System Clock source            = PLL (HSE)
 *            SYSCLK(Hz)                     = 216000000
 *            HCLK(Hz)                       = 216000000
 *            AHB Prescaler                  = 1
 *            APB1 Prescaler                 = 4
 *            APB2 Prescaler                 = 2
 *            HSI Frequency(Hz)              = 8000000
 *            PLL_M                          = 8
 *            PLL_N                          = 432
 *            PLL_P                          = 2
 *            VDD(V)                         = 3.3
 *            Main regulator output voltage  = Scale1 mode
 *            Flash Latency(WS)              = 7
 * @param  None
 * @retval None
 */
void SystemClock_Config(void)
{
   /* Enable HSE clock */
   LL_RCC_HSE_EnableBypass();
   LL_RCC_HSE_Enable();
   while (LL_RCC_HSE_IsReady() != 1)
   {
   };

   /* Set FLASH latency */
   LL_FLASH_SetLatency(LL_FLASH_LATENCY_7);

   /* Enable PWR clock */
   LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

   /* Activation OverDrive Mode */
   LL_PWR_EnableOverDriveMode();
   while (LL_PWR_IsActiveFlag_OD() != 1)
   {
   };

   /* Activation OverDrive Switching */
   LL_PWR_EnableOverDriveSwitching();
   while (LL_PWR_IsActiveFlag_ODSW() != 1)
   {
   };

   /* Main PLL configuration and activation */
   LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_8, 432, LL_RCC_PLLP_DIV_2);
   LL_RCC_PLL_Enable();
   while (LL_RCC_PLL_IsReady() != 1)
   {
   };

   /* Sysclk activation on the main PLL */
   LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
   LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
   while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
   {
   };

   /* Set APB1 & APB2 prescaler */
   LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_4);
   LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_2);

   /* Set systick to 1ms */
   SysTick_Config(216000000 / 1000);

   /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
   SystemCoreClock = 216000000;
}

/******************************************************************************/
/*   USER IRQ HANDLER TREATMENT Functions                                     */
/******************************************************************************/
/**
 * @brief  Function to manage User push-button
 * @param  None
 * @retval None
 */
void UserButton_Callback(void)
{
   /* Update User push-button variable : to be checked in waiting loop in main program */
   ubButtonPress = 1;
}

/**
 * @brief  Function called from DMA2 IRQ Handler when Tx transfer is completed
 * @param  None
 * @retval None
 */
void DMA2_TransmitComplete_Callback(void)
{
   /* DMA Tx transfer completed */
   ubTransmissionComplete = 1;
}

/**
 * @brief  Function called from DMA1 IRQ Handler when Rx transfer is completed
 * @param  None
 * @retval None
 */
void DMA1_ReceiveComplete_Callback(void)
{
   /* DMA Rx transfer completed */
   ubReceptionComplete = 1;
}

/**
 * @brief  Function called in case of error detected in SPI IT Handler
 * @param  None
 * @retval None
 */
void SPI_TransferError_Callback(void)
{
   /* Disable DMA2 Tx Channel */
   LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_3);
   /* Disable DMA1 Rx Channel */
   LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_0);

   /* Set LED1 to Blinking mode to indicate error occurs */
   LED_Blinking(LED_BLINK_ERROR);
}

#ifdef USE_FULL_ASSERT

/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
   /* User can add his own implementation to report the file name and line number,
      ex: printf("Wrong parameters value: file %s on line %d", file, line) */

   /* Infinite loop */
   while (1)
   {
   }
}
#endif
/**
 * @brief  CPU L1-Cache enable.
 * @param  None
 * @retval None
 */
static void CPU_CACHE_Enable(void)
{
   /* Enable I-Cache */
   SCB_EnableICache();

   /* Enable D-Cache */
   SCB_EnableDCache();
}
/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/