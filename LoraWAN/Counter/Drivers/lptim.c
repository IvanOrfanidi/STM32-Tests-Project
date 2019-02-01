#include "lptim.h"
#include "stm32l0xx_hal.h"
#include "rtc.h"

/* External variables --------------------------------------------------------*/
extern RTC_HandleTypeDef hrtc;

/* Private variables ---------------------------------------------------------*/
LPTIM_HandleTypeDef hlptim1;

/* LPTIM1 init function */
void MX_LPTIM1_Init(void)
{
    hlptim1.Instance = LPTIM1;
    hlptim1.Init.Clock.Source = LPTIM_CLOCKSOURCE_ULPTIM;
    hlptim1.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV1;
    hlptim1.Init.UltraLowPowerClock.Polarity = LPTIM_CLOCKPOLARITY_FALLING;
    hlptim1.Init.UltraLowPowerClock.SampleTime = LPTIM_CLOCKSAMPLETIME_DIRECTTRANSITION;
    hlptim1.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
    hlptim1.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_LOW;
    hlptim1.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
    hlptim1.Init.CounterSource = LPTIM_COUNTERSOURCE_EXTERNAL;
    if(HAL_LPTIM_Init(&hlptim1) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }
    HAL_LPTIM_Counter_Start_IT(&hlptim1, 0xFFFF);
}

void HAL_LPTIM_MspInit(LPTIM_HandleTypeDef* hlptim)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    if(hlptim->Instance == LPTIM1) {
        /* Peripheral clock enable */
        __HAL_RCC_LPTIM1_CLK_ENABLE();

        /**LPTIM1 GPIO Configuration
      PC0     ------> LPTIM1_IN1
      */
        GPIO_InitStruct.Pin = GPIO_PIN_0;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF0_LPTIM1;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    }
}

void HAL_LPTIM_MspDeInit(LPTIM_HandleTypeDef* hlptim)
{
    if(hlptim->Instance == LPTIM1) {
        /* Peripheral clock disable */
        __HAL_RCC_LPTIM1_CLK_DISABLE();

        /**LPTIM1 GPIO Configuration
      PC0     ------> LPTIM1_IN1
      */
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_0);
        HAL_NVIC_DisableIRQ(LPTIM1_IRQn);
    }
}

/*
ѕолучить значение счетного регистра(по факту регистра backup).
Input: no.
Return: значение счетного регистра.
*/
uint32_t getLtimCountVal(void)
{
    extern LPTIM_HandleTypeDef hlptim1;
    uint32_t countLptim = HAL_LPTIM_ReadCounter(&hlptim1);    // Get value counter
    uint32_t countCurVal = HAL_RTCEx_BKUPRead(&hrtc, DEF_REG_COUNT_VAL_LPTIM) & 0xFFFF0000;
    countCurVal |= countLptim;
    HAL_RTCEx_BKUPWrite(&hrtc, DEF_REG_COUNT_VAL_LPTIM, countCurVal);
    extern _Bool __DEBUG__;
    if(__DEBUG__) {
        printf("Counter Value = %0.0f\r\n", (float)countCurVal);
    }
    return countCurVal;
}

/** INTERRUPT COLLBACK ********************************************************/
/*
ќбщий обработчик перерываний от LPTIM.
*/
void LPTIM1_IRQHandler(void)
{
    HAL_LPTIM_IRQHandler(&hlptim1);
}
/******************************************************************************/

/*
ѕрерывание таймера куда он вылетит после переполнени€.
ќбработчик переполнени€ аппаротно-счетного регистра lptima(CNT).
*/
void HAL_LPTIM_AutoReloadMatchCallback(LPTIM_HandleTypeDef* hlptim)
{
    /* Disable all IRQs */
    __disable_irq();
    __DSB();
    __ISB();

    /* обрабатываем переполнение счетного регистра lptima */
    uint32_t Data = HAL_RTCEx_BKUPRead(&hrtc, DEF_REG_COUNT_VAL_LPTIM);
    Data >>= 16;
    Data++;
    Data <<= 16;
    HAL_RTCEx_BKUPWrite(&hrtc, DEF_REG_COUNT_VAL_LPTIM, Data);

    /* Enable IRQs */
    __enable_irq();
}
