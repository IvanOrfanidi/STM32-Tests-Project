
// User functions //
#include "includes.h"
#include "platform_config.h"

void InitGPIO(void)
{
   // Init GPIO Structure
   GPIO_InitTypeDef GPIO_InitStructure;

   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD,
                          ENABLE);

   // Init Led Light
   RCC_APB2PeriphClockCmd(PORT_LED_CLK, ENABLE);

   // Init Led Light Pin
   GPIO_InitStructure.GPIO_Pin = LED;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_Init(PORT_LED, &GPIO_InitStructure);
}

void InitTIM3(void)
{
   TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
   TIM_OCInitTypeDef TIM_OCInitStructure;

   /* TIM3 clock enable */
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

   TIM_TimeBaseStructure.TIM_Prescaler = (uint16_t)(SystemCoreClock / 36000000) - 1;

   TIM_TimeBaseStructure.TIM_Period = TIM3_PERIOD;
   TIM_TimeBaseStructure.TIM_ClockDivision = 0;
   TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
   TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

   /* PWM1 Mode configuration: Channel1 */
   TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
   TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
   TIM_OCInitStructure.TIM_Pulse = 0;
   TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
   TIM_OC1Init(TIM3, &TIM_OCInitStructure);

   TIM_SetCompare1(TIM3, usTIMET3_CCR1_Val);
   TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
   TIM_ARRPreloadConfig(TIM3, ENABLE);

   /* TIM1 enable counter */
   TIM_Cmd(TIM3, ENABLE);
}

void InitTIM4(void)
{
   TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
   TIM_OCInitTypeDef TIM_OCInitStructure;

   // Enable the TIM4 Clock
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

   TIM_TimeBaseStructure.TIM_Prescaler = (uint16_t)(SystemCoreClock / (36000000 / 2)) - 1;

   TIM_TimeBaseStructure.TIM_Period = TIM4_PERIOD;
   TIM_TimeBaseStructure.TIM_ClockDivision = 0;
   TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
   TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

   /* PWM1 Mode configuration: Channel1 */
   TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
   TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
   TIM_OCInitStructure.TIM_Pulse = 0;
   TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
   TIM_OC1Init(TIM4, &TIM_OCInitStructure);

   TIM_SetCompare1(TIM4, usTIMET4_CCR1_Val);

   TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);

   TIM_ARRPreloadConfig(TIM4, ENABLE);

   /* TIM4 enable counter */
   TIM_Cmd(TIM4, ENABLE);
}

void InitBKP(void)
{
   /* Enable PWR and BKP clock */
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP, ENABLE);

   /* Enable write access to Backup domain */
   PWR_BackupAccessCmd(ENABLE);

   /* Clear Tamper pin Event(TE) pending flag */
   BKP_ClearFlag();
}

void InitIWDG(void)
{
   /* Enable the LSI OSC */
   RCC_LSICmd(ENABLE);

   /* Wait till LSI is ready */
   while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
   {
   }

   /* IWDG timeout equal to 2000 ms (the timeout may varies due to LSI frequency
    dispersion) */
   /* Enable write access to IWDG_PR and IWDG_RLR registers */
   IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

   /* IWDG counter clock: LSI/32 */
   IWDG_SetPrescaler(IWDG_Prescaler_64);

/* Set counter reload value to obtain 250ms IWDG TimeOut.
   Counter Reload Value = 250ms/IWDG counter clock period
                        = 250ms / (LSI/32)
                        = 0.25s / (LsiFreq/32)
                        = LsiFreq/(32 * 4)
                        = LsiFreq/128
 */
#define LSI_FREQ 40000
   // IWDG_SetReload(LSI_FREQ/128);
   IWDG_SetReload(LSI_FREQ / 1);

   /* Reload IWDG counter */
   IWDG_ReloadCounter();

   /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
   IWDG_Enable();
}

