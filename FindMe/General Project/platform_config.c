
#ifndef BOOTLOADER
#include "includes.h"
#endif

void PVD_Config(void)
{
#ifdef _PWR_CONTROL_ENABLE

    NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;

    /* Enable PWR clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

    /* Enable the PVD Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = PVD_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 16;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Configure EXTI Line16(PVD Output) to generate an interrupt on rising and
      falling edges */
    EXTI_ClearITPendingBit(EXTI_Line16);
    EXTI_InitStructure.EXTI_Line = EXTI_Line16;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Configure the PVD Level to 1 (2.1V) */
    PWR_PVDLevelConfig(PWR_PVDLevel_5);

    /* Enable the PVD Output */
    PWR_PVDCmd(ENABLE);
#endif
}

// Тактирование портов //
void RCC_Configuration(void)
{
    // Enable the GPIOs clocks
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOC, ENABLE);

#ifdef __USART_H
    // USART2, USART3
    RCC_APB1PeriphClockCmd(USART2_CLK | USART3_CLK, ENABLE);
    // USART1, ADC1 clock enable
    RCC_APB2PeriphClockCmd(USART1_CLK, ENABLE);
#endif

#ifdef _SPI_H_
    // Enable SPI1 //
    RCC_APB2PeriphClockCmd(SPI1_CLK, ENABLE);
#endif
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
}

// Конфигурация портов ввода/вывода //
void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Init LEDS */
    GPIO_InitStructure.GPIO_Pin = LED_MCU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_Init(LED_PORT, &GPIO_InitStructure);
    LED_OFF;

    /* GSM REFERENCE */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Pin = GSM_REF_PIN;
    GPIO_Init(GSM_REF_PORT, &GPIO_InitStructure);
    GPIO_LOW(GSM_REF_PORT, GSM_REF_PIN);

    /* GPS REFERENCE */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Pin = GPS_REF_PIN;
    GPIO_Init(GPS_REF_PORT, &GPIO_InitStructure);
    GPIO_LOW(GPS_REF_PORT, GPS_REF_PIN);

#ifdef GSM_MODULE_SIM800
    /* Output PWR KEY SIM800 */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Pin = GSM_PWR_KEY_PIN;
    GPIO_Init(GSM_PWR_KEY_PIN_PORT, &GPIO_InitStructure);
    PWR_KEY_PULL_UP;
#endif

    /* Configure FLASH_SPI_CS_PIN pin: sEE_SPI Card CS pin */
    GPIO_InitStructure.GPIO_Pin = FLASH_SPI_CS_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(FLASH_SPI_CS_GPIO_PORT, &GPIO_InitStructure);
    FLASH_CS_OFF;

    /* Configure ACCEL_SPI_CS_PIN pin: sEE_SPI Card CS pin */
    GPIO_InitStructure.GPIO_Pin = ACCEL_SPI_CS_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(ACCEL_SPI_CS_GPIO_PORT, &GPIO_InitStructure);
    ACCEL_CS_OFF;

    /* Configure SIM PIN ON/OFF */
    GPIO_InitStructure.GPIO_Pin = SIM_ON_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(SIM_ON_PORT, &GPIO_InitStructure);
    SIM_OFF;
    SIM_ON;
}

void GPS_Reference(VALUE prm)
{
    if(prm == ON) {
        GPIO_LOW(GPS_REF_PORT, GPS_REF_PIN);
    }
    else {
        GPIO_HIGH(GPS_REF_PORT, GPS_REF_PIN);
    }
}

void GSM_Reference(uint8_t prm)
{
    if(prm) {
        DPS("-GSM PWR ON-\r\n");
        GPIO_HIGH(GSM_REF_PORT, GSM_REF_PIN);
    }
    else {
        DPS("-GSM PWR OFF-\r\n");
        GPIO_LOW(GSM_REF_PORT, GSM_REF_PIN);
    }
}

void IWDGInit(void)
{
#ifdef _WDG_CONTROL_ENABLE
    /* Enable the LSI OSC */
    RCC_LSICmd(ENABLE);

    /* Wait till LSI is ready */
    while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
        ;

    /* Enable write access to IWDG_PR and IWDG_RLR registers */
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

    /* IWDG counter clock: LSI/256 */
    IWDG_SetPrescaler(IWDG_Prescaler_256);

#define LSI_FREQ 32768
    IWDG_SetReload(LSI_FREQ / 32);

    /* Reload IWDG counter */
    IWDG_ReloadCounter();

    /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
    IWDG_Enable();
#endif
}

void CheckFuses(void)
{
#ifdef _CHECK_FUSES_ENABLE
    if(FLASH_OB_GetRDP() == RESET) {
        DPS("---SET FUSES---\r\n");
        for(uint32_t i = 0; i < 100000; i++) {
            /* Reload IWDG counter */
            IWDG_ReloadCounter();
            __NOP();
        }

        FLASH_OB_Unlock();    // Открывает возможность блокировать доступ байт.
        FLASH_OB_BORConfig(OB_BOR_LEVEL2);
        FLASH_OB_RDPConfig(OB_RDP_Level_1);
        FLASH_OB_Launch();    // Запустите опцию загрузки байт.
        FLASH_OB_Lock();
    }
#endif
}

uint32_t ReadNameNewFirmware(void)
{
    uint8_t DataBoot[256];
    uint32_t uiNameNewFirmware = 0;    //Имя новой прошивки во внешней flash.

    char* p1 = (char*)&uiNameNewFirmware;
    char* p2 = (char*)&DataBoot[240];
    EXT_FLASH_Read(DataBoot, ADDR_EXT_FLASH_NEW_FIRMWARE + 256 * 487, 256);
    for(uint8_t i = 0; i < sizeof(uiNameNewFirmware); i++) {
        p1[i] = p2[i];
    }

    return uiNameNewFirmware;
}

uint32_t ReadNameBaseFirmware(void)
{
    char TempBuf[256];
    uint32_t uiNameBaseFirmware = 0;    //Имя новой прошивки во внешней flash.

    memset(TempBuf, 0, sizeof(TempBuf));
    char* p1 = (char*)&uiNameBaseFirmware;
    char* p2 = (char*)&TempBuf[240];

    if(osKernelRunning() == 1) {
        xSemaphoreTake(xBinSemFLASH, portMAX_DELAY);
    }
    EXT_FLASH_Read((uint8_t*)TempBuf, ADDR_EXT_FLASH_BASE_FIRMWARE + 256 * 487, 256);

    if(osKernelRunning() == 1) {
        xSemaphoreGive(xBinSemFLASH);
    }
    for(uint8_t i = 0; i < sizeof(uiNameBaseFirmware); i++) {
        p1[i] = p2[i];
    }

    return uiNameBaseFirmware;
}