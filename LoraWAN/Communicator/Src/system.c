
/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_hal.h"

/* Private variables ---------------------------------------------------------*/
IWDG_HandleTypeDef hiwdg;

/* NVIC Configuration */
void MX_NVIC_Init(void)
{
    /* ADC1_COMP_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(ADC1_COMP_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(ADC1_COMP_IRQn);

    /* RTC interrupt configuration */
    HAL_NVIC_SetPriority(RTC_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(RTC_IRQn);
}

uint32_t GetFlagsControlRegister(void)
{
    uint32_t StatReg = RCC->CSR;

    extern _Bool __DEBUG__;
    if(__DEBUG__) {
        const char* pstrControlRegister[] = {
            "LPWRRSTF",    // Low-power reset flag. For further information on Low-power management reset.
            "WWDGRSTF",    // Window watchdog reset flag
            "IWDGRSTF",    // Independent watchdog reset flag (power-on reset value in set).
            "SFTRSTF",     // Software reset flag (power-on reset value in set).
            "PORRSTF",     // POR/PDR reset flag.
            "PINRSTF",     // PIN reset flag.
            "OBLRSTF",     // Options bytes loading reset flag.
            "RMVF",
            "RTCRST"    // RTC software reset
        };              // Remove reset flag.

        printf("Control Register Flags:\r\n");
#define SIZE_MSG_CONTR_REG 9
        for(uint8_t i = 0, n = 31; i < SIZE_MSG_CONTR_REG; i++, n--) {
            if(StatReg & (1 << n)) {
                printf(" ");
                printf(pstrControlRegister[i]);
                printf("\r\n");
            }
        }
    }
    return StatReg;
}

/* IWDG init function */
void MX_IWDG_Init(void)
{
    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_64;
    hiwdg.Init.Window = 4095;
    hiwdg.Init.Reload = 4095;
    if(HAL_IWDG_Init(&hiwdg) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }
}

/**
 * Initializes the Global MSP.
 */
void HAL_MspInit(void)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    /* System interrupt init*/
    /* SVC_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SVC_IRQn, 0, 0);
    /* PendSV_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(PendSV_IRQn, 3, 0);
    /* SysTick_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SysTick_IRQn, 3, 0);
}

void stopDevice(void)
{
    // sleepRadio();               //!Функция перевода радиомодуля LoRa в сон.

    HAL_PWREx_EnableUltraLowPower();
    while(1)
        HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFE);
}

/** System Clock Configuration */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Configure the main internal regulator output voltage  */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Configure LSE Drive Capability
    */
    __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

    /**Initializes the CPU, AHB and APB busses clocks  */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_24;
    RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_3;
    if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }

    /**Initializes the CPU, AHB and APB busses clocks  */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LPUART1 | RCC_PERIPHCLK_RTC | RCC_PERIPHCLK_USB;

    PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_LSE;
    PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL;
    if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }

    /**Enables the Clock Security System
    */
    HAL_RCC_EnableCSS();

    /**Enables the Clock Security System
    */
    HAL_RCCEx_EnableLSECSS();

    /**Configure the Systick interrupt time
    */
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

    /**Configure the Systick
    */
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

    /* SysTick_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SysTick_IRQn, 3, 0);
}
