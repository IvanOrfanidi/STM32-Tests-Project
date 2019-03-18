
#include "includes.h"

uint16_t usTIMET4_CCR1_Val = TIM4_PERIOD / 2;

void RCC_Configuration(void)
{
    /* GPIO Clock Enable */
    RCC_APB2PeriphClockCmd(PORT_OLED_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(PORT_USB_M_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(PORT_BUZ_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(PORT_BUT_BOOT_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(PORT_BUT_SECOND_CLK, ENABLE);

    /* Enable the TIM4 Clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    /* Enable PWR and BKP clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP, ENABLE);

#if _SPI_ADXL345_PORT == 1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(ADXL345_SPI_CLK, ENABLE);
#elif _SPI_ADXL345_PORT == 2
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(ADXL345_SPI_CLK, ENABLE);
#endif
}

void GPIO_Configuration(void)
{
    // Init GPIO Structure
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(PORT_BUZ_CLK, ENABLE);
    BuzzerOff();

    /* POWER DISPLAY */
    RCC_APB2PeriphClockCmd(PORT_OLED_CLK, ENABLE);
    GPIO_InitStructure.GPIO_Pin = PWR_OLED_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(PWR_OLED_PORT, &GPIO_InitStructure);
    DISPLAY_PWR_OFF;

    /* BUTTON BOOT */
    RCC_APB2PeriphClockCmd(PORT_BUT_BOOT_CLK, ENABLE);
    GPIO_InitStructure.GPIO_Pin = BUT_BOOT_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(BUT_BOOT_PORT, &GPIO_InitStructure);

    /* BUTTON SECOND */
    RCC_APB2PeriphClockCmd(PORT_BUT_SECOND_CLK, ENABLE);
    GPIO_InitStructure.GPIO_Pin = BUT_SECOND_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(BUT_SECOND_PORT, &GPIO_InitStructure);

    /* USB-M */
    RCC_APB2PeriphClockCmd(PORT_USB_M_CLK, ENABLE);
    GPIO_InitStructure.GPIO_Pin = USB_M_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(PORT_USB_M_PORT, &GPIO_InitStructure);
    USB_CONN_OFF;
}

void TIM4_Configuration(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    // Enable the TIM4 Clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    TIM_TimeBaseStructure.TIM_Prescaler = (uint16_t)(SystemCoreClock / (36000000 / 14)) - 1;

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

    /* TIM4 disable counter */
    TIM_Cmd(TIM4, DISABLE);
}

void BKP_Configuration(void)
{
    /* Enable PWR and BKP clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP, ENABLE);

    /* Enable write access to Backup domain */
    PWR_BackupAccessCmd(ENABLE);

    /* Clear Tamper pin Event(TE) pending flag */
    BKP_ClearFlag();
}

void IWDG_Configuration(void)
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
    // IWDG_SetReload(LSI_FREQ);

    /* Reload IWDG counter */
    IWDG_ReloadCounter();

    /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
    IWDG_Enable();
}

void GetBatMeasAndStatWkupPin(float* const pVbat, TStatWkupPin* const pStatWkupPin)
{
    ADCmeasCFG stAdcSource;
    GetAdcValue(&stAdcSource);
    *pVbat = ((stAdcSource.usBatMeasValue * 3300 / 4096.0) * BLEEDER_BAT) / 1000;
    float Vwkup = ((stAdcSource.usWkupPinValue * 3300 / 4096.0) * BLEEDER_WKUP);

    /* Фиксим подключение к USB */
    if(Vwkup > VBUT_WKP_USB_CONNECT) {
        *pStatWkupPin = USB_CONNECT;
        /* Фиксим нажатие кнопки при подключенном USB задяднике */
        if(Vwkup > VBUT_WKP_USB_CONNECT_BUT_ON) {
            *pStatWkupPin = BUT_ON;
        }
        return;
    }
    /* Фиксим нажатие кнопки без подключение зарядного устройства */
    if(Vwkup > VBUT_WKP_BUT_ON) {
        *pStatWkupPin = BUT_ON;
        return;
    }
    *pStatWkupPin = BUT_OFF;
}

void SleepDevice(void)
{
    /* Enable WKUP pin */
    PWR_WakeUpPinCmd(ENABLE);

    /* Allow access to BKP Domain */
    PWR_BackupAccessCmd(ENABLE);
    PWR_EnterSTANDBYMode();    //  -_-zZ
}

void BuzzerOn(void)
{
    // Init GPIO Structure
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = BUZ_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(BUZ_PORT, &GPIO_InitStructure);

    /* TIM4 enable counter */
    TIM_Cmd(TIM4, ENABLE);
}
void BuzzerOff(void)
{
    /* TIM4 disable counter */
    TIM_Cmd(TIM4, DISABLE);

    // Init GPIO Structure
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = BUZ_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(BUZ_PORT, &GPIO_InitStructure);
    GPIO_LOW(BUZ_PORT, BUZ_PIN);
}

uint32_t GetFlagsControlRegister(void)
{
    /*const char *pstrControlRegister [] = {
    "LPWRRSTF",      // Low-power reset flag. For further information on Low-power management reset.
    "WWDGRSTF",      // Window watchdog reset flag
    "IWDGRSTF",      // Independent watchdog reset flag (power-on reset value in set).
    "SFTRSTF",       // Software reset flag (power-on reset value in set).
    "PORRSTF",       // POR/PDR reset flag.
    "PINRSTF",       // PIN reset flag.
   };         // Remove reset flag.*/

    uint32_t StatReg = RCC->CSR;
// cout(" Control/status register:\r\n");
#define SIZE_MSG_CONTR_REG 8
    for(uint8_t i = 0, n = 31; i < SIZE_MSG_CONTR_REG; i++, n--) {
        if(StatReg & (1 << n)) { /*cout(" "); cout(pstrControlRegister[i]); cout("\r\n");*/
        }
    }
    RCC_ClearFlag();
    return StatReg;
}

uint16_t GetImageWelcome(void)
{
    return BKP_ReadBackupRegister(BKP_DR1);
}

void SetImageWelcome(uint16_t count_img_wel)
{
    BKP_WriteBackupRegister(BKP_DR1, count_img_wel);
}

void SetSound(_Bool sound)
{
    BKP_WriteBackupRegister(BKP_DR2, (uint16_t)sound);
}
_Bool GetSound(void)
{
    return (_Bool)BKP_ReadBackupRegister(BKP_DR2);
}

void SetShot(int shot)
{
    BKP_WriteBackupRegister(BKP_DR3, (uint16_t)shot);
}
int getShot(void)
{
    return (int)BKP_ReadBackupRegister(BKP_DR3);
}

_Bool fInitUsb = FALSE;
void usbInitDriver(void)
{
    if(fInitUsb)
        return;
    fInitUsb = TRUE;

    /* Configure USB Driver */
    USB_CONN_ON;
    Set_System();
    Set_USBClock();
    USB_Interrupts_Enable_Config();
    USB_Init();
}

void usbDeInitDriver(void)
{
    if(!(fInitUsb)) {
        return;
    }
    fInitUsb = FALSE;

    USB_CONN_OFF;
    Reset_System();
    Reset_USBClock();
    USB_Interrupts_Disable_Config();
}

/* Процесс вывода звука на зуммер */
void vBuzzerTask(void* pvParameters)
{
    portTickType xLastWakeTimerDelay;
    portTickType xTimeout = 0;

    while(TRUE) {
        if(!(xQueueReceive(xBuzQueue, &xTimeout, (TickType_t)10))) {
            xTimeout = 0;
        }

        if(GetSound() && xTimeout) {
            BUZ_ON;
            xLastWakeTimerDelay = xTaskGetTickCount();
            vTaskDelayUntil(&xLastWakeTimerDelay, (xTimeout / portTICK_RATE_MS));
            BUZ_OFF;
        }
        else {
            xLastWakeTimerDelay = xTaskGetTickCount();
            vTaskDelayUntil(&xLastWakeTimerDelay, (10 / portTICK_RATE_MS));
        }
    }
}

/* Sleep Device */
void ShutdownDevice(void)
{
    cout("Sleep Device\r\n");

    /* Выводим картинку завершения работы */
    msgShutdown();

    DisplayDisable();
    Accel_Sleep();

    SetShot(getCountShot());    // Save Shot

    portTickType xLastWakeTimerDelay = xTaskGetTickCount();
    vTaskDelayUntil(&xLastWakeTimerDelay, (200 / portTICK_RATE_MS));

    DISPLAY_PWR_OFF;
    USB_CONN_OFF;

    GPIO_DeInit(GPIOA);
    GPIO_DeInit(GPIOB);
    GPIO_DeInit(GPIOC);

    TIM_DeInit(TIM4);

    xLastWakeTimerDelay = xTaskGetTickCount();
    vTaskDelayUntil(&xLastWakeTimerDelay, (100 / portTICK_RATE_MS));

    SleepDevice();
}