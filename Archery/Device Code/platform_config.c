
// User functions //
#include "includes.h"
#include "platform_config.h"

void InitGPIO(void)
{
    // Init GPIO Structure
    GPIO_InitTypeDef GPIO_InitStructure;

#ifdef USE_LED
    // Init Led Light
    RCC_APB2PeriphClockCmd(PORT_LED_CLK, ENABLE);
    // Init Led Light Pin
    GPIO_InitStructure.GPIO_Pin = LED;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(PORT_LED, &GPIO_InitStructure);
    LED_OFF;
#endif

    // Init But
    RCC_APB2PeriphClockCmd(PORT_BUT_CLK, ENABLE);
    GPIO_InitStructure.GPIO_Pin = BUT;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(PORT_BUT, &GPIO_InitStructure);

    // Init Charging
    RCC_APB2PeriphClockCmd(PORT_CHARGING_CLK, ENABLE);
    GPIO_InitStructure.GPIO_Pin = CHARGING_COMPLETED_GPIO_PIN | CHARGING_POWER_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(CHARGING_GPIO_PORT, &GPIO_InitStructure);

#ifdef USE_BUZ
    RCC_APB2PeriphClockCmd(PORT_BUZ_CLK, ENABLE);

    // Init Led Light Pin
    GPIO_InitStructure.GPIO_Pin = BUZ_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(PORT_BUZ, &GPIO_InitStructure);
    // GPIO_PinRemapConfig(GPIO_FullRemap_TIM1, ENABLE);
#endif
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

void SleepDevice(void)
{
    /* Enable WKUP pin */
    PWR_WakeUpPinCmd(ENABLE);

    /* Allow access to BKP Domain */
    PWR_BackupAccessCmd(ENABLE);

    PWR_EnterSTANDBYMode();    //  -_-zZ
}

void InitTIM_BUZ(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    // Enable the TIM4 Clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    TIM_TimeBaseStructure.TIM_Prescaler = (uint16_t)(SystemCoreClock / 36000000) - 1;

    TIM_TimeBaseStructure.TIM_Period = TIM_BUZ_PERIOD;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    /* PWM1 Mode configuration: Channel1 */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM4, &TIM_OCInitStructure);

    TIM_SetCompare1(TIM4, usTIM_BUZ_CCR1_Val);

    TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);

    TIM_ARRPreloadConfig(TIM4, ENABLE);

    /* TIM4 enable counter */
    TIM_Cmd(TIM4, ENABLE);
}