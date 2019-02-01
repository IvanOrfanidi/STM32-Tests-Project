#include "includes.h"
#include "power.h"

uint32_t g_uiSysMonitor = 0;

void NVIC_Deinit(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;

    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel3_IRQn;
    NVIC_Init(&NVIC_InitStructure);
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;
    NVIC_Init(&NVIC_InitStructure);
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel6_IRQn;
    NVIC_Init(&NVIC_InitStructure);
    NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
    NVIC_Init(&NVIC_InitStructure);
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_Init(&NVIC_InitStructure);
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
    NVIC_Init(&NVIC_InitStructure);
}

void RCC_Deinit(void)
{
    RCC_APB2PeriphClockCmd(USART1_CLK | RCC_APB2Periph_SPI1 | RCC_APB2Periph_ADC1, DISABLE);

    RCC_APB1PeriphClockCmd(USART2_CLK | USART3_CLK | RCC_APB1Periph_PWR | RCC_APB1Periph_TIM7 | RCC_APB1Periph_TIM4 |
                               RCC_APB1Periph_SPI2 | RCC_APB1Periph_SPI3,
        DISABLE);

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOC | RCC_AHBPeriph_DMA1, DISABLE);
}

void GPIO_Deinit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;

    GPIO_InitStructure.GPIO_Pin = LED_MCU;
    GPIO_Init(LED_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GSM_REF_PIN;
    GPIO_Init(GSM_REF_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPS_REF_PIN;
    GPIO_Init(GPS_REF_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GSM_PWR_KEY_PIN;
    GPIO_Init(GSM_PWR_KEY_PIN_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = FLASH_SPI_CS_PIN;
    GPIO_Init(FLASH_SPI_CS_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = ACCEL_SPI_CS_PIN;
    GPIO_Init(ACCEL_SPI_CS_GPIO_PORT, &GPIO_InitStructure);
}

void PVD_Config(void)
{
#ifdef _PWR_CONTROL_ENABLE

    NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;

    /* Enable PWR clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

    /* Enable the PVD Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = PVD_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1;
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

void updateStatusReset(void)
{
    uint32_t CurSecAlr;    //Время когда должны были проснуться
    uint32_t CurSecCur;    //Время когда проснулись

    RESET_STATUS_DEVICE eStatReset = (RESET_STATUS_DEVICE)GetStatusDeviceReset();
    if(eStatReset > 10) {    //Не штатная перезагрузка девайса.
        SetStatusReset(eStatReset);
        DP_GSM("D_WARNUNG RESET DEV ERR: %i\r\n", eStatReset);
        return;
    }

    CurSecCur = time();

    if((GetDeviceSleep() == 0) || (CurSecCur < DEF_TIME_DATA + 2)) {
        DPS("D_RESET DEV: <LOW POWER>\r\n");
        SetStatusReset(LOW_POWER);
        SetDeviceSleep(1000);    //
        return;
    }

    PWR_RTCAccessCmd(ENABLE);
    if(GetEnableAccelToFind()) {
        DPS("D_RESET DEV: ");
        RCC_AHBPeriphClockCmd(ACCEL_INT_GPIO_CLK, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

        /* Configure accel pin as input */
        GPIO_InitTypeDef GPIO_InitStructure;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_InitStructure.GPIO_Pin = ACCEL_INT_PIN;
        GPIO_Init(ACCEL_INT_GPIO_PORT, &GPIO_InitStructure);

        if(GPIO_ReadInputDataBit(ACCEL_INT_GPIO_PORT, ACCEL_INT_PIN)) {    //проверить флаг PINRSTF
            DPS("<ACCEL>\r\n");
            g_stRam.stAccel.eAccelState = ACC_STATE_MOVE;
            SetStatusReset(WAKE_UP_ACCEL);
            SetStatusDeviceReset(GetStatusReset());
        }
        else {
            if(RCC->CSR & RCC_CSR_PINRSTF) {    //Проснулись по нажатию тревожной кнопки
                DPS("<BUTTON_B>\r\n");
                SetStatusReset(BUTTON_RESET);
            }
            else {
                DPS("<WAKE UP>\r\n");
                SetStatusReset(WAKE_UP_ALARM);
                SetStatusDeviceReset(GetStatusReset());
            }
        }
    }
    else {
        CurSecAlr = GetDeviceSleep();
        if(CurSecAlr != 0xFFFFFFFF) {
            DP_GSM("D_TIME WUP: %d\r\n", CurSecAlr);
        }
        DP_GSM("D_TIME CUR: %d\r\n", CurSecCur);
        DPS("D_RESET DEV: ");

        if(CurSecCur + DELTA_TIME_OFFSET >= CurSecAlr) {
            SetStatusReset(WAKE_UP_ALARM);
            SetDeviceSleep(0xFFFFFFFF);
            DPS("<WAKE UP>\r\n");
        }
        else {    //Проснулись по нажатию тревожной кнопки
            if(RCC->CSR & RCC_CSR_PINRSTF) {
                SetStatusReset(BUTTON_RESET);
                DPS("<BUTTON_T>\r\n");
            }
        }
    }
}

uint32_t GetWakingTime(void)
{
    static uint32_t uiWakingTime = 0;
    if(!(uiWakingTime)) {
        uiWakingTime = time();
    }

    return uiWakingTime;
}

void SetStatusReset(RESET_STATUS_DEVICE eStatReset)
{
    g_stRam.stDevice.eResetStatusDevice = eStatReset;
}

RESET_STATUS_DEVICE GetStatusReset(void)
{
    return g_stRam.stDevice.eResetStatusDevice;
}

#pragma optimize = none
void RebootDevice(void)
{
    IWDG_ReloadCounter();
    vTaskSuspendAll();      // Stop All Task
    vTaskEndScheduler();    // Stop OS
    __disable_interrupt();
    while(1)
        ;    //Ждем перезагрузки по watchdogу.
}

void UpdateTimeSleepDevice(void)
{
    SetStatusReset(WAKE_UP_ALARM);
    SetStatusDeviceReset(GetStatusReset());
    InitAlarm();

    uint32_t CurSecAlr = GetDeviceSleep();    //Время точного пробуждения

    RTC_t DateRTC;
    Sec2Date(&DateRTC, CurSecAlr);
    DP_GSM("D_UPDATE WAKEUP DATE: %02d/", DateRTC.mday);
    DP_GSM("%02d/", DateRTC.month);
    DP_GSM("%02d ", DateRTC.year);
    DP_GSM("%02d:", DateRTC.hour);
    DP_GSM("%02d:", DateRTC.min);
    DP_GSM("%02d\r\n", DateRTC.sec);
    DelayResolution100us(100);

    int reset_alarm = SetAlarmTime(&DateRTC);    //прове6ряем установку пробуждения
    while(reset_alarm) {
        ResetAlarmTime();    //сбрасываем флаг тревоги естли не получилось установить тревогу
        DelayResolution100us(10);
        reset_alarm = SetAlarmTime(&DateRTC);
    }
    // SetDeviceSleep(CurSecAlr);      //Фиксируем время когда должны проснуться

    NVIC_Deinit();
    RCC_Deinit();
    GPIO_Deinit();

    /* Enable the RTC Clock */
    RCC_RTCCLKCmd(ENABLE);

    /* Enable the PWR clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    /* Enable the RTC Clock */
    RCC_RTCCLKCmd(ENABLE);
    /* Allow access to RTC */
    PWR_RTCAccessCmd(ENABLE);

    PWR_PVDCmd(DISABLE);
    PWR_UltraLowPowerCmd(ENABLE);

    // Set low power configuration
    RCC->AHBENR = 0;
    RCC->AHBLPENR = 0;
    // PWR management enable
    RCC->APB1ENR = RCC_APB1ENR_PWREN;
    RCC->APB2ENR = 0;
    SysTick->CTRL = 0;
    PWR_UltraLowPowerCmd(ENABLE);

    _Bool accel_enable = FALSE;
    if(GetModeProtocol() == FINDME_911 && GetEnableAccelToFind()) {    //Если включено просыпание от акселерометра и стоит режим поиска.
        accel_enable = TRUE;
    }

    if(GetModeProtocol() == ION_FM && GetModeDevice() == TIMER_FIND && GetEnableAccelToFind()) {
        accel_enable = TRUE;
    }

    if(accel_enable) {    //Если включено просыпание от аксел.
        /* Enable WKUP pin 1 */
        PWR_WakeUpPinCmd(PWR_WakeUpPin_1, ENABLE);
        PWR_UltraLowPowerCmd(ENABLE);
    }

    PWR_UltraLowPowerCmd(ENABLE);

    for(;;) {
        // PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
        PWR_EnterSTANDBYMode();    //  -_-zZ
    }
}

static void printDate(const RTC_t* const pDate)
{
    DP_GSM("%02d/", pDate->mday);
    DP_GSM("%02d/", pDate->month);
    DP_GSM("%02d ", pDate->year);
    DP_GSM("%02d:", pDate->hour);
    DP_GSM("%02d:", pDate->min);
    DP_GSM("%02d\r\n", pDate->sec);
}

uint32_t GetFlagsControlRegister(void)
{
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

    uint32_t StatReg = RCC->CSR;
    DPS("D_CONTROL REGISTER VALUE:\r\n");
#define SIZE_MSG_CONTR_REG 9
    for(uint8_t i = 0, n = 31; i < SIZE_MSG_CONTR_REG; i++, n--) {
        if(StatReg & (1 << n)) {
            DPS(" ");
            DPS(pstrControlRegister[i]);
            DPS("\r\n");
        }
    }
    // RCC_ClearFlag();
    return StatReg;
}

void controlSleepMode(void)
{
    if(ResetFlagSleep()) {
        InitUSART(UART_DBG, DBG_BAUDRATE);
        InitDMA(UART_DBG);
        DPS("\r\n-=D_WATCHDOG IN SLEEP RESET=-\r\n");
        GetFlagsControlRegister();
        RCC_ClearFlag();
        if(RTC_Configuration()) {
            DPS("\r\nD_RESET DATE RTC\r\n\r\n");
            resetSystemDateTime();    // Сбрасываем время по умолчанию
        }
        SetTimeSleepDevice();
    }

    if(GetTypeConnectFm911() == TYPE_REG_TO_BASE) {      //Если в режиме регистраций
        if(!(GetFlagsControlRegister() & PIN_RSTF)) {    //Проверяем нажатие кнопки и если кнопка не нажата, то уснем.
            RCC_ClearFlag();
            InitUSART(UART_DBG, DBG_BAUDRATE);
            InitDMA(UART_DBG);
            DPS("\r\n-=D_BAD RESET=-\r\n");
            DelayResolution100us(100);

            /* Enable the RTC Clock */
            RCC_RTCCLKCmd(ENABLE);

            /* Enable the PWR clock */
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
            /* Enable the RTC Clock */
            RCC_RTCCLKCmd(ENABLE);
            /* Allow access to RTC */
            PWR_RTCAccessCmd(ENABLE);
            PWR_PVDCmd(DISABLE);

            PWR_UltraLowPowerCmd(ENABLE);

            // Set low power configuration
            RCC->AHBENR = 0;
            RCC->AHBLPENR = 0;
            // PWR management enable
            RCC->APB1ENR = RCC_APB1ENR_PWREN;
            RCC->APB2ENR = 0;
            SysTick->CTRL = 0;
            PWR_UltraLowPowerCmd(ENABLE);
            for(;;) {
                // PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
                PWR_EnterSTANDBYMode();    //  -_-zZ
            }
        }
    }
    else {    //Если в стандартных режимах

        uint32_t CurSecCur = time();
        uint32_t CurSecAlr = GetDeviceSleep();

        /* Проверяем срабатывание акселерометра */
        if(GetEnableAccelToFind()) {
            if((CurSecCur < CurSecAlr) && (!(GetFlagsControlRegister() & PIN_RSTF))) {
                RCC_AHBPeriphClockCmd(ACCEL_INT_GPIO_CLK, ENABLE);
                RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

                /* Configure accel pin as input */
                GPIO_InitTypeDef GPIO_InitStructure;
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
                GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
                GPIO_InitStructure.GPIO_Pin = ACCEL_INT_PIN;
                GPIO_Init(ACCEL_INT_GPIO_PORT, &GPIO_InitStructure);
                if(!(GPIO_ReadInputDataBit(ACCEL_INT_GPIO_PORT, ACCEL_INT_PIN))) {
                    RCC_ClearFlag();
                    InitUSART(UART_DBG, DBG_BAUDRATE);
                    InitDMA(UART_DBG);
                    DPS("\r\n-=D_BAD RESET=-\r\n");
                    DelayResolution100us(100);
                    UpdateTimeSleepDevice();
                }
            }
            return;
        }

        /* Проверим время просыпания и нажатие на кнопку */
        if((CurSecCur < CurSecAlr) && (!(GetFlagsControlRegister() & PIN_RSTF))) {
            RCC_ClearFlag();
            InitUSART(UART_DBG, DBG_BAUDRATE);
            InitDMA(UART_DBG);
            DPS("\r\n-=D_BAD RESET=-\r\n");
            DelayResolution100us(100);
            UpdateTimeSleepDevice();
        }
    }
}

// заходим с 1, что бы сбросить статус девайса, 0 - что бы  оставить.
RESET_STATUS_DEVICE GetFlagsResetDevice(_Bool reset_status_device)
{
    RESET_STATUS_DEVICE temp = GetStatusReset();
#ifdef FM4
    if(temp == BUTTON_RESET) {
        temp = POWER_ON;
    }
#endif
    if(reset_status_device) {
        RCC_ClearFlag();
        SetStatusReset(NO_RESET);    //Сбросим статус reset девайса.
        SetStatusDeviceReset(GetStatusReset());
    }
    return temp;
}

void SystemUpdateMonitor(void)
{
    g_uiSysMonitor++;
}

uint32_t GetSystemMonitor(void)
{
    return g_uiSysMonitor;
}

#pragma optimize = none
static uint32_t GetTimeSleepDevice(void)
{
    uint32_t uiTimeSleepDevice;

    if(GetModeDevice() == TRACK_ION) {
        return 60;
    }

    if(GetCountReConnect()) {
        uiTimeSleepDevice = time();
        uiTimeSleepDevice += GetTimeReconnect((GetCountReConnect() - 1));    //Если были неудачные попытки выхода на связь.
    }
    else {
        if(GetModeProtocol() == FINDME_911) {
            /* Для протокола FindMe не важно в каком режиме девайс, время вычесляется одинаково */
            uiTimeSleepDevice = GetSleepTimeFind();
        }
        else {
            if(GetModeDevice() == STANDART) {                                          //Время сна в СТАНДАРТНОМ режиме.
                uiTimeSleepDevice = time() + GetSleepTimeStandart() * 60 * 60 * 24;    //режим в сутках и переводим в секунды
            }
            else {                                                       //Время сна в режиме ПОИСК.
                uiTimeSleepDevice = time() + GetSleepTimeFind() * 60;    //режим в минутах и переводим в секунды
            }
        }
    }
    return uiTimeSleepDevice;
}

void SetTimeSleepDevice(void)
{
    RTC_t DateRTC;
    SetStatusReset(WAKE_UP_ALARM);
    SetStatusDeviceReset(GetStatusReset());

    uint32_t CurSec = time();
    getSystemDate(&DateRTC);
    DP_GSM("D_CUR DATE: ");
    printDate(&DateRTC);

    InitAlarm();

    /* Вычисляем время пробуждения */
    T_TYPE_CONNECT eTypeCon = GetTypeConnectFm911();
    uint32_t WkpSec;
    if(eTypeCon == TYPE_REG_USER || eTypeCon == TYPE_REG_TO_BASE) {
        /* получаем номер пользователя и если оно уже задан, то делаем переоткладывание сеанца */
        char strTel[SIZE_TEL] = { 0 };
        if(GetUserTel(strTel) && eTypeCon == TYPE_REG_USER) {
            WkpSec = GetTimeSleepDevice() + DELTA_TIME_OFFSET;
        }
        else {
            /* Если мы в режиме регистрации в БД или ожидании пользователя, то уснем на длительное время(1год) */
            WkpSec = CurSec + TIME_STANDBY;
        }
    }
    else {
        WkpSec = GetTimeSleepDevice() + DELTA_TIME_OFFSET;
    }

    /* Проверка переполния времени пробуждения */
    if(CurSec >= (WkpSec - DELTA_TIME_OFFSET)) {    // Если текущее время больше времени пробуждения, то сдвигаем время на дельту
        WkpSec = CurSec + DELTA_TIME_OFFSET;
    }

    Sec2Date(&DateRTC, WkpSec);
    DP_GSM("D_WAKEUP DATE: ");
    printDate(&DateRTC);

    DPS("\r\n-=SLEEP DEVICE=-\r\n");

    DelayResolution100us(200);

    int reset_alarm = SetAlarmTime(&DateRTC);    //проверяем установку пробуждения
    while(reset_alarm) {
        ResetAlarmTime();    //сбрасываем флаг тревоги естли не получилось установить тревогу
        DelayResolution100us(10);
        reset_alarm = SetAlarmTime(&DateRTC);
    }
    SetDeviceSleep(WkpSec);    //Фиксируем время когда должны проснуться

    NVIC_Deinit();
    RCC_Deinit();
    GPIO_Deinit();

    /* Enable the RTC Clock */
    RCC_RTCCLKCmd(ENABLE);

    /* Enable the PWR clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    /* Enable the RTC Clock */
    RCC_RTCCLKCmd(ENABLE);
    /* Allow access to RTC */
    PWR_RTCAccessCmd(ENABLE);

    PWR_PVDCmd(DISABLE);
    PWR_UltraLowPowerCmd(ENABLE);

    // Set low power configuration
    RCC->AHBENR = 0;
    RCC->AHBLPENR = 0;
    // PWR management enable
    RCC->APB1ENR = RCC_APB1ENR_PWREN;
    RCC->APB2ENR = 0;
    SysTick->CTRL = 0;
    PWR_UltraLowPowerCmd(ENABLE);

    _Bool accel_enable = FALSE;
    if(GetModeProtocol() == FINDME_911 && GetEnableAccelToFind()) {    //Если включено просыпание от акселерометра и стоит режим поиска.
        accel_enable = TRUE;
    }

    if(GetModeProtocol() == ION_FM && GetModeDevice() == TIMER_FIND && GetEnableAccelToFind()) {
        accel_enable = TRUE;
    }

    if(accel_enable) {    //Если включено просыпание от аксел.
        /* Enable WKUP pin 1 */
        PWR_WakeUpPinCmd(PWR_WakeUpPin_1, ENABLE);
        PWR_UltraLowPowerCmd(ENABLE);
    }

    PWR_UltraLowPowerCmd(ENABLE);

    for(;;) {
        // PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
        PWR_EnterSTANDBYMode();    //  -_-zZ
    }
}

_Bool FlagReset = FALSE;
void setFlagReset(void)
{
    FlagReset = TRUE;
}

_Bool getFlagReset(void)
{
    return FlagReset;
}