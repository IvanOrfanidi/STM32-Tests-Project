
// User functions //
#include "includes.h"
#include "platform_config.h"

void InitGPIO(void)
{
#ifdef USE_LED
    GPIO_InitTypeDef GPIO_InitStructure;
    // Init Led Light
    RCC_APB2PeriphClockCmd(PORT_LED_CLK, ENABLE);
    // Init Led Light Pin
    GPIO_InitStructure.GPIO_Pin = LED_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(PORT_LED, &GPIO_InitStructure);
    LED_OFF;
#endif

#ifdef USE_BUZZER
#ifndef USE_LED
    GPIO_InitTypeDef GPIO_InitStructure;
#endif
    // Init Led Light
    RCC_APB2PeriphClockCmd(PORT_BUZ_CLK, ENABLE);

    GPIO_InitStructure.GPIO_Pin = BUZ_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(PORT_BUZ, &GPIO_InitStructure);
    BUZ_OFF;
#endif

#ifdef USE_BUTTON
#ifndef USE_BUZZER
    GPIO_InitTypeDef GPIO_InitStructure;
#endif
    RCC_APB2PeriphClockCmd(PORT_BUT_CLK, ENABLE);
    GPIO_InitStructure.GPIO_Pin = BUT_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(PORT_BUT, &GPIO_InitStructure);
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

void InitIWDG(void)
{
    /* Enable the LSI OSC */
    RCC_LSICmd(ENABLE);

    /* Wait till LSI is ready */
    while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET) {
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

void SleepMasterDevice(void)
{
    /* Allow access to BKP Domain */
    PWR_BackupAccessCmd(ENABLE);
    PWR_EnterSTANDBYMode();    //  -_-zZ
}

void SleepSlaveDevice(void)
{
#if 0
  NVIC_InitTypeDef   NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = RTCAlarm_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  EXTI_InitTypeDef   EXTI_InitStructure;
  EXTI_ClearITPendingBit(EXTI_Line17);
  EXTI_InitStructure.EXTI_Line = EXTI_Line17;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  
  /* включить тактирование интерфейса доступа к регистрам часов и Backup domain */
  RCC->APB1ENR |= RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN;
  osDelay(200);
  /* установить бит  DBP регистра PWR_CR, разрешающий доступ к  Backup domain */
  PWR->CR |= PWR_CR_DBP;
  while(!(PWR->CR & PWR_CR_DBP)) {
     __NOP();
  }
  RTC_SetCounter(1262304000);
  RTC_WaitForSynchro();
  RTC_WaitForLastTask();
  RTC_SetAlarm(1262304000 + 10);
  RTC_WaitForSynchro();
  RTC_WaitForLastTask();
#endif
#if 1
    /* Enable WKUP pin */
    PWR_WakeUpPinCmd(ENABLE);

    /* Allow access to BKP Domain */
    PWR_BackupAccessCmd(ENABLE);
    PWR_EnterSTANDBYMode();    //  -_-zZ
#endif
}