void RCC_Configuration(void)
{
   RCC_ClocksTypeDef RCC_ClockFreq;

   /* RCC system reset(for debug purpose) */
   RCC_DeInit();

   /* Enable HSE */
   RCC_HSEConfig(RCC_HSE_ON);

   /* Wait till HSE is ready */
   ErrorStatus HSEStartUpStatus = RCC_WaitForHSEStartUp();

   if (HSEStartUpStatus != ERROR)
   {
      /* Enable Prefetch Buffer */
      FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

      /****************************************************************/
      /*      HSE=25MHz, HCLK=72MHz, PCLK2=72MHz, PCLK1=36MHz         */
      /****************************************************************/
      /* Flash 2 wait state */
      FLASH_SetLatency(FLASH_Latency_2);
      /* HCLK = SYSCLK */
      RCC_HCLKConfig(RCC_SYSCLK_Div1);
      /* PCLK2 = HCLK */
      RCC_PCLK2Config(RCC_HCLK_Div1);
      /* PCLK1 = HCLK/2 */
      RCC_PCLK1Config(RCC_HCLK_Div2);
      /*  ADCCLK = PCLK2/4 */
      RCC_ADCCLKConfig(RCC_PCLK2_Div6);

      /* Configure PLLs *********************************************************/
      /* PPL2 configuration: PLL2CLK = (HSE / 5) * 8 = 40 MHz */
      RCC_PREDIV2Config(RCC_PREDIV2_Div5);
      RCC_PLL2Config(RCC_PLL2Mul_8);

      /* Enable PLL2 */
      RCC_PLL2Cmd(ENABLE);

      /* Wait till PLL2 is ready */
      while (RCC_GetFlagStatus(RCC_FLAG_PLL2RDY) == RESET)
      {
      }

      /* PPL1 configuration: PLLCLK = (PLL2 / 5) * 9 = 72 MHz */
      RCC_PREDIV1Config(RCC_PREDIV1_Source_PLL2, RCC_PREDIV1_Div5);
      RCC_PLLConfig(RCC_PLLSource_PREDIV1, RCC_PLLMul_9);

      /* Enable PLL */
      RCC_PLLCmd(ENABLE);

      /* Wait till PLL is ready */
      while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
      {
      }

      /* Select PLL as system clock source */
      RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

      /* Wait till PLL is used as system clock source */
      while (RCC_GetSYSCLKSource() != 0x08)
      {
      }
   }

   RCC_GetClocksFreq(&RCC_ClockFreq);

   /* Enable USART2 clock */
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

   /* Enable ETHERNET clock  */
   RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ETH_MAC | RCC_AHBPeriph_ETH_MAC_Tx | RCC_AHBPeriph_ETH_MAC_Rx, ENABLE);

   /* Enable GPIOs clocks */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD |
                             RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO,
                          ENABLE);
   /* Enable ADC1 clock */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
}

void ETH_GPIO_Configuration(void)
{
   GPIO_InitTypeDef GPIO_InitStructure;

   /* ETHERNET pins remapp disable*/
   GPIO_PinRemapConfig(GPIO_Remap_ETH, DISABLE);

   /* ETHERNET pins configuration */
   /* AF Output Push Pull:
   - ETH_MII_MDIO: PA2
   - ETH_MII_MDC: PC1
   - ETH_MII_TX_EN: PB11
   - ETH_MII_TXD0: PB12
   - ETH_MII_TXD1: PB13
   - ETH_MII_TXD2: PB12
   - ETH_MII_TXD3: PB13
   - ETH_MII_PPS_OUT: PB5*/
   /* Input (Reset Value):
   - ETH_MII_REF_CLK: PA1
   - ETH_MII_CRS_DV: PA7
   - ETH_MII_RXD0: PC4
   - ETH_MII_RXD1: PC5*/

   /* Configure PA2 as alternate function push-pull */
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
   GPIO_Init(GPIOA, &GPIO_InitStructure);
   /* Configure PC1, PC2 as alternate function push-pull*/
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_1;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
   GPIO_Init(GPIOC, &GPIO_InitStructure);

   /* Configure PB8, PB11, PB12 and PB13 as alternate function push-pull */
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
   GPIO_Init(GPIOB, &GPIO_InitStructure);

   /* Configure PA0, PA1, PA3 and PA7 as input */
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_7;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
   GPIO_Init(GPIOA, &GPIO_InitStructure);

   /* Configure PB0, PB1 and PB10 as input */
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_10;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
   GPIO_Init(GPIOB, &GPIO_InitStructure);

   /* Configure PC3, PC4, PC5 as input */
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
   GPIO_Init(GPIOC, &GPIO_InitStructure); /**/

   /* MCO pin configuration------------------------------------------------- */
   /* Configure MCO (PA8) as alternate function push-pull */
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
   GPIO_Init(GPIOA, &GPIO_InitStructure);

   /* ADC Channel10 config --------------------------------------------------------*/
   /* Configure PC.0 (ADC Channel10) as analog input -------------------------*/
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
   GPIO_Init(GPIOC, &GPIO_InitStructure);
}
