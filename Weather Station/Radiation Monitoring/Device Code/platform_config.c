
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
   RCC_APB2PeriphClockCmd(PORT_LED_CLK, ENABLE);
   GPIO_InitStructure.GPIO_Pin = LED;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_Init(PORT_LED, &GPIO_InitStructure);
   LED_OFF;

   // Init Buzzer
   RCC_APB2PeriphClockCmd(PORT_BUZ_CLK, ENABLE);
   GPIO_InitStructure.GPIO_Pin = BUZ;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_Init(PORT_BUZ, &GPIO_InitStructure);
   BUZ_OFF;

   /* WIFI ESP */
   GPIO_InitStructure.GPIO_Pin = ESP_CP_PD_PIN;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_Init(ESP_CP_PD_PORT, &GPIO_InitStructure);
   ESP_CP_PD_OFF;

   GPIO_InitStructure.GPIO_Pin = ESP_RST_PIN;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_Init(ESP_RST_PORT, &GPIO_InitStructure);
   GPIO_HIGH(ESP_RST_PORT, ESP_RST_PIN);

   /* RADIO */
   GPIO_InitStructure.GPIO_Pin = DRF_EN_PIN;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_Init(DRF_EN_PORT, &GPIO_InitStructure);
   DRF_ENABLE;

   GPIO_InitStructure.GPIO_Pin = DRF_SET_PIN;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_Init(DRF_SET_PORT, &GPIO_InitStructure);
   DRF_SET_DISABLE;
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

   TIM_SetCompare1(TIM3, usTIMET1_CCR1_Val);
   TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
   TIM_ARRPreloadConfig(TIM3, ENABLE);

   /* TIM1 enable counter */
   TIM_Cmd(TIM3, ENABLE);
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
   IWDG_SetPrescaler(IWDG_Prescaler_128);

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

void EXTI0_Config(void)
{
   EXTI_InitTypeDef EXTI_InitStructure;
   GPIO_InitTypeDef GPIO_InitStructure;
   NVIC_InitTypeDef NVIC_InitStructure;

   /* Enable GPIOA clock */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

   /* Configure PA.00 pin as input floating */
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
   GPIO_Init(GPIOA, &GPIO_InitStructure);

   /* Enable AFIO clock */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

   /* Connect EXTI0 Line to PA.00 pin */
   GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);

   /* Configure EXTI0 line */
   EXTI_InitStructure.EXTI_Line = EXTI_Line0;
   EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
   EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
   EXTI_InitStructure.EXTI_LineCmd = ENABLE;
   EXTI_Init(&EXTI_InitStructure);

   /* Enable and set EXTI0 Interrupt to the lowest priority */
   NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
}
